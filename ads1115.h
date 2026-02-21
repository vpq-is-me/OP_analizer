#ifndef ADS1115_H
#define ADS1115_H

#ifdef __cplusplus
 extern "C" {
#endif
#include "main.h"
#include "I2Celem.h"

#define ADS1115_ADDR_PTR_Pos (0U)
#define ADS1115_ADDR_PTR_Msk (0x03U << ADS1115_ADDR_PTR_Pos)

#define ADS1115_ADDR_PTR_CONV (0x00U)
#define ADS1115_ADDR_PTR_CFG  (0x01U)
#define ADS1115_ADDR_PTR_LoTH (0x02U)
#define ADS1115_ADDR_PTR_HiTH (0x03U)


#define ADS1115_CFG_COMP_QUE_Pos (0U)
#define ADS1115_CFG_COMP_QUE_Msk (0x03U << ADS1115_CFG_COMP_QUE_Pos)
#define ADS1115_CFG_COMP_QUE ADS1115_CFG_COMP_QUE_Msk
#define ADS1115_CFG_COMP_QUE_ONE (0x00U << ADS1115_CFG_COMP_QUE_Pos)
#define ADS1115_CFG_COMP_QUE_TWO (0x01U << ADS1115_CFG_COMP_QUE_Pos)
#define ADS1115_CFG_COMP_QUE_FOUR (0x02U << ADS1115_CFG_COMP_QUE_Pos)
#define ADS1115_CFG_COMP_QUE_DIS (0x03U << ADS1115_CFG_COMP_QUE_Pos)

#define ADS1115_CFG_COMP_LAT_Pos (2U)
#define ADS1115_CFG_COMP_LAT_Msk (0x01U << ADS1115_CFG_COMP_LAT_Pos)
#define ADS1115_CFG_COMP_LAT ADS1115_CFG_COMP_LAT_Msk

#define ADS1115_CFG_COMP_POL_Pos (3U)
#define ADS1115_CFG_COMP_POL_Msk (0x01U << ADS1115_CFG_COMP_POL_Pos)
#define ADS1115_CFG_COMP_POL ADS1115_CFG_COMP_POL_Msk

#define ADS1115_CFG_COMP_MODE_Pos (4U)
#define ADS1115_CFG_COMP_MODE_Msk (0x01U << ADS1115_CFG_COMP_MODE_Pos)
#define ADS1115_CFG_COMP_MODE ADS1115_CFG_COMP_MODE_Msk

#define ADS1115_CFG_DR_Pos (5U)
#define ADS1115_CFG_DR_Msk (0x07U << ADS1115_CFG_DR_Pos)
#define ADS1115_CFG_DR ADS1115_CFG_DR_Msk
#define ADS1115_CFG_DR_8SPS (0x00 << ADS1115_CFG_DR_Pos)
#define ADS1115_CFG_DR_16SPS (0x01 << ADS1115_CFG_DR_Pos)
#define ADS1115_CFG_DR_32SPS (0x02 << ADS1115_CFG_DR_Pos)
#define ADS1115_CFG_DR_64SPS (0x03 << ADS1115_CFG_DR_Pos)
#define ADS1115_CFG_DR_128SPS (0x04 << ADS1115_CFG_DR_Pos)
#define ADS1115_CFG_DR_250SPS (0x05 << ADS1115_CFG_DR_Pos)
#define ADS1115_CFG_DR_475SPS (0x06 << ADS1115_CFG_DR_Pos)
#define ADS1115_CFG_DR_860SPS (0x07 << ADS1115_CFG_DR_Pos)

#define ADS1115_CFG_MODE_Pos (8U)
#define ADS1115_CFG_MODE_Msk (0x01U << ADS1115_CFG_MODE_Pos)
#define ADS1115_CFG_MODE ADS1115_CFG_MODE_Msk
#define ADS1115_CFG_MODE_CONT (0x0 << ADS1115_CFG_MODE_Pos)
#define ADS1115_CFG_MODE_SNGL (0x1 << ADS1115_CFG_MODE_Pos)

#define ADS1115_CFG_PGA_Pos (9U)
#define ADS1115_CFG_PGA_Msk (0x07U << ADS1115_CFG_PGA_Pos)
#define ADS1115_CFG_PGA ADS1115_CFG_PGA_Msk
#define ADS1115_CFG_PGA_6_144V (0x00 << ADS1115_CFG_PGA_Pos)
#define ADS1115_CFG_PGA_4_096V (0x01 << ADS1115_CFG_PGA_Pos)
#define ADS1115_CFG_PGA_2_048V (0x02 << ADS1115_CFG_PGA_Pos)
#define ADS1115_CFG_PGA_1_024V (0x03 << ADS1115_CFG_PGA_Pos)
#define ADS1115_CFG_PGA_0_512V (0x04 << ADS1115_CFG_PGA_Pos)
#define ADS1115_CFG_PGA_0_256V (0x05 << ADS1115_CFG_PGA_Pos)

#define ADS1115_CFG_MUX_Pos (12U)
#define ADS1115_CFG_MUX_Msk (0x07U << ADS1115_CFG_MUX_Pos)
#define ADS1115_CFG_MUX ADS1115_CFG_MUX_Msk
#define ADS1115_CFG_MUX_A0A1 (0x00 << ADS1115_CFG_MUX_Pos)
#define ADS1115_CFG_MUX_A0A3 (0x01 << ADS1115_CFG_MUX_Pos)
#define ADS1115_CFG_MUX_A1A3 (0x02 << ADS1115_CFG_MUX_Pos)
#define ADS1115_CFG_MUX_A2A3 (0x03 << ADS1115_CFG_MUX_Pos)
#define ADS1115_CFG_MUX_A0G (0x04 << ADS1115_CFG_MUX_Pos)
#define ADS1115_CFG_MUX_A1G (0x05 << ADS1115_CFG_MUX_Pos)
#define ADS1115_CFG_MUX_A2G (0x06 << ADS1115_CFG_MUX_Pos)
#define ADS1115_CFG_MUX_A3G (0x07 << ADS1115_CFG_MUX_Pos)

#define ADS1115_CFG_OS_Pos (15U)
#define ADS1115_CFG_OS_Msk (0x01U << ADS1115_CFG_OS_Pos)
#define ADS1115_CFG_OS ADS1115_CFG_OS_Msk

#define ADS1115_I2C_ADDR_BASE (0x48U)
#define ADS1115_I2C_ADDR_DEV_Msk (0x03U)


/*****************************************************************************
* @brief Configuring ADC ADS1115
* @param bus_num - MCU's I2C bus number, 1,2... (I2C1,I2C2...)
* @param dev_addr - address of device on bus depending of connection ADDR pin.
*           Only 0-3 address range required. Base address of ADS1115 ORed automatically. 
* @param cfg - desired content of CONFIG register contents. Entire register will be
*           updated. Use read-modify-write externally by combining with 'ADS115_ConfigRd'
*           NOTE: keep in mind that little endianness is used by compiler. 
* @return void                                                                         
******************************************************************************/
void ADS115_ConfigWr(int bus_num,int dev_addr,uint16_t* cfg);
/*****************************************************************************
* @brief Read configuration register ADC ADS1115
* @param bus_num - MCU's I2C bus number, 1,2... (I2C1,I2C2...)
* @param dev_addr - address of device on bus depending of connection ADDR pin.
*           Only 0-3 address range required. Base address of ADS1115 ORed automatically. 
* @param dst - destination variable to store content of CONFIG register contents.
*           NOTE: keep in mind that little endianness is used by compiler. 
* @return void                                                                         
******************************************************************************/
void ADS115_ConfigRd(int bus_num,int dev_addr,uint16_t* dst);
/*****************************************************************************
* @brief Set internal address to point to Conversion register. So subsequent
*      reading not required to transmit internal address  
* @param bus_num - MCU's I2C bus number, 1,2... (I2C1,I2C2...)
* @param dev_addr - address of device on bus depending of connection ADDR pin.
*           Only 0-3 address range required. Base address of ADS1115 ORed automatically. 
* @return void                                                                         
******************************************************************************/
void ADS115_PrepRdConv(int bus_num,int dev_addr);
/*****************************************************************************
* @brief Set internal address to point to Conversion register. So subsequent
*      reading not required to transmit internal address  
* @param bus_num - MCU's I2C bus number, 1,2... (I2C1,I2C2...)
* @param dev_addr - address of device on bus depending of connection ADDR pin.
*           Only 0-3 address range required. Base address of ADS1115 ORed automatically. 
* @param dst - address of destination buffer to receive data
* @return void                                                                         
******************************************************************************/
void ADS1115_RdConv(int bus_num,int dev_addr,int16_t* dst);



#ifdef __cplusplus
}
#endif

#endif