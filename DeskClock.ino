#include<Wire.h>
#include<ds3231.h>
#include<U8glib.h>
#include "avr/sleep.h"
#include "avr/interrupt.h"

#define BUFF_MAX 32
#define DEBUG 1

#ifdef DEBUG
const uint8_t LEDPIN = 13;
#endif


// Arduino pins where the navigation buttons are connected to
const uint8_t SELECT = 5, MENUBACK = 6;				// PORT D
const uint8_t UP = 14, DOWN = 15, LEFT = 16, RIGHT = 17;	// PORT C

const uint8_t CLKIN = 4; 		//This is the clock pulse pin input PD4
/*	btnREGISTER 
	bit 				7		6		5		4		3		2		1		0
	function		StatusFlag	N/A	SELECT	MENU/BACK	RIGHT	LEFT	DOWN	UP	
*/


/*	Initialize display
	U8GLIB_ST7920_128X64_1X(sclk, mosi, cs [, reset])
	SCLK - EN, MOSI - RW, CS - RS 
*/
#ifdef DEBUG
	U8GLIB_ST7920_128X64_1X u8g(12, 11, 10, U8G_PIN_NONE);
#else
	U8GLIB_ST7920_128X64_1X u8g(13, 11, 10, U8G_PIN_NONE);
#endif
struct ts time; // ts is the DS3231 time structure

/* 	Initialize clock
	Set control registers to values desired
	The RS2 and RS1 bits are set to 0,0 for 1 Hz and 
	INTCN is also set to 0 to enable the square wave
	as it is active low.
	The alarms are disabled. Basically we're writing 0 to 0x0E */
const uint8_t DS3231controlReg = 0b00000000;

volatile uint8_t getTime = 0, canSleep = 0;
volatile uint8_t btnREGISTER = 0, processButtons = 0;

// Function pointer to display the current page on the screen
void (*display_screen)();


void setup () 
{
	/*	Initialize interrupts
		PCICR = Pin Change Interrupt Control Register pg. 73/448
		Enable pin change interrupt on PCMSK 1 and 2 as our PCINTs are located 
		in these two mask registers
		Ref. doc.txt for functions
		pg. 74/448
		We need interrupts on the pins PCINT 20 to 23 which reside on PCMSK2
		[20 - RTCSQW] [21 - SELECT] [22 - MENUBACK]
		We need interrupts on the pins PCINT 08 and 11 which reside on PCMSK1
		[08 - UP] [09 - DOWN] [10 - LEFT] [11 - RIGHT]
	*/
	PCICR = PCICR | (1 << PCIE2) | (1 << PCIE1); 	
	PCMSK2 = PCMSK2 | 0b01110000;
	PCMSK1 = PCMSK1 | 0b00001111;
	
	/* Timer 2 to debounce button inputs
	The debounce logis is a bit different. Basically what's happening here is 
	that the Timer 2 is going to run and each time it overflow, the Timer ISR 
	will trigger. This is the ISR that actually tells the program to process 
	the button inputs. The other ISRs merely fill up a buffer that indicate why
	which button was pressed and quickly exit.
	*/	
	// Timer 2 register for Button helper
	TCCR2A = TCCR2A | 0b00100000;		// Mode is Clear OC2B on compare match
	TCCR2B = TCCR2B | 0b00000111;		// Setting the prescaler to 1024
	OCR2B = 155;						// Register value 155
	TIMSK2 = TIMSK2 | 0b100;	

	//Serial.begin(9600);
	pinMode(CLKIN, INPUT_PULLUP); 
	pinMode(UP, INPUT_PULLUP);
	pinMode(DOWN, INPUT_PULLUP);
	pinMode(LEFT, INPUT_PULLUP);
	pinMode(RIGHT, INPUT_PULLUP);
	pinMode(SELECT, INPUT_PULLUP);
	pinMode(MENUBACK, INPUT_PULLUP);
	#ifdef DEBUG
		pinMode(LEDPIN, OUTPUT);
	#endif
	
	display_screen = clock_page;

	Wire.begin();
	// Initializing the DS3231 control registers so that we get a 1Hz Square wave from the INT_/SQW pin
	DS3231_set_creg(DS3231controlReg);
	
}

void loop () 
{	
	if(getTime){
		display_screen();
		canSleep = 1;
		getTime = 0;
	}
	sleepNow();
}

// ISR executed every 1 second by the external Square wave of 1Hz
ISR(PCINT2_vect)
{
	/*	If condition INPUT PULLUP note
		Since every pin is in the INPUT_PULLUP mode we need to check if the pin is LOW
		which means the if statement condition is switched. Since the conditional check
		will return 0 if the pin is low, and non-zero if the pin is high we need to check
		for when the pin is low as that is our changed state from the default	
	*/
	// The clock pin and code related to the clock pin
	if(0 == (PIND & _BV(PIND4)))
	{
		getTime = 1;
		canSleep = 0;
	}	// ELSE statement below note	
	/*	Single Line bit field expression note
	We need to figure out which bit field flag needs to be set. Our input pins are
	consecutive on the microcontroller so we can just dump the port data directly 
	into the bitfield variable since it's ordered exactly the way we want it
	This saves us cycles and flash space by compressing 4 instructions into 1.
	
	Since the input pins are configured in PULLUP mode, when a button is pressed that
	pin will actually read a ZERO/LOW and to capture that as a 1, we need to bit invert
	it which is the ~ symbol. Not to confuse with the conditional-logical NOT which is the 
	! symbol.
	We require pins 5, 6 from PORT D so we can directly negate, bitmask, right shift 
	and add it to our bitfield variable.
	*/
	else if(~PIND & 0b01100000)
	{
			btnREGISTER = btnREGISTER | ((~PIND & 0b01100000)>>1);
	}
	
}

ISR(PCINT1_vect)
{
	/*	If condition INPUT PULLUP note
		Since every pin is in the INPUT_PULLUP mode we need to check if the pin is LOW
		which means the if statement condition is switched. Since the conditional check
		will return 0 if the pin is low, and non-zero if the pin is high we need to check
		for when the pin is low as that is our changed state from the default	
	*/
	/*	Single Line bit field expression note
		We need to figure out which bit field flag needs to be set. Our input pins are
		consecutive on the microcontroller so we can just dump the port data directly 
		into the bitfield variable since it's ordered exactly the way we want it
		This saves us cycles and flash space by compressing 4 instructions into 1.
		
		Since the input pins are configured in PULLUP mode, when a button is pressed that
		pin will actually read a ZERO/LOW and to capture that as a 1, we need to bit invert it
		which is the ~ symbol. Not to confuse with the conditional-logical NOT which is the 
		! symbol.
		We require pins 0, 1, 2, 3 from PORT C so we can directly negate, bitmask, and add
		it to our bitfield variable.
	*/
	btnREGISTER = btnREGISTER | ((~PINC) & 0b00001111);
	/*
	if (0 == (PINC & _BV(PINC0)))
	{
		btnREGISTER = btnREGISTER | 0b00000001;	//Set UP bit
	}
	else if (0 == (PINC & _BV(PINC1)))
	{
		btnREGISTER = btnREGISTER | 0b00000010; //Set DOWN bit
	}
	else if (0 == (PINC & _BV(PINC2)))
	{
		btnREGISTER = btnREGISTER | 0b00000100; //Set LEFT bit
	}
	else if(0 == (PINC & _BV(PINC3)))
	{
		btnREGISTER = btnREGISTER | 0b00001000; //Set RIGHT bit
	}
	*/
}

ISR(TIMER2_COMPB_vect)
{
	if(btnREGISTER)
	{
	    processButtons = 1;
	}
}
