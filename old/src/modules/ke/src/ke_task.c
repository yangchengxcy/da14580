/**
 ****************************************************************************************
 *
 * @file ke_task.c
 *
 * @brief This file contains the implementation of the kernel task management.
 *
 * Copyright (C) RivieraWaves 2009-2013
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup TASK
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"       // stack configuration

#include <stddef.h>            // standard definition
#include <stdint.h>            // standard integer
#include <stdbool.h>           // standard boolean
#include <string.h>            // memcpy defintion

#include "ke_config.h"         // kernel configuration
#include "ke_task.h"           // kernel task
#include "ke_env.h"            // kernel environment
#include "ke_queue.h"          // kernel queue
#include "ke_event.h"          // kernel event
#include "ke_mem.h"            // kernel memory




/*
 * DEFINES
 ****************************************************************************************
 */


/*
 * STRUCTURES DEFINTIONS
 ****************************************************************************************
 */

/// KE TASK element structure
struct ke_task_elem
{
    uint8_t   type;
    struct ke_task_desc const * p_desc;
};

/// KE TASK environment structure
struct ke_task_env_tag
{
    uint8_t task_cnt;
    struct ke_task_elem task_list[10];    
};


/*
 * GLOBAL VARIABLES
 ****************************************************************************************
 */

/// KE TASK environment
static struct ke_task_env_tag ke_task_env_ram __attribute__((section("exchange_mem_case1"))); 
extern struct ke_task_env_tag ke_task_env;

/*
 * LOCAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Compare destination task callback.
 *
 * @param[in] msg          kernel message
 * @param[in] dest_id      destination id
 *
 * @return bool
 ****************************************************************************************
 */
static bool cmp_dest_id(struct co_list_hdr const * msg, uint32_t dest_id)
{
    return ((struct ke_msg*)msg)->dest_id == dest_id;
}


/**
 ****************************************************************************************
 * @brief Reactivation of saved messages.
 *
 * This primitive looks for all the messages destined to the task ke_task_id that
 * have been saved and inserts them into the sent priority queue. These
 * messages will be scheduled at the next scheduler pass.
 *
 * @param[in] ke_task_id    Destination Identifier
 ****************************************************************************************
 */
static void ke_task_saved_update(ke_task_id_t const ke_task_id)
{
    struct ke_msg * msg;

    for(;;)
    {
        // if the state has changed look in the Save queue if a message
        // need to be handled
        msg = (struct ke_msg*) ke_queue_extract(&ke_env.queue_saved,
                                                &cmp_dest_id,
                                                (uint32_t) ke_task_id);

        if (msg == NULL) break;

        //printf ("-- saved found %x %x\n", ke_task_id, msg->id);

        // Insert it back in the sent queue
        GLOBAL_INT_DISABLE();
        ke_queue_push(&ke_env.queue_sent, (struct co_list_hdr*)msg);
        GLOBAL_INT_RESTORE();

        // trigger the event
        ke_event_set(KE_EVENT_KE_MESSAGE);
    }

    return;
}


/**
 ****************************************************************************************
 * @brief Search message handler function matching the msg id
 *
 * @param[in] msg_id        Message identifier
 * @param[in] state_handler Pointer to the state handler
 *
 * @return                  Pointer to the message handler (NULL if not found)
 *
 ****************************************************************************************
 */
static ke_msg_func_t ke_handler_search(ke_msg_id_t const msg_id, struct ke_state_handler const *state_handler)
{
    // Get the message handler function by parsing the message table
    for (int i = (state_handler->msg_cnt-1); 0 <= i; i--)
    {
        if ((state_handler->msg_table[i].id == msg_id)
                || (state_handler->msg_table[i].id == KE_MSG_DEFAULT_HANDLER))
        {
            // If handler is NULL, message should not have been received in this state
            ASSERT_ERR(state_handler->msg_table[i].func);

            return state_handler->msg_table[i].func;
        }
    }

    // If we execute this line of code, it means that we did not find the handler
    return NULL;
}


/**
 ****************************************************************************************
 * @brief Retrieve appropriate message handler function of a task
 *
 * @param[in]  msg_id   Message identifier
 * @param[in]  task_id  Task instance identifier
 *
 * @return              Pointer to the message handler (NULL if not found)
 *
 ****************************************************************************************
 */
static ke_msg_func_t ke_task_handler_get(struct ke_task_env_tag *ke_task_env_ptr, ke_msg_id_t const msg_id, ke_task_id_t const task_id)
{
    ke_msg_func_t func = NULL;
    int idx = KE_IDX_GET(task_id);
    int type = KE_TYPE_GET(task_id);
    uint8_t hdl;
    struct ke_task_desc const * p_task_desc = NULL;
    struct ke_task_elem * curr_list = ke_task_env_ptr->task_list;
    uint8_t curr_nb = ke_task_env_ptr->task_cnt;

    ASSERT_ERR(type < TASK_MAX);

    // Search task handle
    for(hdl=0 ; hdl < curr_nb; hdl++)
    {
        if(curr_list[hdl].type == type)
        {
            p_task_desc = ke_task_env_ptr->task_list[hdl].p_desc;
            break;
        }
    }

    ASSERT_INFO(hdl < curr_nb, hdl, task_id);

    ASSERT_INFO((idx < p_task_desc->idx_max), idx, p_task_desc->idx_max);

    // If the idx found is out of range return NULL
    if(idx < p_task_desc->idx_max)
    {
        // Retrieve a pointer to the task instance data
        if (p_task_desc->state_handler)
        {
            func = ke_handler_search(msg_id, p_task_desc->state_handler + p_task_desc->state[idx]);
        }

        // No handler... need to retrieve the default one
        if (func == NULL && p_task_desc->default_handler)
        {
            func = ke_handler_search(msg_id, p_task_desc->default_handler);
        }
    }

    return func;
}


/**
 ****************************************************************************************
 * @brief Scheduler entry point.
 *
 * This function is the scheduler of messages. It tries to get a message
 * from the sent queue, then try to get the appropriate message handler
 * function (from the current state, or the default one). This function
 * is called, then the message is saved or freed.
 ****************************************************************************************
 */
static void ke_task_schedule(void)
{
    // Process one message at a time to ensure that events having higher priority are
    // handled in time
    do
    {
        int msg_status;
        struct ke_msg *msg;
        // Get a message from the queue
        GLOBAL_INT_DISABLE();
        msg = (struct ke_msg*) ke_queue_pop(&ke_env.queue_sent);
        GLOBAL_INT_RESTORE();
        if (msg == NULL) break;

        // Retrieve a pointer to the task instance data
        // ROM first
        ke_msg_func_t func = ke_task_handler_get(&ke_task_env, msg->id, msg->dest_id);
        if (!func) // then RAM
            func = ke_task_handler_get(&ke_task_env_ram, msg->id, msg->dest_id);

        // sanity check
        ASSERT_WARN(func != NULL);

        // Call the message handler
        if (func != NULL)
        {
            msg_status = func(msg->id, ke_msg2param(msg), msg->dest_id, msg->src_id);
        }
        else
        {
            msg_status = KE_MSG_CONSUMED;
        }

        switch (msg_status)
        {
        case KE_MSG_CONSUMED:
            // Free the message
            ke_msg_free(msg);
            break;

        case KE_MSG_NO_FREE:
            break;

        case KE_MSG_SAVED:
            // The message has been saved
            // Insert it at the end of the save queue
            ke_queue_push(&ke_env.queue_saved, (struct co_list_hdr*) msg);
            break;

        default:
            ASSERT_ERR(0);
            break;
        } // switch case
    } while(0);

    // Verify if we can clear the event bit
    GLOBAL_INT_DISABLE();
    if (co_list_is_empty(&ke_env.queue_sent))
        ke_event_clear(KE_EVENT_KE_MESSAGE);
    GLOBAL_INT_RESTORE();
}


/*
 * EXPORTED FUNCTION DEFINITIONS
 ****************************************************************************************
 */

void ke_task_init_ram(void) 
{
    memset(&ke_task_env_ram, 0, sizeof(ke_task_env_ram));

    // Register message event
    ke_event_callback_set(KE_EVENT_KE_MESSAGE, &ke_task_schedule);
}

uint8_t ke_task_create(uint8_t task_type, struct ke_task_desc const * p_task_desc)
{
    uint8_t status = KE_TASK_OK;
    uint8_t hdl = 0;

    GLOBAL_INT_DISABLE();

    struct ke_task_elem * curr_list = ke_task_env_ram.task_list;
    uint8_t curr_nb = ke_task_env_ram.task_cnt;

    do
    {
        if(curr_nb > 0)
        {
            // Search task handle
            for(hdl=0 ; hdl < curr_nb; hdl++)
            {
                if(curr_list[hdl].type == task_type)
                {
                    status = KE_TASK_ALREADY_EXISTS;
                    break;
                }
            }
        }

        if(status != KE_TASK_OK)
            break;

        ASSERT_INFO(hdl < KE_TASK_SIZE, hdl, KE_TASK_SIZE);

        if(hdl >= KE_TASK_SIZE)
        {
            status = KE_TASK_CAPA_EXCEEDED;
            break;
        }

        // Save task ID
        ke_task_env_ram.task_list[curr_nb].type = task_type;

        // Save task descriptor
        ke_task_env_ram.task_list[curr_nb].p_desc = p_task_desc;

        // Increment number of tasks
        ke_task_env_ram.task_cnt = curr_nb + 1;

    } while(0);

    GLOBAL_INT_RESTORE();

#if  DEVELOPMENT__NO_OTP    
    if((status != KE_TASK_OK) && (status != KE_TASK_ALREADY_EXISTS))
        __asm("BKPT #0\n");
#warning "ke_task_create(): remove BreakPoint in the final version!!!"
#endif
    
    return status;
}


void ke_state_set(ke_task_id_t const id, ke_state_t const state_id)
{
    int idx = KE_IDX_GET(id);
    int type = KE_TYPE_GET(id);
    uint8_t hdl;
    struct ke_task_desc const * p_task_desc = NULL;
    struct ke_task_elem * curr_list = ke_task_env_ram.task_list;
    uint8_t curr_nb = ke_task_env_ram.task_cnt;
    ke_state_t *ke_stateid_ptr = NULL;

    // sanity checks
    ASSERT_ERR(type < TASK_MAX);

    // Search task handle
    for(hdl=0 ; hdl < curr_nb; hdl++)
    {
        if(curr_list[hdl].type == type)
        {
            p_task_desc = ke_task_env_ram.task_list[hdl].p_desc;
            break;
        }
    }

    ASSERT_INFO(hdl < curr_nb, hdl, curr_nb);
    ASSERT_INFO((idx < p_task_desc->idx_max), idx, p_task_desc->idx_max);

    // If the idx found is out of range return NULL
    if(idx < p_task_desc->idx_max)
    {
        // Get the state
        ke_stateid_ptr = &p_task_desc->state[idx];

        ASSERT_ERR(ke_stateid_ptr);

        // set the state
        if (*ke_stateid_ptr != state_id)
        {
            *ke_stateid_ptr = state_id;

            // if the state has changed update the SAVE queue
            ke_task_saved_update(id);
        }
    }
}


ke_state_t ke_state_get(ke_task_id_t const id)
{
    int idx = KE_IDX_GET(id);
    int type = KE_TYPE_GET(id);
    uint8_t hdl;
    struct ke_task_desc const * p_task_desc = NULL;
    struct ke_task_elem * curr_list = ke_task_env_ram.task_list;
    uint8_t curr_nb = ke_task_env_ram.task_cnt;

    ASSERT_ERR(type < TASK_MAX);

    // Search task handle
    for(hdl=0 ; hdl < curr_nb; hdl++)
    {
        if(curr_list[hdl].type == type)
        {
            p_task_desc = ke_task_env_ram.task_list[hdl].p_desc;
            break;
        }
    }

    ASSERT_INFO(hdl < curr_nb, hdl, curr_nb);
    ASSERT_INFO((idx < p_task_desc->idx_max), idx, p_task_desc->idx_max);

    // Get the state
    return p_task_desc->state[idx];
}


/// @} TASK
