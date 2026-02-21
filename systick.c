#include "main.h"

static uint32_t tick_cnt=0;
static volatile char tick_fg=0;

uint32_t GetTickCnt(void){
    return tick_cnt;
}
static inline uint32_t GetTickDelta(uint32_t prev_tick){
    return tick_cnt-prev_tick;
}
void SysTick_Handler(void){
	(void)SysTick->CTRL;
	tick_fg=1;
	tick_cnt++;
}
char IsSysTickExp(void){
	char res;
	res=tick_fg;
	if(res)tick_fg=0;
	return res;
}
char IsSysTickTimerExp(uint32_t* prev_tick,uint32_t timeout){
    char res=0;
    if((tick_cnt-*prev_tick)>timeout){
        res=1;
        *prev_tick+=timeout;
    }
    return res;
}
