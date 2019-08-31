/*
 *      main.c
 *
 *      Created on: Jul 28, 2019
 *      Author: junaidkhan
 */

#include "at_spi.h"
#include "adesto.h"

/*
 * SPI Pin Configuration
 *
 * PA2 -> SSI0_CLK
 * PA4 -> SSI0_RX
 * PA5 <- SSI0_TX
 * PA3 -> SSI0_CS
 */

/*
 *  UART Pin Configuration
 *
 *  PA0 -> UART0_RX
 *  PA1 <- UART0_TX
 */

#define SSI_DATA    8  // The number of bytes to send and receive

int main()
{

       uint8_t pui8DataRx[SSI_DATA]; // The dummy data recieved from the External Flash

       SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL |SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

       // Initialize peripherals
       UART_Init();
       SPI_Init();

       // Confirm Communicating with External Flash
       ReadId();
       // Read From a 32-bit address and store result into pui32Da
       ReadFlash(0x000100, SSI_DATA, pui8DataRx);

       ReadFlash(0x3FFF00, SSI_DATA, pui8DataRx);

       uint8_t pui8DataTx[] = {0x34, 0xFF, 0x12, 0x23, 0x45, 0xAB, 0xDF, 0xEF} ; // Transmission Commands for the External Flash

       EraseFlash(0x3FFF00, SSI_DATA);
       // Write to Specific Address on the Flash
       WriteToFlash(0x3FFF00,  SSI_DATA,  pui8DataTx);

       ReadFlash(0x3FFF00, SSI_DATA, pui8DataRx);

       return 0;

}



