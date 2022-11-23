/***********************************************************************
 * 
 * Use Analog-to-digital conversion to read push buttons on LCD keypad
 * shield and display it on LCD screen.
 * 
 * ATmega328P (Arduino Uno), 16 MHz, PlatformIO
 *
 * Copyright (c) 2018 Tomas Fryza
 * Dept. of Radio Electronics, Brno University of Technology, Czechia
 * This work is licensed under the terms of the MIT license.
 * 
 **********************************************************************/


/* Includes ----------------------------------------------------------*/
#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include <gpio.h>           // GPIO library for AVR-GCC
#include "timer.h"          // Timer library for AVR-GCC
#include <lcd.h>            // Peter Fleury's LCD library
#include <stdlib.h>         // C library. Needed for number conversions

//defines

#define LED_RED PB0

#define PB0 10


/* Function definitions ----------------------------------------------*/
/**********************************************************************
 * Function: Main function where the program execution begins
 * Purpose:  Use Timer/Counter1 and start ADC conversion every 100 ms.
 *           When AD conversion ends, send converted value to LCD screen.
 * Returns:  none
 **********************************************************************/
int main(void)
{
   
    
}


/* Interrupt service routines ----------------------------------------*/
/**********************************************************************
 * Function: Timer/Counter1 overflow interrupt
 * Purpose:  Use single conversion mode and start conversion every 100 ms.
 **********************************************************************/
ISR(TIMER1_OVF_vect)
{
    // Start ADC conversion
    ADCSRA = ADCSRA | (1<<ADSC);
}

/**********************************************************************
 * Function: ADC complete interrupt
 * Purpose:  Display converted value on LCD screen.
 **********************************************************************/
ISR(ADC_vect)
{
    static uint8_t no_of_overflows = 0;
    uint16_t value;

    no_of_overflows++;
    if (no_of_overflows >= 3)
    {
      no_of_overflows=0;
      // Read converted value
      // Note that, register pair ADCH and ADCL can be read as a 16-bit value ADC
      value = ADC;
      // Convert "value" to "string" and display it
      uint8_t led_value = 0;  // Local variable to keep LED status

    // Set pin as output
    pinMode(LED_RED, 0x1);

       while (1) {
        // Pause several milliseconds
        _delay_ms(250);

        // Change LED value
        if (value > 511) {
            led_value = 1;
            // Set pin(s) to HIGH
            digitalWrite(LED_RED, 1);

        }
        else {
            led_value = 0;
            // Clear pin(s) to LOW
            digitalWrite(LED_RED, 0);
       }
       }

    }
}
