/*
 *      adesto.h
 *
 *      Created on: Aug 19, 2019
 *      Author: junaidkhan
 */

#ifndef ADESTO_H_
#define ADESTO_H_

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ssi.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/ssi.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "inc/tm4c123gh6pm.h"
//*****************************************************************************
//
// Adesto(AT) External Flash's Command MACROS
//
//*****************************************************************************

#define AT_WRITE_ENABLE             0x06
#define AT_WRITE_DISABLE            0x04
#define AT_CHIP_ERASE               0xc7
#define AT_READ_STATUS_FORMAT_1     0x05      // 8-bit Status of the Device; Bit[0] = Ready/Busy Status; value: 1 = Device busy, 0 = Device Ready
#define AT_READ_DATA                0x03
#define AT_PAGE_PROGRAM             0x02
#define AT_READ_ID                  0x90
#define AT_BLOCK_ERASE_64           0xd8
#define AT_PAGE_SIZE_256            0x100     // 1 Page  = 256 Bytes
#define AT_BLOCK_SIZE_64            0x10000   // 1 Block = 64  KiloBytes
//*****************************************************************************
//
// Adesto External Flash's Interface Functions
//
//*****************************************************************************


void UART_Init(void);  // For Testing Purposes
void ChipSelect(uint32_t value);
void ReadId(void);
void PageWrite(uint32_t startAddress, uint32_t numberOfBytes, uint8_t *Data);
void DeviceBusyDelay(void);
void WriteToFlash(uint32_t startAddress, uint32_t numberOfBytes, uint8_t *Data);
void ReadFlash(uint32_t startAddress, uint32_t numberOfBytes, uint8_t *DataRx);

#endif /* ADESTO_H_ */
