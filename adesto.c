/*
 * adesto.c
 *
 *      Created on: Aug 19, 2019
 *      Author: junaidkhan
 */

#include "adesto.h"

/*
 * UART Pin Configuration
 *
 *  PA0 -> UART0_RX
 *  PA1 <- UART0_TX
 */


/*
 * Function used for Initializing UART Module 0 for printing Serial Data - for testing the SPI communication
 */
void UART_Init(void) {
    // Enable clock access to the GPIO Peripheral used by the UART - Use GPIO Port A, as we're already using it for SPI com
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    // Enable clock access to UART0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    // Configure GPIO Pins for UART mode.
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // Use the internal 16MHz oscillator as the UART clock source.
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    // Initialize the UART for console I/O.
    UARTStdioConfig(0, 9600, 16000000);
}

/*
 * This function selects the Adesto External Flash for communication.
 *
 * @value is used to assert and deassert the CS pin of the external flash.
 *
 */

void ChipSelect(uint32_t value)
{
    // Set the CS of External Flash; Pin normally HIGH, i.e. PA3 = 1 when not in use.
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_3, value);
}

/*
 *  This function is used to read the External Flash's Manufacturing and Device ID.
 *  It is used to ensure for Device identity to ensure communication with the right device.
 *
 *  First Byte Sent = Adesto SPI Command for Read ID - Check Datasheet of Adesto.
 *  Next 3 bytes sent are Dummy Bytes, Slave data output is garbage for all 3 dummy bytes
 *  Next 2 bytes sent are Dummy bytes but bytes received are Manufacturing ID of 1Fh, followed by Device ID of 14h.
 *
 */

void ReadId(void)
{

        uint8_t manufacturingId, deviceId; // Create Place holders for Manufacturing and Device ID

        UARTprintf("Acquiring Flash Info...\n");

        ChipSelect(~GPIO_PIN_3);    //Assert External Flash Chip-select

        TransferByte(AT_READ_ID);      // Adesto's Instruction Command to retrieve Device Information
        TransferByte(0x00);            // Dummy byte
        TransferByte(0x00);            // Dummy byte
        TransferByte(0x00);            // Dummy byte
        manufacturingId = TransferByte(0x00);   // Manufacturing ID
        deviceId= TransferByte(0x00); // Device ID

        ChipSelect(GPIO_PIN_3);     // Deassert External Flash Chip-Select

        UARTprintf("Manufacturing ID: \%2x\n", manufacturingId);
        UARTprintf("Device ID: \%2x\n", deviceId);

}

