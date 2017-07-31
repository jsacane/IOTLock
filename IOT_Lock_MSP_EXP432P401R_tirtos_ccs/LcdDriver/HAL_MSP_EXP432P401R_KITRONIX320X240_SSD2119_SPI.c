/* --COPYRIGHT--,BSD
 * Copyright (c) 2016, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/
//*****************************************************************************
//*****************************************************************************
//
//! \addtogroup display_api
//! @{
//
//*****************************************************************************

#include <ti/grlib/grlib.h>
#include "HAL_MSP_EXP432P401R_KITRONIX320X240_SSD2119_SPI.h"
#include "kitronix320x240x16_ssd2119_spi.h"
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

const uint32_t TEMP_GPIO_PORT_TO_BASE[] =
{
    0x00000000,
    0x40004C00,
    0x40004C01,
    0x40004C20,
    0x40004C21,
    0x40004C40,
    0x40004C41,
    0x40004C60,
    0x40004C61,
    0x40004C80,
    0x40004C81,
    0x40004D20
};

//*****************************************************************************
//
//! Initializes the display driver.
//!
//! This function initializes the KITRONIX320X240 display. This function
//! configures the GPIO pins used to control the LCD display when the basic
//! GPIO interface is in use. On exit, the LCD has been reset and is ready to
//! receive command and data writes.
//!
//! \return None.
//
//*****************************************************************************
void HAL_LCD_initLCD(void)
{
    //
    // Configure the pins that connect to the LCD as GPIO outputs.
    //
    MAP_GPIO_setAsOutputPin(LCD_RESET_PORT,
                            LCD_RESET_PIN);

    MAP_GPIO_setOutputLowOnPin(LCD_RESET_PORT,
                               LCD_RESET_PIN);

    MAP_GPIO_setAsOutputPin(LCD_SDC_PORT,
                            LCD_SDC_PIN);

    MAP_GPIO_setOutputLowOnPin(LCD_SDC_PORT,
                               LCD_SDC_PIN);

    MAP_GPIO_setAsOutputPin(LCD_SCS_PORT,
                            LCD_SCS_PIN);

    MAP_GPIO_setOutputLowOnPin(LCD_SCS_PORT,
                               LCD_SCS_PIN);

    //
    // Configure SPI peripheral.
    //
    // Configure LCD_SIMO_PIN
    MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(LCD_SDI_PORT,
                                                    LCD_SDI_PIN,
                                                    LCD_SDI_PIN_FUNCTION);

    // Configure LCD_CLK_PIN  option select CLK
    MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(LCD_SCL_PORT, LCD_SCL_PIN,
                                                    LCD_SCL_PIN_FUNCTION);

    eUSCI_SPI_MasterConfig spiMasterConfig =
    {
        EUSCI_B_SPI_CLOCKSOURCE_SMCLK,                      // SMCLK Clock Source
        MAP_CS_getSMCLK(),                                  // Get SMCLK frequency
        16000000,                                                   // SPICLK = 16 MHz
        EUSCI_B_SPI_MSB_FIRST,                              // MSB First
        EUSCI_B_SPI_PHASE_DATA_CAPTURED_ONFIRST_CHANGED_ON_NEXT,                 //Phase
        EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_LOW,           // Low polarity
        EUSCI_B_SPI_3PIN                                    // 3Wire SPI Mode
    };

    MAP_SPI_initMaster(LCD_EUSCI_MODULE, &spiMasterConfig);

    MAP_SPI_clearInterruptFlag(LCD_EUSCI_MODULE,
                               EUSCI_B_SPI_RECEIVE_INTERRUPT);

    MAP_SPI_enableModule(LCD_EUSCI_MODULE);

    //
    // Set the LCD Backlight high to enable
    //
    MAP_GPIO_setAsOutputPin(LCD_PWM_PORT,
                            LCD_PWM_PIN);

    MAP_GPIO_setOutputHighOnPin(LCD_PWM_PORT,
                                LCD_PWM_PIN);

    //
    // Set the LCD control pins to their default values.
    //

    MAP_GPIO_setOutputHighOnPin(LCD_SDC_PORT,
                                LCD_SDC_PIN);

    MAP_GPIO_setOutputLowOnPin(LCD_SCS_PORT,
                               LCD_SCS_PIN);

    //
    // Delay for 1ms.
    //
    HAL_LCD_delay(1);

    //
    // Deassert the LCD reset signal.
    //

    MAP_GPIO_setOutputHighOnPin(LCD_RESET_PORT,
                                LCD_RESET_PIN);

    //
    // Delay for 1ms while the LCD comes out of reset.
    //
    HAL_LCD_delay(1);
}

//*****************************************************************************
//
// Writes a command to the SSD2119.  This function implements the basic SPI
// interface to the LCD display.
//
//*****************************************************************************
void HAL_LCD_writeCommand(uint8_t command)
{
    //
    // Wait for any SPI transmission to complete before setting the LCD_SDC signal.
    //
    while(UCB0STATW & UCBUSY)
    {
        ;
    }

    //
    // Set the LCD_SDC signal low, indicating that following writes are commands.
    //
    HWREG16(TEMP_GPIO_PORT_TO_BASE[LCD_SDC_PORT] + OFS_PAOUT) &= ~LCD_SDC_PIN;

    //
    // Transmit the command.
    //
    UCB0TXBUF = command;

    //
    // Wait for the SPI transmission to complete before setting the LCD_SDC signal.
    //
    while(UCB0STATW & UCBUSY)
    {
        ;
    }

    //
    // Set the LCD_SDC signal high, indicating that following writes are data.
    //
    HWREG16(TEMP_GPIO_PORT_TO_BASE[LCD_SDC_PORT] + OFS_PAOUT) |= LCD_SDC_PIN;
}

//*****************************************************************************
//
// Writes a data word to the SSD2119.  This function implements the basic SPI
// interface to the LCD display.
//
//*****************************************************************************
void HAL_LCD_writeData(uint16_t data)
{
    uint8_t ui8Data;

    //
    // Calculate the high byte to transmit.
    //
    ui8Data = (uint8_t)(data >> 8);

    //
    // Wait for the transmit buffer to become empty.
    //
//	while(!SPI_getInterruptStatus(LCD_EUSCI_MODULE,
    while(UCB0STATW & UCBUSY)
    {
        ;
    }

    //
    // Transmit the high byte.
    //
    UCB0TXBUF = ui8Data;

    //
    // Calculate the low byte to transmit.
    //
    ui8Data = (uint8_t)(data & 0xff);

    //
    // Wait for the transmit buffer to become empty.
    //
    while(UCB0STATW & UCBUSY)
    {
        ;
    }

    //
    // Transmit the high byte.
    //
//	SPI_transmitData(LCD_EUSCI_MODULE,ui8Data);
    UCB0TXBUF = ui8Data;
}

//*****************************************************************************
//
// Clears CS line
//
// This macro allows to clear the Chip Select (CS) line
//
// \return None
//
//*****************************************************************************

void HAL_LCD_selectLCD(){
    //
    // Wait for any SPI transmission to complete before setting the LCD_SCS signal.
    //
    while(SPI_isBusy(LCD_EUSCI_MODULE))
    {
        ;
    }

    HWREG16(TEMP_GPIO_PORT_TO_BASE[LCD_SCS_PORT] + OFS_PAOUT) &= ~LCD_SCS_PIN;
}

//*****************************************************************************
//
// Set CS line
//
// This macro allows to set the Chip Select (CS) line
//
// \return None
//
//*****************************************************************************

void HAL_LCD_deselectLCD(){
    //
    // Wait for any SPI transmission to complete before setting the LCD_SCS signal.
    //
    while(SPI_isBusy(LCD_EUSCI_MODULE))
    {
        ;
    }

    HWREG16(TEMP_GPIO_PORT_TO_BASE[LCD_SCS_PORT] + OFS_PAOUT) |= LCD_SCS_PIN;
}

//*****************************************************************************
//
// Generates delay of
//
// \param cycles number of cycles to delay
//
// \return None
//
//*****************************************************************************
void HAL_LCD_delay(uint16_t msec)
{
    uint32_t i = 0;
    uint32_t time = (msec / 1000) * (SYSTEM_CLOCK_SPEED / 15);

    for(i = 0; i < time; i++)
    {
        ;
    }
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
