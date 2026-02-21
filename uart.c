#include "uart.h"
#include"stm32f103xb.h"

static uint8_t usart2_tx_busy_fg=0;
static uint8_t usart2_tx_completed=0;
static uint8_t usart2_rx_completed=0;
 /*****************************************************************************
* @brief initialize USART2
* @param void
* @return void                                                                         
******************************************************************************/
void USART2init(void){
    /**< initialize DMA  for Tx*/
    DMA1_Channel7->CCR=0;
    DMA1_Channel7->CCR=0/*| DMA_CCR_EN */| DMA_CCR_TCIE/* | DMA_CCR_HTIE | DMA_CCR_TEIE*/| DMA_CCR_DIR /*| DMA_CCR_CIRC | DMA_CCR_PINC*/
                   | DMA_CCR_MINC | (DMA_CCR_PSIZE&(2<<DMA_CCR_PSIZE_Pos)) | (DMA_CCR_MSIZE&(0<<DMA_CCR_MSIZE_Pos))
                   | (DMA_CCR_PL &(1<<DMA_CCR_PL_Pos)) /* | DMA_CCR_MEM2MEM*/;
    DMA1_Channel7->CNDTR=0;/**< set before transfer start */
    DMA1_Channel7->CMAR=0;/**< set before transfer start */
    DMA1_Channel7->CPAR=(uint32_t)&USART2->DR;
    NVIC_SetPriority(DMA1_Channel7_IRQn,IRQ_HIGHEST_PRIOR+2);//highest priority
    NVIC_EnableIRQ(DMA1_Channel7_IRQn);
    /**< initialize DMA  for Rx*/
    //DMA1_Channel5->CCR=0;
    //DMA1_Channel5->CCR=/*DMA_CCR_EN | DMA_CCR_TCIE | DMA_CCR_HTIE | DMA_CCR_TEIE | DMA_CCR_DIR | DMA_CCR_CIRC | DMA_CCR_PINC
    //               |*/ DMA_CCR_MINC | (DMA_CCR_PSIZE&(2<<DMA_CCR_PSIZE_Pos)) | (DMA_CCR_MSIZE&(0<<DMA_CCR_MSIZE_Pos))
    //               | (DMA_CCR_PL &(1<<DMA_CCR_PL_Pos)) /* | DMA_CCR_MEM2MEM*/;
    //DMA1_Channel5->CNDTR=MAXRXSTR;/**<must be reset after processing received data */
    //DMA1_Channel5->CMAR=(uint32_t)(rx_str.buf);
    //DMA1_Channel5->CPAR=(uint32_t)&USART1->RDR;
    //DMA1_Channel5->CCR|=DMA_CCR_EN;
    /**< initialize USART */
    USART2->CR1=0;//disable at first
    USART2->BRR=(UART2_CORE_CLK_FREQ+BAUDRATE/2)/BAUDRATE;
    USART2->CR2=0/*| USART_CR2_ADDM7 | USART_CR2_LBDL | USART_CR2_LBDIE | USART_CR2_LBCL | USART_CR2_CPHA | USART_CR2_CPOL
          | USART_CR2_CLKEN | USART_CR2_STOP | USART_CR2_LINEN | USART_CR2_SWAP | USART_CR2_RXINV | USART_CR2_TXINV
          | USART_CR2_DATAINV | USART_CR2_MSBFIRST | USART_CR2_ABREN | USART_CR2_ABRMODE | USART_CR2_RTOEN | (USART_CR2_ADD_Msk &('\n'<USART_CR2_ADD_Pos))*/;
    USART2->CR3=0/*| USART_CR3_EIE | USART_CR3_IREN | USART_CR3_IRLP | USART_CR3_HDSEL | USART_CR3_NACK | USART_CR3_SCEN
              | USART_CR3_DMAR*/ | USART_CR3_DMAT /*| USART_CR3_RTSE | USART_CR3_CTSE | USART_CR3_CTSIE | USART_CR3_ONEBIT
              | USART_CR3_OVRDIS | USART_CR3_DDRE | USART_CR3_DEM | USART_CR3_DEP | USART_CR3_SCARCNT | USART_CR3_WUS | USART_CR3_WUFIE*/;
    USART2->GTPR=0;
    USART2->CR1=USART_CR1_UE |/* USART_CR1_UESM |*/ USART_CR1_RE | USART_CR1_TE /*| USART_CR1_IDLEIE | USART_CR1_RXNEIE*/
          /*| USART_CR1_TCIE | USART_CR1_TXEIE | USART_CR1_PEIE | USART_CR1_PS | USART_CR1_PCE | USART_CR1_WAKE
          | USART_CR1_M0 | USART_CR1_MME | USART_CR1_CMIE | USART_CR1_OVER8 | USART_CR1_DEDT
          | USART_CR1_DEAT | USART_CR1_RTOIE | USART_CR1_EOBIE*/;
    NVIC_SetPriority(USART2_IRQn,IRQ_LOWEST_PRIOR);//lowest priority
    NVIC_EnableIRQ(USART2_IRQn);
}
 /*****************************************************************************
* @brief Prepare USART2 and associated DMA1 (channel 7) for transmitting
*         contents of buffer 'buf' which counts 'length' bytes
* @param length uint32_t - bytes counts to transmit
* @param buf char* - data buffer to transmit 
* @return '0' in case of success, nonzero - otherwise                                                                           
******************************************************************************/
int USART2TransmitBuf(char* buf,uint32_t length){
    if(usart2_tx_busy_fg)return -1;
    DMA1_Channel7->CCR&=~DMA_CCR_EN;
    usart2_tx_busy_fg=1;
    DMA1_Channel7->CNDTR=length;
    DMA1_Channel7->CMAR=(uint32_t)buf;
    DMA1_Channel7->CCR|=DMA_CCR_EN;
    return 0;
}
/*****************************************************************************
* @brief USART2 Tx complete interrupt (by DMA1 channel 7)
* @return void
******************************************************************************/
void DMA1_Channel7_IRQHandler(void){
    uint32_t status=DMA1->ISR;
    if(status | (0x0001U << 24)){// Channel 7 global interrupt flag
        usart2_tx_busy_fg=0;
        usart2_tx_completed=1;
    }
    DMA1->IFCR = (0x0001U << 24); //clear DMA ch 7 interrupt flag
}
/** \brief  USART interrupt routine
 *
 * \param void
 * \return void
 *
 */
void USART2_IRQHandler(void){
    uint32_t status;
    status=USART2->SR;
    /**< Character Match Flag, received '\n'*/
    /**< Receiver Timeout flag, predefined timeout from last received char and last transaction have't processed already in USART_ISR_CMF*/
    //if((status & USART_SR_CMF) ||
    //   ((status & USART_ISR_RTOF) && (DMA1_Channel5->CNDTR!=MAXRXSTR))){
    //    DMA1_Channel5->CCR&=~DMA_CCR_EN;
    //    rx_str.length=MAXRXSTR - DMA1_Channel5->CNDTR;
    //    DMA1_Channel5->CNDTR=MAXRXSTR;
    //    DMA1_Channel5->CCR|=DMA_CCR_EN;
    //    usart1_rx_completed=1;
    //}
    //USART1->ICR=status;
}
uint8_t USART2IsTxBusy(void){
    return usart2_tx_busy_fg;
}
uint8_t USART2IsTxCompleted(void){
    uint8_t res=usart2_tx_completed;
    if(res)usart2_tx_completed=0;
    return res;
}
/** \brief Check was received any sentence from last call this function
 *
 * \param void
 * \return uint8_t 0 - if there was no any receiptions, 1- otherwise
 *
 */
//uint8_t USART2IsRxCompleted(void){
//    uint8_t res=usart1_rx_completed;
//    if(res)usart1_rx_completed=0;
//    return res;
//}
