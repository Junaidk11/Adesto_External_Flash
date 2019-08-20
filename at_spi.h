/*
 * at_spi.h
 *
 *      Created on: Aug 19, 2019
 *      Author: junaidkhan
 */

#ifndef AT_SPI_H_
#define AT_SPI_H_

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
//  Interface Macros for SPI Com b/w Master & Adesto Flash Memory
//
//*****************************************************************************
#define SSI_DATA    8   // Transfer bits


//*****************************************************************************
//
//  Interface Function Declarations for SPI Com b/w Master & Adesto Flash Memory
//
//*****************************************************************************

void SPIInit(void);
uint8_t TransferByte(uint8_t Byte);
void SSI0IntHandler(void);

#endif /* AT_SPI_H_ */
