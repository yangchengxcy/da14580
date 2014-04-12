/**
 ****************************************************************************************
 *
 * @file gapm.c
 *
 * @brief Generic Access Profile Manager Implementation.
 *
 * Copyright (C) RivieraWaves 2009-2013
 *
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @addtogroup GAPM Generic Access Profile Manager
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"  // Software configuration

#include "gap.h"        // Generic access profile
#include "gapm.h"       // Generic access profile Manager
#include "gapc.h"       // Generic access profile Controller

#include "gapm_task.h"  // Generic access profile Manager

#include "attm_db.h"    // Attribute Database management
#include "attm_util.h"  // Attribute Database management Utils

#include "ke_mem.h"     // Kernel memory management

#if (NVDS_SUPPORT)
#include "nvds.h"
#endif // (NVDS_SUPPORT)

#include "l2cm.h"
#include "smpm.h"
#include "prf_utils.h"


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */



/*
 * DEFINES
 ****************************************************************************************
 */

/*
 * MACROS
 ****************************************************************************************
 */


/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
 
 
 		
		
extern struct gap_cfg_table_struct *gap_cfg_table __attribute__((section("exchange_mem_case1")));;


const struct gap_cfg_table_struct gap_timeout_table  __attribute__((section("timeout_table_area")))=	{

.GAP_TMR_LIM_ADV_TIMEOUT_VAR=0x4650,
.GAP_TMR_GEN_DISC_SCAN_VAR=0x0300,
.GAP_TMR_LIM_DISC_SCAN_VAR=0x0300,
.GAP_TMR_PRIV_ADDR_INT_VAR=0x3A98,
.GAP_TMR_CONN_PAUSE_CT_VAR=0x0064,
.GAP_TMR_CONN_PAUSE_PH_VAR=0x01F4,
.GAP_TMR_CONN_PARAM_TIMEOUT_VAR=0x0BB8,
.GAP_TMR_SCAN_FAST_PERIOD_VAR=0x0C00,
.GAP_TMR_ADV_FAST_PERIOD_VAR=0x0BB8,
.GAP_LIM_DISC_SCAN_INT_VAR=0x0012,
.GAP_SCAN_FAST_INTV_VAR=0x0030,
.GAP_SCAN_FAST_WIND_VAR=0x0030,
.GAP_SCAN_SLOW_INTV1_VAR=0x00CD,
.GAP_SCAN_SLOW_INTV2_VAR=0x019A,
.GAP_SCAN_SLOW_WIND1_VAR=0x0012,
.GAP_SCAN_SLOW_WIND2_VAR=0x0024,
.GAP_ADV_FAST_INTV1_VAR=0x0030,
.GAP_ADV_FAST_INTV2_VAR=0x0064,
.GAP_ADV_SLOW_INTV_VAR=0x00B0,
.GAP_INIT_CONN_MIN_INTV_VAR=0x0018,
.GAP_INIT_CONN_MAX_INTV_VAR=0x0028,
.GAP_INQ_SCAN_INTV_VAR=0x0012,
.GAP_INQ_SCAN_WIND_VAR=0x0012,
.GAP_CONN_SUPERV_TIMEOUT_VAR=0x07D0,
.GAP_CONN_MIN_CE_VAR=0x0000,
.GAP_CONN_MAX_CE_VAR=0xFFFF,
.GAP_CONN_LATENCY_VAR=0x0000,
.GAP_LE_MASK_VAR=0x1F,
.GAP_DEV_NAME_VAR="RIVIERAWAVES-BLE",

};
		
		
#if 0
 
 
struct gapm_env_tag gapm_env __attribute__((section("exchange_mem_case1"))); //@WIKRETENTION MEMORY;


/// GAP Manager task descriptor
static const struct ke_task_desc TASK_DESC_GAPM  =
    {gapm_state_handler, &gapm_default_handler, gapm_state, GAPM_STATE_MAX, GAPM_IDX_MAX};



#if (BLE_ATTS)
/// GAP Primary Service
static const att_svc_desc_t gapm_svc = ATT_SVC_GENERIC_ACCESS;

/// Device Name Characteristic
static const struct att_char_desc gapm_devname_ch = ATT_CHAR(ATT_CHAR_PROP_RD,
        GAP_IDX_DEVNAME,
        ATT_CHAR_DEVICE_NAME);

/// Appearance or Icon Characteristic
static const struct att_char_desc gapm_icon_ch = ATT_CHAR(ATT_CHAR_PROP_RD,
        GAP_IDX_ICON,
        ATT_CHAR_APPEARANCE);

/// Appearance or Icon value
		
static const uint16_t gapm_iconval = GAP_APPEARANCE;

#if (BLE_PERIPHERAL)
/// Privacy Flag Characteristic
static const struct att_char_desc gapm_privy_flag_ch = ATT_CHAR(ATT_CHAR_PROP_RD  | ATT_CHAR_PROP_WR,
        GAP_IDX_PRIVY_FLAG,
        ATT_CHAR_PRIVACY_FLAG);

/// Slave Parameters Characteristic
static const struct att_char_desc gapm_slave_param_ch = ATT_CHAR(ATT_CHAR_PROP_RD,
        GAP_IDX_SLAVE_PREF_PARAM,
        ATT_CHAR_PERIPH_PREF_CON_PARAM);

/// Slave Preferred Parameter value
static const struct att_slv_pref gapm_slave_param = {GAP_PPCP_CONN_INTV_MAX,
        GAP_PPCP_CONN_INTV_MIN,
        GAP_PPCP_SLAVE_LATENCY,
        GAP_PPCP_STO_MULT};

/// Reconnection Address Characteristic
static const struct att_char_desc gapm_recon_ch =
        ATT_CHAR(ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR |ATT_CHAR_PROP_WR_NO_RESP,
                GAP_IDX_RECON_ADDR,
                ATT_CHAR_RECONNECTION_ADDR);
#endif /* #if (BLE_PERIPHERAL)*/

/// GAP Attribute database description
static const struct attm_desc gapm_att_db[GAP_IDX_NUMBER] =
{
    // GAP_IDX_PRIM_SVC - GAP service
    [GAP_IDX_PRIM_SVC]          =   {ATT_DECL_PRIMARY_SERVICE, PERM(RD, ENABLE),
                                     sizeof(gapm_svc), sizeof(gapm_svc), (void *)&gapm_svc},
    // GAP_IDX_CHAR_DEVNAME - device name declaration
    [GAP_IDX_CHAR_DEVNAME]      =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE),
                                     sizeof(gapm_devname_ch), sizeof(gapm_devname_ch),
                                     (void *)&gapm_devname_ch},
    // GAP_IDX_DEVNAME - device name definition
    [GAP_IDX_DEVNAME]           =   {ATT_CHAR_DEVICE_NAME, PERM(RD, ENABLE), BD_NAME_SIZE, 0, NULL},
    // GAP_IDX_CHAR_ICON - appearance declaration
    [GAP_IDX_CHAR_ICON]         =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE),
                                     sizeof(gapm_icon_ch), sizeof(gapm_icon_ch), (void *)&gapm_icon_ch},
    // GAP_IDX_ICON -appearance
    [GAP_IDX_ICON]              =   {ATT_CHAR_APPEARANCE, PERM(RD, ENABLE),
                                     sizeof(gapm_iconval), sizeof(gapm_iconval), (void *)&gapm_iconval},
    #if (BLE_PERIPHERAL)
    // GAP_IDX_CHAR_PRIVY_FLAG - privacy flag declaration
    [GAP_IDX_CHAR_PRIVY_FLAG]   =   {ATT_DECL_CHARACTERISTIC,  PERM(RD, ENABLE),
                                     sizeof(gapm_privy_flag_ch), sizeof(gapm_privy_flag_ch),
                                     (void *)&gapm_privy_flag_ch},
    // GAP_IDX_PRIVY_FLAG - privacy flag definition
    [GAP_IDX_PRIVY_FLAG]        =   {ATT_CHAR_PRIVACY_FLAG, (PERM(RD, ENABLE) | PERM(WR, ENABLE)),
                                     sizeof(uint8_t), 0, NULL},
    // GAP_IDX_CHAR_SLAVE_PREF_PARAM - Peripheral parameters declaration
    [GAP_IDX_CHAR_SLAVE_PREF_PARAM] = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE),
                                       sizeof(gapm_slave_param_ch), sizeof(gapm_slave_param_ch),
                                       (void *)&gapm_slave_param_ch},
    // GAP_IDX_SLAVE_PREF_PARAM - Peripheral parameters definition
    [GAP_IDX_SLAVE_PREF_PARAM]  =   {ATT_CHAR_PERIPH_PREF_CON_PARAM, PERM(RD, ENABLE),
                                     sizeof(gapm_slave_param), sizeof(gapm_slave_param),
                                     (void *)&gapm_slave_param},
    // GAP_IDX_CHAR_RECON_ADDR - reconnection address declaration
    [GAP_IDX_CHAR_RECON_ADDR]   =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE),
                                     sizeof(gapm_recon_ch), sizeof(gapm_recon_ch), (void *)&gapm_recon_ch},
    // GAP_IDX_RECON_ADDR - reconnection address definition
    [GAP_IDX_RECON_ADDR]        =   {ATT_CHAR_RECONNECTION_ADDR,  (PERM(RD, ENABLE) | PERM(WR, ENABLE)),
                                     sizeof(struct bd_addr), 0, NULL},
    #endif /* (BLE_PERIPHERAL) */
};
#endif /* (BLE_ATTS) */


/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */
#if (BLE_ATTS)

/**
 ****************************************************************************************
 * @brief Create GAP database using specific start handle.
 *
 * @param[in|out] start_hdl expected GAP database start handle, used also as a
 *                          return value.
 *
 * @return Command status code:
 *  - @ref ATT_ERR_NO_ERROR: If database creation succeeds.
 *  - @ref ATT_ERR_INVALID_HANDLE: If start_hdl given in parameter + nb of attribute override
 *                            some existing services handles.
 *  - @ref ATT_ERR_INSUFF_RESOURCE: There is not enough memory to allocate service buffer.
 *                           or of new attribute cannot be added because all expected
 *                           attributes already add
 ****************************************************************************************
 */
static uint8_t gapm_create_db(uint16_t* start_hdl)
{
    /* Create database using atts shortcut. */
    return  attm_svc_create_db(start_hdl, NULL, GAP_IDX_NUMBER, NULL,
                               TASK_GAPM, &gapm_att_db[0]);
}

/* function starts */
static void gapm_init_attr(void)
{
    uint8_t status;

    /* initialize Start Handle */
    gapm_env.svc_start_hdl = 0;

    /* Create the database */
    status = gapm_create_db(&(gapm_env.svc_start_hdl));

    /* Set database initial values. */

    // Set GAP_IDX_DEVNAME
    if(status == ATT_ERR_NO_ERROR)
    {
        /* set device name */
        #if (NVDS_SUPPORT) // TODO [FBE] add to device configuration !? I think yes
        //With NVDS
        uint8_t dev_name_length = NVDS_LEN_DEVICE_NAME;
        uint8_t dev_name_data[NVDS_LEN_DEVICE_NAME];

        // __copy into global in 1 shot
        if(nvds_get(NVDS_TAG_DEVICE_NAME, &dev_name_length, dev_name_data) == NVDS_OK)
        {
            /* set device name */
            status = attmdb_att_set_value(GAPM_GET_ATT_HANDLE(GAP_IDX_DEVNAME),
                                          dev_name_length, (uint8_t*) dev_name_data);
        }
        else
        #endif // NVDS_SUPPORT
        {
            status = attmdb_att_set_value(GAPM_GET_ATT_HANDLE(GAP_IDX_DEVNAME),
                                          strlen(GAP_DEV_NAME), (uint8_t*) GAP_DEV_NAME);
        }
    }

    ASSERT_ERR(status == ATT_ERR_NO_ERROR);
}

#endif // (BLE_ATTS)

/**
 ****************************************************************************************
 * @brief Cleanup operation
 *
 * @param[in] op_type Operation type.
 ****************************************************************************************
 */
static void gapm_operation_cleanup(uint8_t op_type)
{
    uint8_t state = ke_state_get(TASK_GAPM);

    // check if operation is freed
    if(gapm_env.operation[op_type] != NULL)
    {
        // free operation
        ke_msg_free(ke_param2msg(gapm_env.operation[op_type]));
        gapm_env.operation[op_type] = NULL;
    }

    // specific air operation cleanup
    if(op_type == GAPM_AIR_OP)
    {
        // stop timers
        #if (RW_BLE_USE_CRYPT)
        ke_timer_clear(GAPM_ADDR_RENEW_TO_IND , TASK_GAPM);
        #endif // (RW_BLE_USE_CRYPT)


        #if (BLE_PERIPHERAL || BLE_BROADCASTER)
        ke_timer_clear(GAPM_LIM_DISC_TO_IND , TASK_GAPM);
        #endif // (BLE_PERIPHERAL || BLE_BROADCASTER)

        #if (BLE_CENTRAL || BLE_OBSERVER)
        ke_timer_clear(GAPM_SCAN_TO_IND , TASK_GAPM);
        // free bd address filter
        if(gapm_env.scan_filter != NULL)
        {
            ke_free(gapm_env.scan_filter);
            gapm_env.scan_filter = NULL;
        }
        #endif // (BLE_CENTRAL || BLE_OBSERVER)
    }


    if(state != GAPM_DEVICE_SETUP)
    {
        // set state to Idle
        ke_state_set(TASK_GAPM, (state & (~(1 << op_type))));
    }
}



/*
 * EXPORTED FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

void gapm_init(bool reset)
{
    // boot configuration
    if(!reset)
    {
        // Create GAP Manager task
        ke_task_create(TASK_GAPM, &TASK_DESC_GAPM);

        // by default set operation pointer to null
        gapm_env.operation[GAPM_CFG_OP] = NULL;
        gapm_env.operation[GAPM_AIR_OP] = NULL;
        #if (BLE_CENTRAL || BLE_OBSERVER)
        gapm_env.scan_filter = NULL;
        #endif // (BLE_CENTRAL || BLE_OBSERVER)
    }
		
		gap_cfg_table = (struct gap_cfg_table_struct*)&gap_timeout_table;

    #if (BLE_ATTS)
    /* Initialize database */
    attmdb_init(reset);
    #endif /* (BLE_ATTS) */

    gapm_operation_cleanup(GAPM_CFG_OP);
    gapm_operation_cleanup(GAPM_AIR_OP);

    #if (BLE_CENTRAL || BLE_PERIPHERAL)
    // Initialize GAP controllers
    gapc_init(reset);
    #endif //(BLE_CENTRAL || BLE_PERIPHERAL)

    // clear current role
    gapm_env.role = GAP_NO_ROLE;
    // all connections reset
    gapm_env.connections = 0;

    #if (BLE_ATTS)
    // since a software reset is mandatory before doing anything, reduce startup time
    if(reset)
    {
    // Initialize GAP database
    gapm_init_attr();
    }
    #endif // (BLE_ATTS)

    /* Set the GAP state to GAP_DEV_SETUP
     * - First time GAP is used, a reset shall be performed to correctly initialize lower
     *   layers configuration. It can be performed automatically if receive a ready event
     *   from lower layers, but environment, since it's not mandatory to trigger this
     *   message, GAP must wait for a reset request before doing anything.
     */
    ke_state_set(TASK_GAPM, GAPM_DEVICE_SETUP);
}


void gapm_host_init(void)
{

    // initialize GAP
    gapm_init(true);

    #if (BLE_CENTRAL || BLE_PERIPHERAL)
    // Initialize L2CAP
    l2cm_init(true);
    // Initialize GATT & ATT
    gattm_init(true);
    attm_init(true);
    #endif /* #if (BLE_CENTRAL || BLE_PERIPHERAL) */

    #if (RW_BLE_USE_CRYPT)
    // Initialize SMP
    smpm_init(true);
    #endif /* #if (RW_BLE_USE_CRYPT) */

    #if (BLE_CLIENT_PRF || BLE_SERVER_PRF)
    // Initialize Profiles
    prf_init();
    #endif /* (BLE_CLIENT_PRF || BLE_SERVER_PRF) */
}


uint8_t gapm_get_operation(uint8_t op_type)
{
    // by default no operation
    uint8_t ret = GAPM_NO_OP;

    ASSERT_ERR(op_type < GAPM_MAX_OP);

    // check if an operation is registered
    if(gapm_env.operation[op_type] != NULL)
    {
        // operation code if first by of an operation command
        ret = (*((uint8_t*) gapm_env.operation[op_type]));
    }

    return ret;
}

void gapm_send_complete_evt(uint8_t op_type, uint8_t status)
{
    // prepare command completed event
    struct gapm_cmp_evt* cmp_evt = KE_MSG_ALLOC(GAPM_CMP_EVT,
                gapm_env.requester[op_type], TASK_GAPM, gapm_cmp_evt);

    cmp_evt->operation = gapm_get_operation(op_type);
    cmp_evt->status = status;

    // send event
    ke_msg_send(cmp_evt);

    // cleanup operation
    gapm_operation_cleanup(op_type);
}

void gapm_send_error_evt(uint8_t operation, const ke_task_id_t requester, uint8_t status)
{
    // prepare command completed event with error status
    struct gapm_cmp_evt* cmp_evt = KE_MSG_ALLOC(GAPM_CMP_EVT,
            requester, TASK_GAPM, gapm_cmp_evt);

    cmp_evt->operation = operation;
    cmp_evt->status = status;

    // send event
    ke_msg_send(cmp_evt);
}

#if (BLE_CENTRAL || BLE_PERIPHERAL)
uint8_t gapm_con_create(struct llc_create_con_cmd_complete const *con_params,
                        struct gapm_air_operation* air_op)
{
    // First create GAP controller task (that send indication message)
    uint8_t conidx = gapc_con_create(con_params,
            ((air_op->code == GAPM_CONNECTION_NAME_REQUEST) ? TASK_GAPM : gapm_env.requester[GAPM_AIR_OP]),
            ((air_op->addr_src == GAPM_PUBLIC_ADDR) ? &(gapm_env.addr) : &(air_op->addr)),
            ((air_op->addr_src == GAPM_PUBLIC_ADDR) ? ADDR_PUBLIC : ADDR_RAND));





    // check if an error occurs.
    if(conidx != GAP_INVALID_CONIDX)
    {
        // Increment number of connections.
        gapm_env.connections++;

        /* ******** Inform other tasks that connection has been established. ******** */

        // Inform L2CAP about new connection
        l2cm_create(conidx);
        // Inform GATT about new connection
        gattm_create(conidx);
        // Inform ATT about new connection
        attm_create(conidx);

        #if (RW_BLE_USE_CRYPT)
        #if (BLE_CENTRAL || BLE_PERIPHERAL)
        // Inform SMP about new connection
        smpm_create(conidx);
        #endif //(BLE_CENTRAL || BLE_PERIPHERAL)
        #endif /* #if (RW_BLE_USE_CRYPT) */

        #if (BLE_CLIENT_PRF || BLE_SERVER_PRF)
        // Inform profiles about new connection
        prf_create(conidx);
        #endif /* (BLE_CLIENT_PRF || BLE_SERVER_PRF) */
    }

    return conidx;
}


void gapm_con_enable(uint8_t conidx)
{
    // sanity check.
    if(conidx != GAP_INVALID_CONIDX)
    {
        // Inform ATT that connection information has be set
        attm_con_enable(conidx);

//        #if (RW_BLE_USE_CRYPT)
//        // Inform SMP that connection information has be set
//        smpm_con_enable(conidx); // TODO [FBE] implement
//        #endif /* #if (RW_BLE_USE_CRYPT) */
    }
}


void gapm_con_cleanup(uint8_t conidx, uint16_t conhdl, uint8_t reason)
{
    // check if an error occurs.
    if(conidx != GAP_INVALID_CONIDX)
    {
        // Decrement number of connections.
        gapm_env.connections--;

        /* ******** Inform other tasks that connection has been disconnected. ******** */

        // Inform GAPC about terminated connection
        gapc_con_cleanup(conidx);
        // Inform L2CAP about terminated connection
        l2cm_cleanup(conidx);
        // Inform GATT about terminated connection
        gattm_cleanup(conidx);
        // Inform ATT about terminated connection
        attm_cleanup(conidx);

        #if (RW_BLE_USE_CRYPT)
        // Inform SMP about terminated connection
        smpm_cleanup(conidx);
        #endif /* #if (RW_BLE_USE_CRYPT) */

        #if (BLE_CLIENT_PRF || BLE_SERVER_PRF)
        // Inform profiles about terminated connection
        prf_cleanup(conidx, conhdl, reason);
        #endif /* (BLE_CLIENT_PRF || BLE_SERVER_PRF) */
    }
}

#endif /* (BLE_CENTRAL || BLE_PERIPHERAL) */

#endif

/// @} GAPM
