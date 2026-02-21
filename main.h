#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f103xb.h"
#ifndef NULL
#define NULL 0
#endif
/*NOTE in system_stm32f10x.c in function SetSysClockTo72() added software delays for
* stable external oscillator generator starting
*/
extern uint32_t SystemCoreClock;
/*NOTE about APB1 frequency from initializing. E.g. 
 *      RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE1_DIV2;
 *        set APB1 to SystemCoreClock/2=36MHz
 */
#define APB1_CLOCK_FREQ (36000000)

/**<  MCU IRQ priority level may be in [0(highest)---IRQ_LOWEST_PRIOR] range*/
#define IRQ_HIGHEST_PRIOR 0
#define IRQ_LOWEST_PRIOR ((1<<__NVIC_PRIO_BITS)-1)
/**<All IRQ with level greater number (i.e. lower priority) switched off at break condition */
#define IRQ_LOWEST_BREAK_PRIOR ((1<<__NVIC_PRIO_BITS)/2)





/*****************************************************************************
* @brief 
* @param 
* @param  
* @param 
* @return                                                                          
******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* MAIN_H_INCLUDED */
