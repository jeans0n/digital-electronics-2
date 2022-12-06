/***********************************************************************
 *
 * Joystick-meter project
 *
 * ATmega328P (Arduino Uno), 16 MHz, PlatformIO, Analog Joystick,
 * Rotary encoder, LCD screen
 *
 * Copyright (c) 2022 Noé Fortune, Paul Cascarino
 * Dept. of Radio Electronics, Brno University of Technology, Czechia
 * This work is licensed under the terms of the MIT license.
 *
 **********************************************************************/

/* Defines -----------------------------------------------------------*/

#define false 0
#define true 1


#define ROTARY_ENCODER_OUT_CLK      PD2
#define ROTARY_ENCODER_OUT_SW       PD3
#define ROTARY_ENCODER_OUT_DT       PB2


#define ADC0    0
#define ADC1    1 

/* Includes ----------------------------------------------------------*/
#include <avr/io.h>        // AVR device-specific IO definitions
#include <avr/interrupt.h> // Interrupts standard C library for AVR-GCC
#include "timer.h"         // Timer library for AVR-GCC
#include <lcd.h>            // Peter Fleury's LCD library
#include <gpio.h>          // GPIO library for AVR-GCC
#include <stdlib.h>        // C library. Needed for number conversions

/* Global variables --------------------------------------------------*/

uint8_t current_adc_channel;
uint16_t val_adc[2];
volatile uint8_t adc_measure_available[2];
uint8_t rotary_counter;
uint8_t rotary_mode;
uint8_t symbol;
volatile int8_t overflow_max;

/* Constants */

const uint8_t encoder_symbol_lookup_table[] = {0xFF, '<', '>', '|'};

/* Function definitions ----------------------------------------------*/

/**********************************************************************
 * Function: init_gpio
 * Purpose:  Configure GPIOs related to the rotary encoder.
 * Returns:  none
 **********************************************************************/
void init_gpio()
{
    GPIO_mode_input_pullup(&DDRD, ROTARY_ENCODER_OUT_CLK);
    GPIO_mode_input_pullup(&DDRD, ROTARY_ENCODER_OUT_SW);
    GPIO_mode_input_pullup(&DDRB, ROTARY_ENCODER_OUT_DT);
    EIMSK &= ~((1 << INT0) | (1 << INT1)); //External Interrupt Mask Register - EIMSK - is for enabling INT[6;3:0] interrupts, INT0 and INT1 are disabled to avoid false interrupts when mainuplating EICRA
    EICRA |= (1 << ISC01)|(1 << ISC00) | (1 << ISC11); //External Interrupt Control Register A - EICRA - defines the interrupt edge profile, here configured to trigger on rising edge INT0 and falling edge INT1
    EIFR &= ~((1 << INTF0) | (1 << INTF1)); //External Interrupt Flag Register - EIFR controls interrupt flags on INT[6;3:0], here it is cleared
    EIMSK |= (1 << INT0) | (1 << INT1); //Enable INT0 and INT1
}

/**********************************************************************
 * Function: init_adc
 * Purpose:  Configure Analog-to-Digital Conversion unit.
 *           Sets the voltage reference, the clock and enables 
 *           the conversion complete interrupt.
 * Returns:  none
 **********************************************************************/
void init_adc()
{
    // Select ADC voltage reference to "AVcc with external capacitor at AREF pin"
    ADMUX = ADMUX | (1 << REFS0);
    // Enable ADC module
    ADCSRA = ADCSRA | (1 << ADEN);
    // Enable conversion complete interrupt
    ADCSRA = ADCSRA | (1 << ADIE);
    // Set clock prescaler to 128
    ADCSRA = ADCSRA | (1 << ADPS2 | 1 << ADPS1 | 1 << ADPS0);
}

/**********************************************************************
 * Function: select_adc_channel
 * Purpose:  Select the ADC channel.
 * Returns:  none
 **********************************************************************/
void select_adc_channel(uint8_t channel)
{
    ADMUX &= 0xF0;
    ADMUX |= (channel & 0x0F);
}

/**********************************************************************
 * Function: start_adc_measures
 * Purpose:  Start the measurement by enabling the timer interrupt.
 * Returns:  none
 **********************************************************************/
void start_adc_measures() {

    // Configure 16-bit Timer/Counter1 to start ADC conversion
    // Set prescaler to 33 ms and enable overflow interrupt
    TIM1_overflow_33ms();
    TIM1_overflow_interrupt_enable();

    // Enables interrupts by setting the global interrupt mask
    sei();
}

/**********************************************************************
 * Function: get_mask_from_adc
 * Purpose:  Gets the bit mask from the adc value.
 *           The adc range is divided into 14 intervals.
 *           For each adc value interval a 14-bit mask is defined.
 * Input(s): adc_value - the adc value (range 0-1023)
 * Returns:  the mask to use to display the symbol bar.
 **********************************************************************/
uint16_t get_mask_from_adc(uint16_t adc_value) {
    if(adc_value < 69 ) { return 0x00FF; }
    if(adc_value < 137) { return 0x00FE; }
    if(adc_value < 205) { return 0x00FC; }
    if(adc_value < 274) { return 0x00F8; }
    if(adc_value < 342) { return 0x00F0; }
    if(adc_value < 410) { return 0x00E0; }
    if(adc_value < 478) { return 0x00C0; }
    if(adc_value < 547) { return 0x0080; }
    if(adc_value < 615) { return 0x0180; }
    if(adc_value < 683) { return 0x0380; }
    if(adc_value < 751) { return 0x0780; }
    if(adc_value < 820) { return 0x0F80; }
    if(adc_value < 888) { return 0x1F80; }
    if(adc_value < 956) { return 0x3F80; }
    return 0x7F80;
}

/**********************************************************************
 * Function: display_bars
 * Purpose:  Displays the provided symbol in the targeted line as 
 *           indicated by the mask.
 *           The symbol is displayed at the bit position when the bit 
 *           is set, a white space is displayed if not.
 * Input(s): line   - 0 for the first line, 1 for the second line.
 *           symbol - the ascii code of the character to be used.
 *           mask   - the 14-bit mask indicating which position must filled 
 *           with the symbol.
 * Returns:  none.
 **********************************************************************/
void display_bars(uint8_t line, uint8_t symbol, uint16_t mask)
{
    for (uint8_t i = 0; i < 15; i++)
    {
        if (mask & (1 << i))
        {
            lcd_gotoxy(i, line);
            lcd_putc(symbol);
        }
        else
        {
            lcd_gotoxy(i, line);
            lcd_putc(' ');
        }
    }
}


/**********************************************************************
 * Function: Main function where the program execution begins
 * Purpose:  
 * Returns:  none
 **********************************************************************/
int main(void)
{
    uint16_t mask;

    overflow_max = 3;
    rotary_mode = 0;
    rotary_counter = 0;
    symbol = encoder_symbol_lookup_table[rotary_counter];

    // Initialize display
    lcd_init(LCD_DISP_ON);

    init_gpio();

    adc_measure_available[0] = false;
    adc_measure_available[1] = false;

    init_adc();

    current_adc_channel = ADC0;

    select_adc_channel(current_adc_channel);

    start_adc_measures();

    // Infinite loop
    while (1)
    {
        // the bar display is updated with each new acquisition
        // only when both ADC values have been read
        if (adc_measure_available[ADC0] && adc_measure_available[ADC1])
        {
            mask = get_mask_from_adc(val_adc[ADC0]);
            display_bars(0, symbol, mask);
            mask = get_mask_from_adc(val_adc[ADC1]);
            display_bars(1, symbol, mask);
            adc_measure_available[ADC1] = false;
            adc_measure_available[ADC0] = false;
        }
    }

    // Will never reach this
    return 0;
}

/* Interrupt service routines ----------------------------------------*/
/**********************************************************************
 * Function: Timer/Counter1 overflow interrupt
 * Purpose:  Toggle current ADC channel and start a new acquisition.
 **********************************************************************/
ISR(TIMER1_OVF_vect)
{
    // toggle adc channel
    current_adc_channel = current_adc_channel == ADC0 ? ADC1 : ADC0;
    select_adc_channel(current_adc_channel);

    // Start ADC conversion
    ADCSRA = ADCSRA | (1<<ADSC);
}

/**********************************************************************
 * Function: Conversion complete interrupt
 * Purpose:  Read the ADC result and set a flag to indicate it is
 *           available.
 *           An overflow mechanism allow to adjust the sampling rate.
 **********************************************************************/
ISR(ADC_vect)
{
    // We keep the ISR execution time as short as possible
    static uint8_t no_of_overflows = 0;
    if(current_adc_channel == ADC0) {
        no_of_overflows++;
    }
    if (no_of_overflows < overflow_max)
    {
        return;
    }
    if(current_adc_channel == ADC1) {
        no_of_overflows = 0;
    }
    val_adc[current_adc_channel] = ADC;
    adc_measure_available[current_adc_channel] = true;
}


/**********************************************************************
 * Function: Rotary Encoder Clock interrupt (Rising Edge) 
 * Purpose:  Depending on the current mode, update the symbol displayed
 *           or the sampling rate
 **********************************************************************/
ISR (INT0_vect) 
{
    int8_t v;
    v = GPIO_read(&PINB, ROTARY_ENCODER_OUT_DT) ? 1 : -1;
    if(rotary_mode == 0)
    {
        rotary_counter += v;
        symbol = encoder_symbol_lookup_table[rotary_counter%4];
    }
    else
    {
        overflow_max += v;
        if(overflow_max < 0)
        {
            overflow_max = 0;
        }
    }
}

/**********************************************************************
 * Function: Rotary Encoder Switch interrupt (Falling edge) 
 * Purpose:  Toggle rotary mode.
 **********************************************************************/
ISR (INT1_vect) 
{
    rotary_mode = rotary_mode ? 0 : 1;
}

