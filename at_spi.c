/*
 * at_spi.c
 *
 *  Created on: Aug 19, 2019
 *  Author: junaidkhan
 */

#include "at_spi.h"

uint32_t pui32DataRx = 0; // Data Received Stored here.
volatile uint32_t InterruptWait =0;

/*
 * SPI Pin Configuration
 *
 * PA2 -> SSI0_CLK
 * PA4 -> SSI0_RX
 * PA5 <- SSI0_TX
 * PA3 -> SSI0_CS
 */

/*
 * SPI_INIT Definition
 * Initialize SPI Module 0 for Serial communication
 */


void SPIInit(void)
{

     // Enable clock access to SPI Module 0
     SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);

     // Wait for the Peripheral to be ready for configuration
     while (!SysCtlPeripheralReady(SYSCTL_PERIPH_SSI0));

     // Enable clock access to GPIO Port A
     SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

     // Wait for the GPIOA Peripheral to be ready for configuration
     while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));

     // SSI pin configuration
     GPIOPinConfigure(GPIO_PA2_SSI0CLK);
     GPIOPinConfigure(GPIO_PA4_SSI0RX);
     GPIOPinConfigure(GPIO_PA5_SSI0TX);
     GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_5 | GPIO_PIN_4 | GPIO_PIN_2);

     // Configure SSI operation mode
     SSIConfigSetExpClk(SSI0_BASE,SysCtlClockGet(),SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER,1000000,SSI_DATA);

     // SSI CS pin configuration
     GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_3);
     GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_3, GPIO_PIN_3); //CS set to inactive HIGH

     // Enable SSI0 Module
     SSIEnable(SSI0_BASE);

     // Read any residual data from the SSI0 Port
     while (SSIDataGetNonBlocking(SSI0_BASE, &pui32DataRx));

     // Disable and clear Interrupt flagss
     SSIIntDisable(SSI0_BASE, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);
     SSIIntClear(SSI0_BASE, SSI_TXFF | SSI_RXFF | SSI_RXTO | SSI_RXOR);

     // Enable End of Transmit Interrupt mode for the TXRIS interrupt.
     HWREG(SSI0_BASE + SSI_O_CR1) |= SSI_CR1_EOT;

     // Enable the SSI interrupt.
     IntEnable(INT_SSI0);

     // Enable processor interrupts.
     IntMasterEnable();
 }

/* SPI Transfer Byte Definition
 * @Byte = 8-bit data to be transmitted
 * Return = 8-bit data received from the external Flash
 *
 */


uint8_t TransferByte(uint8_t Byte)
{
    InterruptWait = 1; // This placeholder will be used for waiting till transmission is complete.

    SSIIntEnable(SSI0_BASE, SSI_TXFF);  //  Enable End-Of-Transmission Flag, this will initiate the transfer.
    SSIDataPutNonBlocking(SSI0_BASE, (uint32_t) Byte); //Place the data to be transmitted into the Tx FIFO

    while(InterruptWait);  // Wait for the End-of-Transmission Interrupt to be triggered. SSI0IntHandler is called to handle the EOT flag ( i.e. SSI_TXFF)  before you move to the next line.

    /* Returned After SSIOIntHandler, which sets InterruptWait to 0; Exiting the While Loop above */

    return (uint8_t)pui32DataRx;

}

/*
 *  End-of-Transmission Interrupt Handler; Transfers the received 32-bit data in the RXFIFO buffer to pui32DataRx
 *
 */
void SSI0IntHandler(void) {

    /* You're here because a byte has been transferred, i.e. EOT flag was set. */

    InterruptWait = 0; // Acknowledge that Interrupt was triggered by clearing the place holder.
    SSIIntClear(SSI0_BASE, SSI_TXFF); // Clear the EOT flag - to allow for it to be set again.
    SSIDataGetNonBlocking(SSI0_BASE, &pui32DataRx);
    SSIIntDisable(SSI0_BASE, SSI_TXFF); // Disable the EOT Interrupt.

}

