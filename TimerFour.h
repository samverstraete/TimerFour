/*
 *  Interrupt and PWM utilities for 16 bit Timer4 on ATmega2560
 *  Original code by Jesse Tane for http://labs.ideo.com August 2008
 *  Modified March 2009 by Jérôme Despatis and Jesse Tane for ATmega328 support
 *  Modified June 2009 by Michael Polli and Jesse Tane to fix a bug in setPeriod() which caused the timer to stop
 *  Modified April 2012 by Paul Stoffregen - portable to other AVR chips, use inline functions
 *  Modified again, June 2014 by Paul Stoffregen - support Teensy 3.1 & even more AVR chips
 *  Modified March 2020 by Sam Verstraete, porting to Leonarde, Pro Micro or other ATMega32u4/ATMega16u4
 *
 *  This is free software. You can redistribute it and/or modify it under
 *  the terms of Creative Commons Attribution 3.0 United States License. 
 *  To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/us/ 
 *  or send a letter to Creative Commons, 171 Second Street, Suite 300, San Francisco, California, 94105, USA.
 *
 */

#ifndef TimerFour_h_
#define TimerFour_h_

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "config/timer_four_pins.h"
#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
#define TIMER4_RESOLUTION 1024UL  // Timer4 is 10 bit
#define F_PLL 96000000
#endif

// Placing nearly all the code in this .h file allows the functions to be
// inlined by the compiler.  In the very common case with constant values
// the compiler will perform all calculations and simply write constants
// to the hardware registers (for example, setPeriod).


class TimerFour
{

#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
  public:
    //****************************
    //  Configuration
    //****************************
    void initialize(unsigned long microseconds=1000000) __attribute__((always_inline)) {
		TCCR4D = ~_BV(WGM41) & _BV(WGM40);	// set mode as phase and frequency correct pwm, stop the timer
		TCCR4A = 0;							// clear control register A
		TCCR4C = 0;							// clear control register C
		TCCR4E = 0;							// clear control register E
		setPeriod(microseconds);
    }
    void setPeriod(unsigned long microseconds) __attribute__((always_inline)) {
		unsigned long cycles = (F_CPU / 2000000) * microseconds; // Use CPU
		//
		// |  Bit   |      7       |     6     |   5    |   4    |   3    |   2   |   1   |   0   |
		// |--------|--------------|-----------|--------|--------|--------|-------|-------|-------|
		// | PLLFRQ | PINMUX       | PLLUSB    | PLLTM1 | PLLTM0 | PDIV3  | PDIV2 | PDIV1 | PDIV0 |
		// | Value  | 0            | 1         | x      | x      | 1      | 0     | 1     | 0     |
		// | Expl.  | Connect to   | PLL/2 for | (1)             | PLL internal VCO clock ref.    |
		// |        | Prim Sys Clk | USB 48Mhz |                 | Standard: 96Mhz (1010)         |
		//
		// (1) Bit 5:4 – PLLTM1:0: PLL Postcaler for High Speed Timer
		// | PLLTM1 | PLLTM0 | PLL Postcaler Factor |
		// |--------|--------|----------------------|
		// |      0 |      0 | 0 (Disconnected)     |
		// |      0 |      1 | 1                    |
		// |      1 |      0 | 1.5                  |
		// |      1 |      1 | 2                    |
		//
		if (microseconds < 1000) {
			cycles = (F_PLL / 2000000) * microseconds;
			//PLLFRQ = (PLLFRQ & 0xCF) | 0x10; // Use PLL 96MHz
			PLLFRQ = 0x5A; // something resets the PLL to 48Mhz after a power cycle, not sure why, but set the PLL correctly, see issue #1
		} else if (microseconds < 2000) {
			cycles = (F_PLL / 2000000 /2) * microseconds;
			//PLLFRQ = (PLLFRQ & 0xCF) | 0x30; // Use PLL 96MHz / 2 = 48MHz
			PLLFRQ = 0x7A; // see above
		} else {
			PLLFRQ = (PLLFRQ & 0xCF) | 0x00; // Use system clock
		}

		if (cycles < TIMER4_RESOLUTION) {
			clockSelectBits = _BV(CS40);
			pwmPeriod = cycles;
		} else if (cycles < TIMER4_RESOLUTION * 2) {
			clockSelectBits = _BV(CS41);
			pwmPeriod = cycles / 2;
		} else if (cycles < TIMER4_RESOLUTION * 4) {
			clockSelectBits = _BV(CS41) | _BV(CS40);
			pwmPeriod = cycles / 4;
		} else if (cycles < TIMER4_RESOLUTION * 8) {
			clockSelectBits = _BV(CS42);
			pwmPeriod = cycles / 8;
		} else if (cycles < TIMER4_RESOLUTION * 16) {
			clockSelectBits = _BV(CS42) | _BV(CS40);
			pwmPeriod = cycles / 16;
		} else if (cycles < TIMER4_RESOLUTION * 32) {
			clockSelectBits = _BV(CS41) | _BV(CS42);
			pwmPeriod = cycles / 32;
		} else if (cycles < TIMER4_RESOLUTION * 64) {
			clockSelectBits = _BV(CS42) | _BV(CS41) | _BV(CS40);
			pwmPeriod = cycles / 64;
		} else if (cycles < TIMER4_RESOLUTION * 128) {
			clockSelectBits = _BV(CS43);
			pwmPeriod = cycles / 128;
		} else if (cycles < TIMER4_RESOLUTION * 256) {
			clockSelectBits = _BV(CS43) | _BV(CS40);
			pwmPeriod = cycles / 256;
		} else if (cycles < TIMER4_RESOLUTION * 512) {
			clockSelectBits = _BV(CS43) | _BV(CS41);
			pwmPeriod = cycles / 512;
		} else if (cycles < TIMER4_RESOLUTION * 1024) {
			clockSelectBits = _BV(CS43) | _BV(CS41) | _BV(CS40);
			pwmPeriod = cycles / 1024;
		} else if (cycles < TIMER4_RESOLUTION * 2048) {
			clockSelectBits = _BV(CS42) | _BV(CS43);
			pwmPeriod = cycles / 2048;
		} else if (cycles < TIMER4_RESOLUTION * 4096) {
			clockSelectBits = _BV(CS43) | _BV(CS42) | _BV(CS40);
			pwmPeriod = cycles / 4096;
		} else if (cycles < TIMER4_RESOLUTION * 8192) {
			clockSelectBits = _BV(CS43) | _BV(CS42) | _BV(CS41);
			pwmPeriod = cycles / 8192;
		} else if (cycles < TIMER4_RESOLUTION * 16384) {
			clockSelectBits = _BV(CS43) | _BV(CS42) | _BV(CS41) | _BV(CS40);
			pwmPeriod = cycles / 16384;
		} else {
			clockSelectBits = _BV(CS43) | _BV(CS42) | _BV(CS41) | _BV(CS40);
			pwmPeriod = TIMER4_RESOLUTION - 1;
		}

		// For 10-bits all low-byte registers share the same high-byte register, 
		//  and read the high-byte when the low-byte is written
		TC4H = pwmPeriod >> 8;
		OCR4C = pwmPeriod;  // TODO: is this the replacement for ICR4? (ICR4 doesn't exist for Timer4)
		//TCCR4E ENHC4: Enhanced Compare/PWM Mode, implement extra accuracy bit?
		//
		// | PWM4x | WGM41..40 | Timer/Counter Mode of Operation |  TOP  | Update ofOCR4x at | TOV4 FlagSet on |
		// |-------|-----------|---------------------------------|-------|-------------------|-----------------|
		// |     0 | xx        | Normal                          | OCR4C | Immediate         | TOP             |
		// |     1 | 00        | Fast PWM                        | OCR4C | TOP               | TOP             |
		// |     1 | 01        | Phase and Frequency Correct PWM | OCR4C | BOTTOM            | BOTTOM          |
		// |     1 | 10        | PWM6 / Single-slope             | OCR4C | TOP               | TOP             |
		// |     1 | 11        | PWM6 / Dual-slope               | OCR4C | BOTTOM            | BOTTOM          |
		//
		TCCR4D = ~_BV(WGM41) & _BV(WGM40);
		TCCR4B = clockSelectBits;
    }

    //****************************
    //  Run Control
    //****************************
    void start() __attribute__((always_inline)) {
		TCCR4B = 0;
		TCCR4D = 0;
		TC4H = 0;
		TCNT4 = 0;		// TODO: does this cause an undesired interrupt?
		resume();
    }
    void stop() __attribute__((always_inline)) {
		TCCR4B = 0;
    }
    void restart() __attribute__((always_inline)) {
		start();
    }
    void resume() __attribute__((always_inline)) {
		TCCR4B = clockSelectBits;
		TCCR4D = ~_BV(WGM41) & _BV(WGM40);
    }

    //****************************
    //  PWM outputs
    //****************************
    /*
     * Duty goes from 0 to 1023.
     */
    void setPwmDuty(char pin, unsigned int duty) __attribute__((always_inline)) {
		unsigned long dutyCycle = pwmPeriod;
		dutyCycle *= duty;
		dutyCycle >>= 10;
		// For 10-bits all low-byte registers share the same high-byte register, 
		//  and read the high-byte when the low-byte is written
		TC4H = dutyCycle >> 8;
		if (pin == TIMER4_A_PIN || pin == TIMER4_AC_PIN) OCR4A = dutyCycle;
		else if (pin == TIMER4_B_PIN || pin == TIMER4_BC_PIN) OCR4B = dutyCycle;
		else if (pin == TIMER4_D_PIN || pin == TIMER4_DC_PIN) OCR4D = dutyCycle;
    }
    void pwm(char pin, unsigned int duty) __attribute__((always_inline)) {
		//
		// | COM1A1..0 |                OCW1A Behavior                |   OC4A Pin   |   OC4A Pin   |
		// |-----------|----------------------------------------------|--------------|--------------|
		// |        00 | Normal port operation.                       | Disconnected | Disconnected |
		// |        01 | Cleared on Compare Match when up-counting.   |              |              |
		// |           | Set on Compare Match when down-counting.     | Connected    | Connected    |
		// |        10 | Cleared on Compare Match when up-counting.   |              |              |
		// |           | Set on Compare Match when down-counting.     | Connected    | Disconnected |
		// |        11 | Set on Compare Match when up-counting.       |              |              |
		// |           | Cleared on Compare Match when down-counting. | Connected    | Disconnected |
		//
		//TODO: do not write |= 
		if (pin == TIMER4_A_PIN) { pinMode(TIMER4_A_PIN, OUTPUT); TCCR4A |= _BV(COM4A1) | _BV(PWM4A); }
		else if (pin == TIMER4_AC_PIN) { pinMode(TIMER4_AC_PIN, OUTPUT); TCCR4A |= _BV(COM4A0) | _BV(PWM4A); }
		else if (pin == TIMER4_B_PIN) { pinMode(TIMER4_B_PIN, OUTPUT); TCCR4A |= _BV(COM4B1) | _BV(PWM4B); }
		else if (pin == TIMER4_BC_PIN) { pinMode(TIMER4_BC_PIN, OUTPUT); TCCR4A |= _BV(COM4B0) | _BV(PWM4B); }
		else if (pin == TIMER4_D_PIN) { pinMode(TIMER4_D_PIN, OUTPUT); TCCR4C |= _BV(COM4D1) | _BV(PWM4D); }
		else if (pin == TIMER4_DC_PIN) { pinMode(TIMER4_DC_PIN, OUTPUT); TCCR4C |= _BV(COM4D0) | _BV(PWM4D); }
		setPwmDuty(pin, duty);
		TCCR4D = ~_BV(WGM41) & _BV(WGM40);
		TCCR4B = clockSelectBits;
    }
    void pwm(char pin, unsigned int duty, unsigned long microseconds) __attribute__((always_inline)) {
		if (microseconds > 0) setPeriod(microseconds);
		pwm(pin, duty);
    }
    void disablePwm(char pin) __attribute__((always_inline)) {
		if (pin == TIMER4_A_PIN) TCCR4A &= ~_BV(COM4A1);
		else if (pin == TIMER4_AC_PIN) TCCR4A &= ~_BV(COM4A0);
		else if (pin == TIMER4_B_PIN) TCCR4A &= ~_BV(COM4B1);
		else if (pin == TIMER4_BC_PIN) TCCR4A &= ~_BV(COM4B0);
		else if (pin == TIMER4_D_PIN) TCCR4C &= ~_BV(COM4D1);
		else if (pin == TIMER4_DC_PIN) TCCR4C &= ~_BV(COM4D0);
    }

    //****************************
    //  Interrupt Function
    //****************************
    void attachInterrupt(void (*isr)()) __attribute__((always_inline)) {
		isrCallback = isr;
		TIMSK4 = _BV(TOIE4);
    }
    void attachInterrupt(void (*isr)(), unsigned long microseconds) __attribute__((always_inline)) {
		if(microseconds > 0) setPeriod(microseconds);
		attachInterrupt(isr);
    }
    void detachInterrupt() __attribute__((always_inline)) {
		TIMSK4 = 0;
    }
    static void (*isrCallback)();
	static void isrDefaultUnused();

  private:
    // properties
    static unsigned int pwmPeriod;
    static unsigned char clockSelectBits;

#endif
};

extern TimerFour Timer4;

#endif

