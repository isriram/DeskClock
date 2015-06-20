Pin out for ST7920 128x64
1 - Vss - GND
2 - Vcc - 5V
3 - Vee - Contrast - No use in PWMing - connect to ground for max. contrast
4 - RS -  Register Select
5 - RW - Read/Write - Connects to ground since we are only writing to the display
6 - E - Enable
7...14 - D0..D7 - Data pins if using parallel mode
15 - PSB - Parallel or Serial bit - 0 for serial, 1 for parallel. We ground the pin.
16...18 - Ignore
19 - LED+ - Spec. 4.2V, 5V tolerant(need to check) - PWM
20 - LED- - 


/*
 The six Arduino "analog" pins may be used as common GPIO pins and are numbered from 14 to 19 (14 is analog pin 1, etc).
 It is thus legal to issue a pinMode(14, OUTPUT) and then a digitalWrite(14, 0). 
*/

PWM pins on arduino - 3, 5, 6, 9, 10, 11
12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2,   ,   ,RX0,TX1

13,3V3,REF,A0,A1,A2,A3,A4,A5,A6,A7