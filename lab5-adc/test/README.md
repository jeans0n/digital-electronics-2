# Lab 5: NOÉ FORTUNE

### Analog-to-Digital Conversion

1. Complete table with voltage divider, calculated, and measured ADC values for all five push buttons.

   | **Push button** | **PC0[A0] voltage** | **ADC value (calculated)** | **ADC value (measured)** | **ADC value (measured, hex)** |
   | :-: | :-: | :-: | :-: | :-: |
   | Right  | 0&nbsp;V | 0   | 0 | 0 |
   | Up     | 0.495&nbsp;V | 101 | 100 | 64 |
   | Down   | 1.203&nbsp;V | 246 | 256 | 100 |
   | Left   | 1.97&nbsp;V | 403 | 410 | 19A |
   | Select | 3.18&nbsp;V | 650 | 640 | 280 |
   | none   | 5&nbsp;V | 1023 | 1023 | 3ff |

### Temperature meter

Consider an application for temperature measurement. Use analog temperature sensor [TC1046](http://ww1.microchip.com/downloads/en/DeviceDoc/21496C.pdf), LCD, and a LED. Every 30 seconds, the temperature is measured and the value is displayed on LCD screen. When the temperature is too high, the LED will turn on.

2. Draw a schematic of temperature meter. The image can be drawn on a computer or by hand. Always name all components and their values.

   ![Temperature meter schematic](https://raw.githubusercontent.com/jeans0n/digital-electronics-2/main/lab5-adc/test/temperature_meter.png)

3. Draw two flowcharts for interrupt handler `TIMER1_OVF_vect` (which overflows every 1&nbsp;sec) and `ADC_vect`. The image can be drawn on a computer or by hand. Use clear descriptions of the individual steps of the algorithms.

   ![](https://raw.githubusercontent.com/jeans0n/digital-electronics-2/main/lab5-adc/test/adc_flowchart_1.png)
   ![](https://raw.githubusercontent.com/jeans0n/digital-electronics-2/main/lab5-adc/test/adc_flowchart_2.png)
   ![](https://raw.githubusercontent.com/jeans0n/digital-electronics-2/main/lab5-adc/test/adc_flowchart_3.png)
