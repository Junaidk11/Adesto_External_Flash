/*
 *      adesto.c
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
        uint8_t manufacturingId, deviceId;                  // Create Place holders for Manufacturing and Device ID
        UARTprintf("Acquiring Flash Info...\n");
        ChipSelect(~GPIO_PIN_3);                            //Assert External Flash Chip-select
        TransferByte(AT_READ_ID);                           // Adesto's Instruction Command to retrieve Device Information
        TransferByte(0x00);                                 // Dummy byte
        TransferByte(0x00);                                 // Dummy byte
        TransferByte(0x00);                                 // Dummy byte
        manufacturingId = TransferByte(0x00);               // Manufacturing ID
        deviceId= TransferByte(0x00);                       // Device ID
        ChipSelect(GPIO_PIN_3);                             // Deassert External Flash Chip-Select
        UARTprintf("Manufacturing ID: \n", manufacturingId);
        UARTprintf("Device ID: \n", deviceId);

}

/*
 *      This function is used to Program a page of the External FLash. Page = 256 Bytes.
 *      First, you need to send Write Enable Command after assertion of Chip-Select. To indicate End of Communication, deassert Chip-Select.
 *      Next, send Page Program Command, followed by 24-bit start Address, followed by the 256 Bytes of data - A byte at a time.
 *      After 256th Byte, Deassert Chip-Select - Indicates the data to be stored in the Flash has been sent.
 *      After Deassertion, the Flash starts Programming its Memory starting at the 24-bit address - you need to wait until the device has finished writing
 *      all 256 bytes to its Memory.
 *
 */

void PageWrite(uint32_t startAddress, uint32_t numberOfBytes, uint8_t *Data)
{
       ChipSelect(~GPIO_PIN_3);                    // Assert External Flash Chip-Select
       TransferByte(AT_WRITE_ENABLE);              // Send Write-Enable Command to the External Flash
       ChipSelect(GPIO_PIN_3);                     // Deassert External Flash  Chip-Select
       ChipSelect(~GPIO_PIN_3);                    // Assert External Flash Chip-Select Program Page
       TransferByte(AT_PAGE_PROGRAM);              // Send Page Program Command to Flash
       TransferByte((startAddress >> 16) & 0xFF);  // Extracting Address Byte A23:A16 and Sending to Flash
       TransferByte((startAddress >>  8) & 0xFF);  // Extracting Address Byte A15:A08 and Sending to Flash
       TransferByte((startAddress >>  0) & 0xFF);  // Extracting Address Byte A07:A00 and Sending to Flash

       // Now transfer Data
       int i;
       for (i = 0; i < numberOfBytes; i++) {
           TransferByte(Data[i]);
       }
       ChipSelect(GPIO_PIN_3);                    // Deassert External Flash Chip-Select to signal end of communication.

       DeviceBusyDelay();                         // Wait till Flash Programs a page; Flash programs its memory after Chip-Select Deasserted.
}

/*
 *  This function is used to read device status; The 8-bit data returned by Adesto is used to determine device status.
 *   Returned Byte value = 1     Device Busy
 *   Returned Byte value = 0     Device Ready
 */
void DeviceBusyDelay(void)
{
        ChipSelect(~GPIO_PIN_3);                    //Assert Flash Chip-Select
        TransferByte(AT_READ_STATUS_FORMAT_1);      //Request Device Status
        while (TransferByte(0x00) & 1);             //Wait till Device is Ready
        ChipSelect(GPIO_PIN_3);                    //Deassert Flash Chip-Select
}


/*
 *  This function is used to Write Data to the External Flash given the StartingAddress, number of Bytes to write and Pointer
 *  to the data to be written to the External Flash.
 *  The function checks if the starting address aligns with the a 256 Byte page Address and based on the alignment result,
 *  sends bytes of Data to Flash's internal buffer via the PageWrite Method.
 */

void WriteToFlash(uint32_t startAddress, uint32_t numberOfBytes, uint8_t *Data)
{
        uint8_t numberOfFullPages = 0, numberOfSingleBytes = 0, pageNotAligned = 0,  leftOverBytes = 0;
        pageNotAligned = startAddress % AT_PAGE_SIZE_256;   // Checking if start Address aligns with 256 Byte Page Boundary (i.e. checking if A07:A00 = 0x00)
        // number of Full pages
        numberOfFullPages = numberOfBytes / AT_PAGE_SIZE_256;
        // number of Single bytes remaining
        numberOfSingleBytes = numberOfBytes % AT_PAGE_SIZE_256;


        if (pageNotAligned == 0) // Start Address is Aligned with Page Boundaries ; 0-FF => 100h, 256 Bytes = 1 Page = 100h
        {
            while (numberOfFullPages--)
            {
                PageWrite(startAddress, AT_PAGE_SIZE_256, Data);
                startAddress += AT_PAGE_SIZE_256;
                Data += AT_PAGE_SIZE_256;
            }
            if (numberOfSingleBytes > 0) // If total pages not a whole number, the remaining bytes are written on the next page.
            {
                PageWrite(startAddress, numberOfSingleBytes, Data);
            }
        }
        else     // Start Address is not Aligned with Page Boundaries
        {
            leftOverBytes = AT_PAGE_SIZE_256 - pageNotAligned;    // The number of bytes remaining to form a page of 256 Bytes.
            if(numberOfBytes<leftOverBytes)
            {
                PageWrite(startAddress, numberOfBytes, Data);

            }
            else if (numberOfBytes>leftOverBytes)
            {
                PageWrite(startAddress,leftOverBytes,Data);           // Page completely programmed.
                startAddress+=leftOverBytes;                          // Moving address to the start of Next Page.
                Data+=leftOverBytes;                                  // Increment Data Index

                // Now we can write pages to the External Flash - First calculate number of bytes remaining
                numberOfBytes-=leftOverBytes;
                numberOfFullPages = numberOfBytes % AT_PAGE_SIZE_256;
                numberOfSingleBytes = numberOfBytes % AT_PAGE_SIZE_256;
                while (numberOfFullPages--)
                {
                    PageWrite(startAddress, AT_PAGE_SIZE_256, Data);
                    startAddress += AT_PAGE_SIZE_256;
                    Data += AT_PAGE_SIZE_256;
                }
               if (numberOfSingleBytes > 0) // If total pages not a whole number, the remaining bytes are written on the next page.
                {
                   PageWrite(startAddress, numberOfSingleBytes, Data);
                }
            }
        }
}

void ReadFlash(uint32_t startAddress, uint32_t numberOfBytes, uint8_t *DataRx)
{
        ChipSelect(~GPIO_PIN_3);                    // Assert External Flash Chip select
        TransferByte(AT_READ_DATA);                 // Send Adesto Command for requesting Data from Flash.
        TransferByte((startAddress >> 16) & 0xFF);  // Extract A23-A16 from the 32-bit Start Address and send to Flash
        TransferByte((startAddress >>  8) & 0xFF);  // Extract A15-A08 from the 32-bit Start Address and send to Flash
        TransferByte((startAddress >>  0) & 0xFF);  // Extract A07-A00 from the 32-bit Start Address and send to Flash

        int i;
        for (i = 0; i < numberOfBytes; i++)
        {
            DataRx[i] = TransferByte(0x00);         // Send dummy data to retrieve Data from the sent startAddress.
        }

        ChipSelect(GPIO_PIN_3);                     // Deassert External Flash Chip select
        DeviceBusyDelay();                           // Wait till the Device has sent all the Bytes

}


