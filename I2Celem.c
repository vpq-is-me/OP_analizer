#include "I2Celem.h"
#ifndef NULL
#define NULL 0
#endif

typedef enum {IDLE,START,DEV_ADDR,INT_ADDR,RESTART,RE_DEV_ADDR,DATA_WR,DATA_RD,STOP}tI2C_state_en;
typedef struct {
    uint8_t dev_addr;                         //device address on I2C bus
    uint8_t add_length;                       //length in bytes of device internal address
    uint8_t in_addr[MAX_INTERNAL_ADDR_LENGTH];//internal address,e.g. address of memory cell to write to or register address, etc.
    uint16_t data_length;                     //data length to write or read in bytes (address bytes not included)
    uint8_t* buf;                             //pointer to buffer to store data to during I2C reception operation or buffer from which will taken data to transmit to I2C device
    tI2Cdir_en RD_WR;                         //direction of transmitting data RD - receiving from device, WR - transmit to I2C device
    uint8_t end_fg;                           //flag about end of transaction
    uint8_t busy_fg;                          //flag about transmit in progress 
    I2C_TypeDef *i2c_periph;                  //pointer to I2C periphery registers struct  
    tI2C_state_en state;                      //state-machine position
    uint8_t in_error;                         //flag - transaction fail with error
    uint16_t itr_length;                      //internally used current remain transmit length (address or data fields)
}tI2C_transfer_st;

static tI2C_transfer_st I2C1_struct={
    .dev_addr=0x50,
    .in_addr={0},/**< LSB at [0] */
    .add_length=1,
    .data_length=1,
    .RD_WR=RD,
    .buf=NULL,
    .end_fg=0,
    .busy_fg=0,
    .i2c_periph=I2C1,
    .state=IDLE,
    .in_error=0
};
static tI2C_transfer_st I2C2_struct={
    .dev_addr=0x50,
    .in_addr={0},/**< LSB at [0] */
    .add_length=1,
    .data_length=1,
    .RD_WR=RD,
    .buf=NULL,
    .end_fg=0,
    .busy_fg=0,
    .i2c_periph=I2C2,
    .state=IDLE,
    .in_error=0
};
//*****************************************************************************
/** \brief Initialize I2C periphery in SDANDARD(up to 100kHz)or FAST(up to 400kHz) speed mode 
 *   NOTE about APB1 frequency from initializing. E.g. RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE1_DIV2;
 *        set APB1 to SystemCoreClock/2=36MHz check it 
 *        and set definition in I2Celem.h APB1_CLOCK_FREQ accordingly
 * \param i2c I2C_TypeDef* select I2C1 or I2C2 module
 * \param f_bus int desired bus speed in kHz. Notice to set speed not more 100kHz(400kHz)
 * \return void
 *
 */ 
void I2C_Init(I2C_TypeDef *I2Cx,int f_bus_kHz){
    I2Cx->CR1= 0;
    if(f_bus_kHz<=100){//add "1000*f_bus_kHz-1" to round up to next integer
      I2Cx->CCR= 0 | (((APB1_CLOCK_FREQ+1000ul*f_bus_kHz-1)/(1000ul*f_bus_kHz*2)<<I2C_CCR_CCR_Pos)&I2C_CCR_CCR_Msk) /*| I2C_CCR_DUTY | I2C_CCR_FS*/;
      I2Cx->TRISE=APB1_CLOCK_FREQ/1000000ul+1;/**< 1000ns for Standard mode->1000000 */
    }else{
      I2Cx->CCR= 0 | (((APB1_CLOCK_FREQ+1000ul*f_bus_kHz*25-1)/(1000ul*f_bus_kHz*25)<<I2C_CCR_CCR_Pos)&I2C_CCR_CCR_Msk) | I2C_CCR_DUTY | I2C_CCR_FS;
      I2Cx->TRISE=APB1_CLOCK_FREQ/3333333ul+1;/**< 300ns ? for Fast mode-> 3333333*/
    }
    I2Cx->CR1= 0 | I2C_CR1_PE /*| I2C_CR1_SMBUS | I2C_CR1_SMBTYPE | I2C_CR1_ENARP | I2C_CR1_ENPEC | I2C_CR1_ENGC
                | I2C_CR1_NOSTRETCH | I2C_CR1_START | I2C_CR1_STOP | I2C_CR1_ACK | I2C_CR1_POS | I2C_CR1_PEC
                | I2C_CR1_ALERT | I2C_CR1_SWRST*/;
    I2Cx->CR2=(((APB1_CLOCK_FREQ/1000000ul)<<I2C_CR2_FREQ_Pos)&I2C_CR2_FREQ_Msk) | I2C_CR2_ITERREN | I2C_CR2_ITEVTEN | I2C_CR2_ITBUFEN
                /*| I2C_CR2_DMAEN | I2C_CR2_LAST*/;
    I2Cx->OAR1=0 /*| I2C_OAR1_ADDMODE*/;
    I2Cx->OAR2= 0 /*| I2C_OAR2_ENDUAL | ((0<I2C_OAR2_ADD2_Pos)&I2C_OAR2_ADD2_Msk)*/;
    if(I2Cx==I2C1){
        NVIC_SetPriority(I2C1_EV_IRQn,0);
	NVIC_EnableIRQ(I2C1_EV_IRQn);
    }else if(I2Cx==I2C2){
        NVIC_SetPriority(I2C2_EV_IRQn,0);
	NVIC_EnableIRQ(I2C2_EV_IRQn);
    }
#warning TODO (Vladimir#1#): Add ISR for error proccesing
}
//*****************************************************************************
static void I2C_SetError(tI2C_transfer_st *i2c){
    i2c->in_error=1;
    i2c->i2c_periph->CR1=I2C_CR1_SWRST;
}
//*****************************************************************************
/*NOTE if it will require to transmit only device address with data_length and addr_length==0
* then in transfer function just write to DR, set state=DATA_WR to waiting finish transmitting byte and
* allow only I2C_CR2_ITEVTEN but not I2C_CR2_ITBUFEN
*/
void I2C_transfer_itl(tI2C_transfer_st *i2c){
#warning TODO (Vladimir#1#): if bus hang up oscillate on bus  until SDA return to 1 and 'stop' manually!!!
    if(i2c->i2c_periph->SR2 & I2C_SR2_BUSY)I2C_SetError(i2c);// !!!
    if(i2c->in_error){
        I2C_Init(i2c->i2c_periph,I2C_BUS_FREQ_KHZ);
        i2c->in_error=0;
    }
    i2c->busy_fg=1;
    i2c->end_fg=0;
    i2c->state=START;
    i2c->i2c_periph->CR2|=I2C_CR2_ITBUFEN;//I2C_CR2_ITBUFEN was reset at previous transaction
    i2c->i2c_periph->CR1|=I2C_CR1_START;
}

void I2C_EV_IRQHandlerCommon(tI2C_transfer_st *i2c){
    I2C_TypeDef*I2Cx=i2c->i2c_periph; 
    uint16_t status1=I2Cx->SR1;
    uint16_t* length=&i2c->itr_length;
#warning TODO (Vladimir#1#): check ARLO,and other flags
    switch (i2c->state){
    case START://SB
        if(!(status1&I2C_SR1_SB)){I2C_SetError(i2c); break;}
        i2c->state=DEV_ADDR;
        if(i2c->add_length)I2Cx->DR=(i2c->dev_addr<<1) | 0;//always WR because it need internal address
        else I2Cx->DR=(i2c->dev_addr<<1) | (i2c->RD_WR==RD?1:0);
        break;
    case DEV_ADDR://ADDR
        if(!(status1&I2C_SR1_ADDR)){I2C_SetError(i2c); break;}
        (void)I2Cx->SR2;//read SR1 followed by reading SR2 clear ADDR flag in SR1
        if(i2c->add_length){
            *length=i2c->add_length-1;
            i2c->state=INT_ADDR;
            I2Cx->DR=i2c->in_addr[*length];
            /**< Comment out following 'break' purposely. So we fill shifting register at first and than immediately fill data buffer register  */
            // break;
        }else{
            if(i2c->RD_WR==WR){
                *length=i2c->data_length-1;
                i2c->state=DATA_WR;
                I2Cx->DR=*i2c->buf++;
            }else{//read
                *length=i2c->data_length;
                if(*length>1)I2Cx->CR1|=I2C_CR1_ACK;
                if(*length==1)I2Cx->CR1|=I2C_CR1_STOP;
                i2c->state=DATA_RD;
            }
            break;
        }
        /**< NO 'break' see above! */
    case INT_ADDR://TXE
        if(!(status1&I2C_SR1_TXE) || (status1&I2C_SR1_AF)){I2C_SetError(i2c); break;}
        if(*length){
            *length=*length-1;
            I2Cx->DR=i2c->in_addr[*length];
        }else {//internal address transmitted. Start data transfer, read or write.
            if(i2c->RD_WR==WR){
                *length=i2c->data_length-1;
                i2c->state=DATA_WR;
                I2Cx->DR=*i2c->buf++;
            }else{//if required 'read' but now it was in 'write' internal address
                i2c->state=RESTART;
                I2Cx->CR2&=~I2C_CR2_ITBUFEN;//disable TXE flag interrupt
                I2Cx->CR1|=I2C_CR1_START;
                I2Cx->DR=0x55;//dummy fill register to reset TxE and TBF until 'start' finished
            }
        }
        break;
    case DATA_WR://TxE or BTF
        if(status1&I2C_SR1_AF){I2C_SetError(i2c); break;}
        if(*length){
            *length=*length-1;
            I2Cx->DR=*i2c->buf++;
        }else{//before or after last byte transmit
            if(I2Cx->CR2&I2C_CR2_ITBUFEN){//last byte moved from data register to shift register. Disable TXE flag and wait BTF flag
                I2Cx->CR2&=~I2C_CR2_ITBUFEN;//disable TXE flag interrupt
            }else {
                I2Cx->CR1|=I2C_CR1_STOP;
                i2c->end_fg=1;
                i2c->busy_fg=0;
                i2c->state=IDLE;
            }
        }
        break;
    case RESTART://SB
        if(!(status1&I2C_SR1_SB)){/*I2C_SetError(i2c);*/ break;}//sometimes may reach this point with another flag until 'start' finished
        i2c->state=RE_DEV_ADDR;
        *length=i2c->data_length;
        I2Cx->CR2|=I2C_CR2_ITBUFEN;//re-enable RxE (and Txe)
        if(*length>1)I2Cx->CR1|=I2C_CR1_ACK;
        I2Cx->DR=(i2c->dev_addr<<1) | 1;//always RD because it may be here only at 'read'
        break;
    case RE_DEV_ADDR://ADDR
        if(!(status1&I2C_SR1_ADDR)){I2C_SetError(i2c); break;}
        i2c->state=DATA_RD;
        (void)I2Cx->SR2;//read SR1 followed by reading SR2 clear ADDR flag in SR1
        if(*length==1)I2Cx->CR1|=I2C_CR1_STOP;
        break;
    case DATA_RD://RxNE or BTF
        *i2c->buf++=I2Cx->DR;
        *length=*length-1;
        if(*length==0){//last byte received followed by 'Stop',
            i2c->state=IDLE;
            i2c->end_fg=1;
            i2c->busy_fg=0;
        }else if(*length==1){//second last byte received, prepare NACK and Stop
            I2Cx->CR1&=~I2C_CR1_ACK;
            I2Cx->CR1|=I2C_CR1_STOP;
        }
        break;
    default:
        I2C_SetError(i2c);
        break;
    }//END switch
}

void I2C1_ER_IRQHandler(void){
}
void I2C2_ER_IRQHandler(void){
}
void I2C1_EV_IRQHandler(void){
  I2C_EV_IRQHandlerCommon(&I2C1_struct);
}
void I2C2_EV_IRQHandler(void){
  I2C_EV_IRQHandlerCommon(&I2C2_struct);
}
int I2C_Transfer(int bus_num,uint8_t dev_addr, uint8_t addr[MAX_INTERNAL_ADDR_LENGTH], uint8_t addr_length,uint8_t* data, uint16_t data_length, tI2Cdir_en dir){
    tI2C_transfer_st* i2c;
    if(bus_num<MIN_BUS_NUM || bus_num>MAX_BUS_NUM )return -1;
    if(addr_length==0 && data_length==0) return -1;//In this release it is not allowed. If it will require see NOTE above 'I2C_transfer_itl'
    if(bus_num==1)i2c=&I2C1_struct;
    else if(bus_num==2)i2c=&I2C2_struct;
    else return -1;
    i2c->dev_addr=dev_addr;
    if(addr_length){
        if(addr_length>MAX_INTERNAL_ADDR_LENGTH)return -1;
        for(int i=0;i<addr_length;i++)
            {i2c->in_addr[i]=addr[i];}
        i2c->add_length=addr_length;
    }
    i2c->RD_WR=dir;
    if(data_length){   
        i2c->buf=data;
        i2c->data_length=data_length;
    }else{//trick in case only address has to be transmitted. So we transmit 'address' as 'data' with address length =0
        i2c->buf=i2c->in_addr;
        i2c->data_length=i2c->add_length;
        i2c->add_length=0;
        i2c->RD_WR=WR;//only WR allowed any way
    }     
    I2C_transfer_itl(i2c);
    return 0;
}
//*****************************************************************************
int8_t I2C_IsBusy(int bus_num){
    if(bus_num<MIN_BUS_NUM || bus_num>MAX_BUS_NUM )return -1;
    if(bus_num==1)return I2C1_struct.busy_fg;
    else if(bus_num==2)return I2C2_struct.busy_fg;
    else return -1;
}
//*****************************************************************************
int8_t I2C_IsTransferEnd(int bus_num){
    int8_t res;
    tI2C_transfer_st* i2c;
    if(bus_num<MIN_BUS_NUM || bus_num>MAX_BUS_NUM )return -1;
    if(bus_num==1)i2c=&I2C1_struct;
    else if(bus_num==2)i2c=&I2C2_struct;
    else return -1;
    res=i2c->end_fg;
    if(res)i2c->end_fg=0;
    return res;
}
