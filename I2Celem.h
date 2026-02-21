#ifndef I2CELEM_H_INCLUDED
#define I2CELEM_H_INCLUDED
#include "main.h"

#ifdef __cplusplus
 extern "C" {
#endif
#define I2C_BUS_FREQ_KHZ 400
#define MAX_INTERNAL_ADDR_LENGTH 1
#define MIN_BUS_NUM 1
#define MAX_BUS_NUM 2

void I2C_Init(I2C_TypeDef *I2Cx,int f_bus_kHz);
typedef enum {WR,RD}tI2Cdir_en;
/*****************************************************************************
 * @brief Transmit or receive data on I2C bus
 * @details 
 * @param bus_num - MCU periphery bus number, usually 1 or 2 (I2C1 or I2C2)
 * @param dev_addr - device address on bus. First byte of I2C transaction.
 *          only 7-bit address mode released
 * @param addr - address of cell or register inside bus device. Can be NULL or 
 *          array of size up to 'addr_length' length. Inside function this address
 *          is copied to internal buffer. 
 *          NOTE: address is stored in little endian format, 
 *          i.e. addr[0] -> LSB, addr[addr_length-1] -> MSB
 *          This array transmitted in order: addr[addr_length-1],addr[addr_length-2]...addr[0] 
 * @param addr_length - length in bytes of internal address array. 
 *           addr_length<=MAX_INTERNAL_ADDR_LENGTH. If addr_length is 0 than 
 *           address not transmitted at all and immediately after 'dev_addr' will sent begins
 *           'data' transmitting
 * @param data - pointer to buffer to store received data or source of data to be 
 *           transmitted. CANNOT be empty.
 *           NOTE: buffer MUST be consistent until end of transmitting!!!
 * @param data_length - amount of bytes to be transmit/receive. 'data' MUST point to 
 *           buffer of at list 'data_length'.  MUST be more than 0! 
 *           NOTE: 'data_length' and 'addr_length' cannot be 0 at same time! 
 * @param dir - direction of transaction: RD - receive data, WR - transmit data to device
 * @return Return 0 in case of success. Non zero otherwise.
 ******************************************************************************/
int I2C_Transfer(int bus_num,uint8_t dev_addr, uint8_t addr[MAX_INTERNAL_ADDR_LENGTH], uint8_t addr_length,uint8_t* data, uint16_t data_length, tI2Cdir_en dir);
/*****************************************************************************
* @brief Check if transaction on selected bus still in progress 
* @param bus_num - MCU periphery bus number, usually 1 or 2 (I2C1 or I2C2)
* @return int8_t - 0 if bus is free, >0 if bus is busy, <0 in case of error                                                                         
******************************************************************************/
int8_t I2C_IsBusy(int bus_num);
/*****************************************************************************
* @brief Check if current transaction completed from last call to this function 
*     or from start of transaction. In case  of return >0 subsequent call this 
*     function return 0.  
* @param bus_num - MCU periphery bus number, usually 1 or 2 (I2C1 or I2C2)
* @return int8_t - 0 if transaction not completed, >0 if completed, <0 in case of error                                                                         
******************************************************************************/
int8_t I2C_IsTransferEnd(int bus_num);



#ifdef __cplusplus
}
#endif

#endif /* I2CELEM_H_INCLUDED */
