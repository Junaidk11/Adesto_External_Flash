/*
 * main.c
 *
 *      Created on: Jul 28, 2019
 *      Author: junaidkhan
 */

#include <stdint.h>
#include <stdbool.h>
#include "driverlib/ssi.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"


/*
 *  PE4        ->           Flash_CS
 *  SSIRx      ->           SSI0Rx      ->   PA4
 *  SSITx      ->           SSI0Tx      ->   PA5
 *  SSIClk     ->           SSI0Clk     ->   PA2
 *
 */


#define SSI_DATA    16  // The number of bytes to send and receive

int main()
{


    uint32_t pui32DataTx[SSI_DATA]; // Transmission Commands for the External Flash
    uint32_t pui32DataRx[SSI_DATA]; // The dummy data recieved from the External Flash
    uint32_t ui32index=0;

    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                       SYSCTL_XTAL_16MHZ);  // Change later according to the needs of the external RAM


    // enable clock access to GPIO Port A and SSI0 Peripheral
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);

    // Configure the GPIO Port A for SPI0 functionality.
    GPIOPinConfigure(GPIO_PA2_SSI0CLK);
    GPIOPinConfigure(GPIO_PA5_SSI0TX);
    GPIOPinConfigure(GPIO_PA4_SSI0RX);
    GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_4|GPIO_PIN_5 | GPIO_PIN_2);

    SSIConfigSetExpClk(SSI0_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, 1000000, SSI_DATA); // 1MHZ SSI Frequency, 16 bit Data Sent and Received

    SSIEnable(SSI0_BASE);

    //
    // Read any residual data from the SSI port.  This makes sure the receive
    // FIFOs are empty, so we don't read any unwanted junk.  This is done here
    // because the SSI mode is full-duplex, which allows you to send and
    // receive at the same time.
    // The SSIDataGetNonBlocking function returns
    // "true" when data was returned, and "false" when no data was returned.
    // The "non-blocking" function checks if there is any data in the receive
    // FIFO and does not "hang" if there isn't.
    //
    while(SSIDataGetNonBlocking(SSI0_BASE, &pui32DataRx[0]))
    {
    }

    /*
     * Set Write Enable Latch bit in Status Register of External RAM - Required before:
     * Erase Chip, Program Chip
     */
    pui32DataTx[0] = 0x6;     // Command for Write Enable
    pui32DataTx[1] = 0x60;    // Command for Chip Erase
    pui32DataTx[2] = 0x2;     // Program a byte/Page Command

    /*
     * Transmission bit max = 16 bits, External Flash requires 3 byte Address = 24bits
     * Send 16 bits, followed by masking the first 16 bits, extract next 8 bits to send, before moving to the next index
     * Following 24 bit starting addresss, the next 8 bits = data to be programmed into the external Flash
     *
     */

    pui32DataTx[3] = 0x000000AA; // Starting address = 0x000000h, 0xAAh is the data to be sent - dummy data for now
    pui32DataTx[4] = 0xFC989DFA;
    /*
     * Following the Starting Address clocked, if you're programming a Byte to the External Flash Memory you send 8 bytes
     *
     */

    while(1)
    {

        for(ui32index=0 ; ui32index < SSI_DATA ; ui32index++)
        {
            SSIDataPut(SSI0_BASE, pui32DataTx[ui32index]);
        }

        while(SSIBusy(SSI0_BASE))
        {
        }

        for(ui32index=0 ; ui32index < SSI_DATA ; ui32index++)
        {
           SSIDataGet(SSI0_BASE, &pui32DataRx[ui32index]);
        }
    }



}



