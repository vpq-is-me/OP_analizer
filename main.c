#include "main.h"
#include "systick.h"
#include "I2Celem.h"
#include "ads1115.h"
#include "stdio.h"
#include "uart.h"

void ADC_SampleTimerInit(void);
//****************************************************************************************************************
void InitPeripheryPower(void){
    RCC->AHBENR |= 0 | RCC_AHBENR_DMA1EN;
    RCC->APB1ENR |=0 | RCC_APB1ENR_I2C1EN | RCC_APB1ENR_I2C2EN | RCC_APB1ENR_TIM4EN | RCC_APB1ENR_USART2EN;
    RCC->APB2ENR |=0 | RCC_APB2ENR_AFIOEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPAEN;
}
//****************************************************************************************************************
#define ConfigGPIO_HighPin(port,pin,md,cfg) port->CRH=(port->CRH & ~(GPIO_CRH_MODE##pin##_Msk|GPIO_CRH_CNF##pin##_Msk)) \
                                                      | (GPIO_CRH_MODE##pin & (md<<GPIO_CRH_MODE##pin##_Pos)) \
                                                      | (GPIO_CRH_CNF##pin & (cfg<<GPIO_CRH_CNF##pin##_Pos))
#define ConfigGPIO_LowPin(port,pin,md,cfg)  port->CRL=(port->CRL & ~(GPIO_CRL_MODE##pin##_Msk|GPIO_CRL_CNF##pin##_Msk)) \
                                                      | (GPIO_CRL_MODE##pin & (md<<GPIO_CRL_MODE##pin##_Pos)) \
                                                      | (GPIO_CRL_CNF##pin & (cfg<<GPIO_CRL_CNF##pin##_Pos))

void GPIO_Init(void) {
    /**< LED */
    ConfigGPIO_LowPin(GPIOB,2,2,0);
    /**< I2C_1*/
    //AFIO->MAPR|=AFIO_MAPR_I2C1_REMAP;//I burned pin PB6, so need to remap to SCL/PB8, SDA/PB9
    /**< PB6(8) - I2C1 SCL */
    /**< PB7(9) - I2C1 SDA */
    ConfigGPIO_LowPin(GPIOB,6,1,3);
    ConfigGPIO_LowPin(GPIOB,7,1,3);
    /**< I2C_2*/
    /**< PB10   - I2C1 SCL */
    /**< PB11   - I2C1 SDA */
    ConfigGPIO_HighPin(GPIOB,10,1,3);
    ConfigGPIO_HighPin(GPIOB,11,1,3);
    /**< USART2*/
    /**< PA2   - USART2 Tx */
    /**< PA3   - USART2 Rx */
    ConfigGPIO_LowPin(GPIOA,2,2,2);
    ConfigGPIO_LowPin(GPIOA,3,0,2);
    GPIOA->ODR |= 1 << 3;//pull-up pin PA3 in input mode
}
//****************************************************************************************************************
int16_t bswap16(uint16_t x) {
    return (x >> 8) | (x << 8);
}
//****************************************************************************************************************
typedef struct {
    int32_t avrg_arr[32];//here and in all similar below 32 array is for debug test only. In working it will not required!!!
    int32_t _avrg_acc;
    int32_t rectif_arr[32];
    int32_t _rectif_acc;
    uint16_t samp_n_arr[32];
    int16_t _samp_i;
    int16_t _arr_idx;
    int16_t _adc_skipped;
    int16_t _adc_p;
    uint8_t sub_addr;
    }adc_data_st;

adc_data_st adc_data_arr[8]={
    {.sub_addr=2,},
    {.sub_addr=3,},
    {.sub_addr=0,},
    {.sub_addr=1,},
    {.sub_addr=2,},
    {.sub_addr=3,},
    {.sub_addr=0,},
    {.sub_addr=1,},
};

int main(void) {
    char uart_buf[256];
    InitPeripheryPower();
    SysTickInit();
    GPIO_Init();
    I2C_Init(I2C1,I2C_BUS_FREQ_KHZ);
    I2C_Init(I2C2,I2C_BUS_FREQ_KHZ);
    uint32_t sec_tick=0;
    uint32_t adc_tick=0;
    uint16_t cfg_dc=bswap16(ADS1115_CFG_DR_860SPS | ADS1115_CFG_MODE_CONT | ADS1115_CFG_PGA_2_048V | ADS1115_CFG_MUX_A2A3 | ADS1115_CFG_COMP_QUE_DIS);
    uint16_t cfg_ac=bswap16(ADS1115_CFG_DR_860SPS | ADS1115_CFG_MODE_CONT | ADS1115_CFG_PGA_2_048V | ADS1115_CFG_MUX_A0A1 | ADS1115_CFG_COMP_QUE_DIS);
    for(int i=0;i<8;i++){
        adc_data_arr[i]._avrg_acc=adc_data_arr[i]._rectif_acc=0;
        adc_data_arr[i]._samp_i=adc_data_arr[i]._arr_idx=adc_data_arr[i]._adc_skipped=adc_data_arr[i]._adc_p=0;
        }
    //while(1){
    //    ADS115_ConfigWr(2,adc_data_arr[4].sub_addr,&cfg_ac);
    //    while(!IsSysTickTimerExp(&sec_tick,250));
    //}
    for(int i=0;i<4;i++){
        uint16_t cfg= i==0 ? cfg_dc : cfg_ac;
        ADS115_ConfigWr(1,adc_data_arr[i].sub_addr,&cfg);
        ADS115_ConfigWr(2,adc_data_arr[i+4].sub_addr,&cfg_ac);
        while(I2C_IsBusy(1) || I2C_IsBusy(2));
        ADS115_PrepRdConv(1,adc_data_arr[i].sub_addr);
        ADS115_PrepRdConv(2,adc_data_arr[i+4].sub_addr);
        while(I2C_IsBusy(1) || I2C_IsBusy(2));
    }
    ADC_SampleTimerInit();
    USART2init();
    while(1) {
        if(IsSysTickTimerExp(&sec_tick,250)){
            if(!adc_data_arr[0]._arr_idx || 1){
                GPIOB->ODR ^= 1 << 2;
            }
        }
        if(IsSysTickTimerExp(&adc_tick,1000)){
            //int adc=(long)bswap16(adc_data)*2048l*12/32768;//Vref=2.048V,(R1+R2)/R2=12,Nadc=16bit(2s complement)
            //int adc=bswap16(adc_data);//*2048l*12/32768;
            //printf("adc=0x%x\n",adc);
            //for(int i=0;i<4;i++){
            //    printf("adc%d=%d\t",i,adc_data_arr[i].rectif_arr[0]);
            //}      
            //printf("\n");
            uint32_t len=0;
            for(int i=0;i<8;i++){
                len+=sprintf(&uart_buf[len],"adc%d=%d\t",i,adc_data_arr[i].rectif_arr[0]);
            }      
            len+=sprintf(&uart_buf[len],"\r\n");
            USART2TransmitBuf(uart_buf,len);
            //printf("%s",uart_buf);
        }
   }
}
//****************************************************************************************************************
#define ADC_CH_TOTAL_NUMBER (4)
#define ADC_SPS_RATE (860)
void ADC_SampleTimerInit(void){  
    uint32_t devider;
    TIM4->CR1=0/*TIM_CR1_CEN|TIM_CR1_ARPE | (TIM_CR1_CKD&(0<<8)) | (TIM_CR1_CMS&(2<<5)) | TIM_CR1_DIR | TIM_CR1_OPM|TIM_CR1_UDIS|TIM_CR1_URS*/;
    TIM4->CR2=0/*TIM_CR2_CCDS|TIM_CR2_CCPC|TIM_CR2_CCUS|(TIM_CR2_MMS&(2<<TIM_CR2_MMS_Pos))|TIM_CR2_OIS1|TIM_CR2_OIS1N|TIM_CR2_OIS2
              |TIM_CR2_OIS2N|TIM_CR2_OIS3|TIM_CR2_OIS3N|TIM_CR2_OIS4|TIM_CR2_TI1S*/;                                
    devider=SystemCoreClock/(ADC_CH_TOTAL_NUMBER*ADC_SPS_RATE*1.5);                                  
    TIM4->PSC=(uint16_t)(devider >> 16);
    devider/=(TIM4->PSC+1);
    TIM4->ARR=(uint16_t)(devider & 0xffff);
    TIM4->DIER=0 /*TIM_DIER_BIE|TIM_DIER_CC1DE|TIM_DIER_CC1IE|TIM_DIER_CC2DE|TIM_DIER_CC2IE|TIM_DIER_CC3DE
            |TIM_DIER_CC3IE|TIM_DIER_CC4DE|TIM_DIER_CC4IE|TIM_DIER_COMDE|TIM_DIER_COMIE|TIM_DIER_TDE
            |TIM_DIER_TIE|TIM_DIER_UDE*/|TIM_DIER_UIE;
    TIM4->CCR1=0;//TIM4->ARR>>1;//any value bellow upper bound
    TIM4->CCMR1=0 /*| TIM_CCMR1_OC1CE | (TIM_CCMR1_OC1M & (0x3<<4))  | TIM_CCMR1_OC1PE  | TIM_CCMR1_OC1FE | (TIM_CCMR1_CC1S& (0<<0))
                    |   TIM_CCMR1_OC2CE | (TIM_CCMR1_OC2M & (0x0<<12)) | TIM_CCMR1_OC2PE  | TIM_CCMR1_OC2FE | (TIM_CCMR1_CC2S& (0<<8))*/;
    TIM4->CCMR2=0 /*|TIM_CCMR2_OC3CE | (TIM_CCMR2_OC3M & (0x0<<4))  | TIM_CCMR2_OC3PE  | TIM_CCMR2_OC3FE | (TIM_CCMR2_CC3S& (0<<0))
                    |   TIM_CCMR2_OC4CE | (TIM_CCMR2_OC4M & (0x0<<12)) | TIM_CCMR2_OC4PE  | TIM_CCMR2_OC4FE| (TIM_CCMR2_CC4S& (0<<8))*/;
    TIM4->CCER=0/*TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E
                    |  TIM_CCER_CC1P | TIM_CCER_CC2P | TIM_CCER_CC3P | TIM_CCER_CC4P */;
    TIM4->CR1|=TIM_CR1_CEN;
    TIM4->EGR=0/*TIM_EGR_BG|TIM_EGR_CC1G|TIM_EGR_CC2G|TIM_EGR_CC3G|TIM_EGR_CC4G|TIM_EGR_COMG|TIM_EGR_TG|TIM_EGR_UG*/;

    NVIC_SetPriority(TIM4_IRQn,1);//
    NVIC_EnableIRQ(TIM4_IRQn);
}                                                                       
//****************************************************************************************************************

#define abs(x) (x<0?-x:x)
//check if sign of two values are different
#define zerro_cross(x,y) (((x^y) & 0x8000)!=0)
//maximum averaging window stretching to end of current half period (50Hz) + 30%
#define ADC_WIN_STRACH_MAX (ADC_SPS_RATE/50*13/10/2)
//window is 100ms
#define ADC_AVARAGING_WINDOW (ADC_SPS_RATE/9)
int16_t adc_samp[2];
void TIM4_IRQHandler(void){
    uint16_t status=TIM4->SR;
    static uint8_t adc_ch_curr=0;
    uint8_t ch_curr_next;
    adc_data_st *adc_st_ptr;
    ch_curr_next= (adc_ch_curr==3) ? 0 : (adc_ch_curr+1);
    if(status & TIM_SR_UIF){
        for(int i=0;i<2;i++){
            adc_st_ptr=&adc_data_arr[adc_ch_curr+i*4];
            int16_t adc=bswap16(adc_samp[i]);
            if(adc==adc_st_ptr->_adc_p && !(adc_st_ptr->_adc_skipped)){//probably 
                adc_st_ptr->_adc_skipped=1;//only one missing in row allowed. Next iteration will be processed any way 
            }else{
                adc_st_ptr->_adc_skipped=0;
                //if(++samp_i>=(840)){
                if(++adc_st_ptr->_samp_i>=(ADC_AVARAGING_WINDOW)){
                    if(adc_st_ptr->_samp_i>=(ADC_AVARAGING_WINDOW+ADC_AVARAGING_WINDOW) || zerro_cross(adc,adc_st_ptr->_adc_p)){
                        adc_st_ptr->samp_n_arr[adc_st_ptr->_arr_idx]=adc_st_ptr->_samp_i;
                        adc_st_ptr->avrg_arr[adc_st_ptr->_arr_idx]=adc_st_ptr->_avrg_acc;
                        adc_st_ptr->rectif_arr[adc_st_ptr->_arr_idx]=adc_st_ptr->_rectif_acc/adc_st_ptr->_samp_i;
                        adc_st_ptr->_avrg_acc=0;
                        adc_st_ptr->_rectif_acc=0;
                        adc_st_ptr->_samp_i=0;
                        if(++adc_st_ptr->_arr_idx>=32)adc_st_ptr->_arr_idx=0;
                    }
                }
                adc_st_ptr->_avrg_acc+=adc;
                adc_st_ptr->_rectif_acc+=abs(adc);
                adc_st_ptr->_adc_p=adc;
            }
            ADS1115_RdConv(1+i,adc_data_arr[ch_curr_next].sub_addr,&adc_samp[i]);
        }
        adc_ch_curr=ch_curr_next;
    }//END if(status & TIM_SR_UIF)
    TIM4->SR=~status;
}