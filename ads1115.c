#include "ads1115.h"
#define ADS_DADD(x) (ADS1115_I2C_ADDR_BASE | (x & ADS1115_I2C_ADDR_DEV_Msk))
//*****************************************************************************
void ADS115_ConfigWr(int bus_num,int dev_addr,uint16_t* cfg){
    uint8_t reg_addr=ADS1115_ADDR_PTR_CFG;
    I2C_Transfer(bus_num,ADS_DADD(dev_addr), &reg_addr, 1,(uint8_t*)cfg, 2, WR);
}
//*****************************************************************************
void ADS115_ConfigRd(int bus_num,int dev_addr,uint16_t* dst){
    uint8_t reg_addr=ADS1115_ADDR_PTR_CFG;
    I2C_Transfer(bus_num,ADS_DADD(dev_addr), &reg_addr, 1,(uint8_t*)dst, 2, RD);
}
//*****************************************************************************
void ADS115_PrepRdConv(int bus_num,int dev_addr){
    uint8_t reg_addr=ADS1115_ADDR_PTR_CONV;
    I2C_Transfer(bus_num,ADS_DADD(dev_addr), &reg_addr, 1,NULL, 0, WR);    
}
//*****************************************************************************
void ADS1115_RdConv(int bus_num,int dev_addr,int16_t* dst){
    I2C_Transfer(bus_num,ADS_DADD(dev_addr), NULL, 0,(uint8_t*)dst, 2, RD);
}