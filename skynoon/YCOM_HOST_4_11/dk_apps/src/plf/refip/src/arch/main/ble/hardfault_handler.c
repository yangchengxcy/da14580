
// HardFault_Handler() code added to boot_vectors.s

#include "global_io.h"
#include "ARMCM0.h"


void HardFault_HandlerC(unsigned long *hardfault_args)
{
    __asm("BKPT #0\n") ; 
}
