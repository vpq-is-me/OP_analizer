#ifndef SYSTICK_H_INCLUDED
#define SYSTICK_H_INCLUDED

#ifdef __cplusplus
 extern "C" {
#endif
#include "stdint.h"

uint32_t GetTickCnt(void);
uint32_t GetTickDelta(uint32_t prev_tick);
char IsSysTickExp(void);

/** \brief Check timeout  expired from previous checkpoint 'prev_tick'
 *    If so function return 1 and update 'prev_tick' by adding 'timeout'
 * \param prev_tick uint32_t* - relative time (checkpoint)
 * \param timeout uint32_t - timeout value in milliseconds
 * \return char 0 - if timeout not expired
 *              1 - otherwise
 */
char IsSysTickTimerExp(uint32_t* prev_tick,uint32_t timeout);
#define SysTickInit() SysTick_Config(SystemCoreClock/1000)

#ifdef __cplusplus
}
#endif

#endif /* SYSTICK_H_INCLUDED */
