/* This code was made for use on a NetBurner MOD5234
 * Justin Smalley, 2008
 *
 * Nancy Minderman
 * nancy.minderman@ualberta.ca
 * Modified for the CMPE401 Fall2009 semester
 * This file now lives in user space to ensure student access.
 *
 * Modified for the CMPE401 Fall2010 semester
 * Added screen granularity so we can control both screens independently
 *
 * Modified for the CMPE401 Fall2011 semester
 *
 * Modified for the ECE315 Fall2012 semester
 * corrected bug in move method to compensate for line two starting
 * in LCD memory at 0x40
 *
 * Modified for the ECE315 Fall 2013 semester
 * Removed the unused single mode. Removed LCD_ECHO mode. LCD_DEBUG is enough
 *
 * Modified for the ECE 315 Winter 2014 semester
 * Included the display_error wrapper function.
 *
 * */

#include "predef.h"
#include <stdio.h>
#include <ctype.h>
#include <startnet.h>
#include <http.h>
#include <ucosmcfc.h>
#include <system.h>
#include <iosys.h>
#include <autoupdate.h>
#include <serial.h>
#include <dhcpclient.h>
#include <taskmon.h>
#include <NetworkDebug.h>
#include <pins.h>
#include "pinmap.h"
#include "lcd.h"

/* Name:  send_cmd
 * Description: Sends a command to the LCD.
 * The screen is specified as one of LCD_UPPER_SCR, LCD_LOWER_SCR, or LCD_BOTH_SCR
 * A proper command is specified by a constant that starts with
 * CMD_* and is ored with a valid option selection for that command.
 * see lcd.h for valid options
 * Inputs: unsigned char screen, unsigned char cmd
 * Outputs: none
 */
void Lcd::send_cmd(unsigned char screen, unsigned char cmd)
{

   set_data_lines(cmd);
   set_rs_line(RS_CMD);

   //debug outputs. These can be turned on and off in lcd.h
   LCD_DEBUG(iprintf("(%3u) <%u%u%u%u %u%u%u%u>\n", cmd, (cmd >> 7) & 0x01,(cmd >> 6) & 0x01,(cmd >> 5) & 0x01,(cmd >> 4) & 0x01,(cmd >> 3) & 0x01,(cmd >> 2) & 0x01,(cmd >> 1) & 0x01,cmd & 0x01));
   OSTimeDly(1);

   if (screen == LCD_UPPER_SCR){
	   enable_upper();
   } else if (screen == LCD_LOWER_SCR) {
	   enable_lower();
   } else if (screen == LCD_BOTH_SCR) {
	   enable_upper();
	   enable_lower();
   } else {
	   LCD_DEBUG(iprintf("LCD::send_cmd wrong screen type\n"));
   }
}

/* Name: send_data
 * Description: Sends a single character to the LCD.
 * Given that the LCD is divided into 2 separate screens
 * it must be sent to the correct one based on the current position.
 * Inputs:
 * unsigned char screen must be one of LCD_UPPER_SCR
 * LCD_LOWER_SCR, LCD_BOTH_SCR or LCD_SINGLE_SCR
 * unsigned char data must be a printable ASCII character.
 * Outputs: none
 */
void Lcd::send_data(unsigned char  screen, unsigned char data)
{
	set_data_lines(data);
	set_rs_line(RS_DATA);
	/* need a minimal delay before toggling the enable lines */
	/* one tick is the shortest possible delay reagrdless of
	 * TICKS_PER_SECOND setting*/
	OSTimeDly(1);
	if (screen == LCD_UPPER_SCR)
	{
		enable_upper();
	}else if (screen == LCD_LOWER_SCR) {
		enable_lower();
	} else if (screen == LCD_BOTH_SCR){
		enable_upper();
		enable_lower();
	} else {
		LCD_DEBUG(iprintf("LCD::send_data invalid screen"));
	}
}

/* Name: set_data_lines
 * Description: Bit bang to set the GPIO lines corresponding to the
 * data lines.
 * Inputs: unsigned char (8 bits) of data to send.
 * Outputs: none
 */
void Lcd::set_data_lines(unsigned char data)
{
	 //set data lines only
	   LCD_D0 = data & LSBIT;
	   LCD_D1 = (data >> 1) & LSBIT;
	   LCD_D2 = (data >> 2) & LSBIT;
	   LCD_D3 = (data >> 3) & LSBIT;
	   LCD_D4 = (data >> 4) & LSBIT;
	   LCD_D5 = (data >> 5) & LSBIT;
	   LCD_D6 = (data >> 6) & LSBIT;
	   LCD_D7 = (data >> 7) & LSBIT;
}

/* Name: enable_upper
 * Description: Generates toggle of enable line to write data or
 * commands to the upper two lines of the LCD.
 * Inputs: none
 * Outputs: none
 */
void Lcd::enable_upper(void)
{
		// simple toggle from low to high to low
		LCD_E1 = HIGH;
		LCD_E1 = LOW;
}

/* Name:enable_lower
 * Description: Generates toggle of enable line to write data or
 * commands to the lower two lines of the LCD.
 * Inputs:
 * Outputs:
 */
void Lcd::enable_lower(void)
{
		// simple toggle from low to high to low
		LCD_E2 = HIGH;
		LCD_E2 = LOW;
}

/* Name: print_char
 * Description: Prints a single character to the LCD to
 * the current cursor position and increments the position.
 * Decrementing is currently unimplemented.
 * Inputs:
 * unsigned char screen must be one of LCD_UPPER_SCR or LCD_LOWER_SCR
 * LCD_BOTH_SCR is currently unimplemented
 * char c must be a printable ASCII character, a carriage return
 * or a newline character. Anything else will look like garbage.
 * Outputs: none
 */
void Lcd::print_char(unsigned char screen, char c)
{
	switch (screen) {
		case LCD_UPPER_SCR :
			if (c =='\n') {
				if (position_upper < LCD_COLS){
					move(screen, LCD_COLS);
				} else if (position_upper < (LCD_COLS*2)) {
					move(screen, LCD_ORIGIN);
				}
			} else if (c == '\r') {
				if (position_upper < LCD_COLS) {
					move(screen, LCD_ORIGIN);
				} else if ( position_upper < (LCD_COLS*2) ) {
					move(screen, LCD_COLS);
				}
			} else {
				send_data(screen, c);
				position_upper++; // by default we go forward
				if (position_upper > (LCD_COLS*2-1) ) {
					move(screen, LCD_ORIGIN);
				}
			}
			break;
		case LCD_LOWER_SCR :
			if (c =='\n') {
				if (position_lower < LCD_COLS){
					move(screen, LCD_COLS);
				} else if (position_lower < (LCD_COLS*2)) {
					move(screen, LCD_ORIGIN);
				}
			} else if (c == '\r') {
				if (position_lower < LCD_COLS) {
					move(screen, LCD_ORIGIN);
				} else if ( position_lower < (LCD_COLS*2) ) {
					move(screen, LCD_COLS);
				}
			} else {
				send_data(screen, c);
				position_lower++; // by default we go forward
				if (position_lower > (LCD_COLS*2-1) ) {
					move(screen, LCD_ORIGIN);
				}
			}
			break;
		case LCD_BOTH_SCR :
			/* Currently unimplemented */
			break;
		default:
			break;
	}

}

/* Name:set_rs_line
 * Description:Sets the RS line to be either hi (1) or low (0).
 * Inputs: unsigned char rs must be either RS_DATA or RS_CMD.
 * Unpredictable results will occur if you send it something else.
 * Outputs: none
 */
void Lcd::set_rs_line(unsigned char rs)
{
	LCD_RS = rs;
}

/* Name: move
 * Description: Moves the current cursor position but does not make
 * any change to data currently being displayed.
 * Inputs:
 * unsigned char screen should be LCD_UPPER_SCR
 * LCD_LOWER_SCR, or LCD_BOTH_SCR
 * unsigned char position should be between 0 and 79
 * Output: void
 */
void Lcd::move(unsigned char screen, unsigned char position)
{
	unsigned char line_adj = 0;
	if( position >= (LCD_COLS * 2)) {
		LCD_DEBUG(iprintf("Lcd::move bad position"));
		return;
	} else if ( (position >= LCD_COLS) && (position < LCD_COLS*2)) {
		line_adj = LCD_DDRAM_LINE2 - LCD_COLS;
	} else {
		line_adj = LCD_DDRAM_LINE1;
	}
	if (screen == LCD_UPPER_SCR){
			position_upper = position;
			send_cmd(LCD_UPPER_SCR, (CMD_DDRAM_ADR + position + line_adj ));
		} else if (screen == LCD_LOWER_SCR){
			position_lower = position;
			send_cmd(LCD_LOWER_SCR, (CMD_DDRAM_ADR + position + line_adj));
		} else if (screen == LCD_BOTH_SCR){
			position_upper = position;
			position_lower = position;
			send_cmd(LCD_BOTH_SCR, (CMD_DDRAM_ADR + position + line_adj));
	} else {
		LCD_DEBUG(iprintf("Lcd::move invalid screen\n"));
	}
}
/* Name: constructor
 * Description: Constructor for Lcd class. Sets ivars to known
 * correct initial value
 * Inputs: none
 * Outputs: none
 */
Lcd::Lcd()
{
	position_upper = 0;
	position_lower = 0;
	sem_inited = FALSE;

}
/**Name: setLCD
 * Description: sets the LCD as usual
 * Inputs: unsigned char screen should be one of LCD_UPPER_SCR, LCD_LOWER_SCR or LCD_BOTH_SCR
 * or LCD_LOWER_SCR
 * Outputs: none
 */
void Lcd::setLCD(unsigned char screen){
	send_cmd(screen, CMD_FUNCTION | FUNCTION_8BIT | FUNCTION_2LINE | FUNCTION_5x10);
	send_cmd(screen, CMD_CLEAR);
	send_cmd(screen, CMD_DISPLAY | DISPLAY_ON | DISPLAY_NOCURSOR | DISPLAY_NOBLINK);
	send_cmd(screen, CMD_ENTRY_MODE | ENTRY_CURSOR_INC | ENTRY_NOSHIFT);
	send_cmd(screen, CMD_SHIFT | SHIFT_CURSOR | SHIFT_RIGHT);
	send_cmd(screen, CMD_HOME);
}
/* Name: Init
 * Description: Initializes the hardware and the internal semaphore
 * to a known good state. The semaphores guarantee atomic instructions.
 * Inputs: unsigned char screen should be one of LCD_UPPER_SCR, LCD_LOWER_SCR
 * or LCD_LOWER_SCR
 * Outputs: none
 */

void Lcd::Init(unsigned char screen)
{
	BYTE err = OS_NO_ERR;

	//set hardware lines to gpio, and put into initial state
	LCD_RS.function(PIN_GPIO); LCD_RS = 0;
	LCD_E1.function(PIN_GPIO); LCD_E1 = 0;
	LCD_E2.function(PIN_GPIO); LCD_E2 = 0;
	LCD_D0.function(PIN_GPIO); LCD_D0 = 0;
	LCD_D1.function(PIN_GPIO); LCD_D1 = 0;
	LCD_D2.function(PIN_GPIO); LCD_D2 = 0;
	LCD_D3.function(PIN_GPIO); LCD_D3 = 0;
	LCD_D4.function(PIN_GPIO); LCD_D4 = 0;
	LCD_D5.function(PIN_GPIO); LCD_D5 = 0;
	LCD_D6.function(PIN_GPIO); LCD_D6 = 0;
	LCD_D7.function(PIN_GPIO); LCD_D7 = 0;

	//Initialize the internal semaphore here and not in constructor to avoid
	// an endless trap loop
	if (sem_inited == FALSE) {
		err = OSSemInit(&sem,1);
		if(err  == OS_NO_ERR){
			sem_inited = TRUE;
		} else {
			display_error("LCD::Init Error:", err);
		}
	}
	if (sem_inited == TRUE){
		err = OSSemPend(&sem, WAIT_FOREVER);
		if (err == OS_NO_ERR)
		{
			// exercise 2: decode and replace the send_cmd lines
			// 				turn the blink on in both upper and lower screens
			switch (screen)
			{
			/*
			 * 		send_cmd(LCD_LOWER_SCR, 56);
					send_cmd(LCD_LOWER_SCR, 1);
					send_cmd(LCD_LOWER_SCR, 12);
					send_cmd(LCD_LOWER_SCR, 6);
					send_cmd(LCD_LOWER_SCR, 20);
			 */
				case LCD_UPPER_SCR :
					setLCD(LCD_UPPER_SCR);
					break;
				case LCD_LOWER_SCR :
					setLCD(LCD_LOWER_SCR);
					break;
				case LCD_BOTH_SCR :
					setLCD(LCD_BOTH_SCR);
					break;
				default:
					LCD_DEBUG(iprintf("Lcd::Init incorrect screen\n"));
			}
			display_error("Lcd::Init Error:", OSSemPost(&sem));
		} else {
			display_error("Lcd::Init Error:", err);
			}
		}
}


/* Name:Clear
 * Description: Clears both screens and resets the hardware and position
 * variable to 0 (upper left position) on both screens.
 * The semaphores guarantee atomic instructions.
 * Inputs: unsigned char screen should be one of LCD_UPPER_SCR, LCD_LOWER_SCR
 * or LCD_LOWER_SCR
 * Outputs: none
 */
void Lcd::Clear(unsigned char screen)
{
	BYTE err = OS_NO_ERR;

	err = OSSemPend(&sem,WAIT_FOREVER);
	if (err == OS_NO_ERR)
	{
		// clear only the upper screen
		if( screen == LCD_UPPER_SCR) {
			position_upper = 0;
			send_cmd(LCD_UPPER_SCR, CMD_CLEAR);
		// clear only the lower screen
		} else if (screen == LCD_LOWER_SCR) {
			position_lower = 0;
			send_cmd(LCD_LOWER_SCR, CMD_CLEAR);
		// in this case clearing mirrored screens
		} else if (screen == LCD_BOTH_SCR) {
			position_upper = 0;
			position_lower = 0;
			send_cmd(LCD_BOTH_SCR, CMD_CLEAR);
		}
		OSTimeDly(TICKS_PER_SECOND/10);
		display_error ("Lcd::Clear Error:", OSSemPost(&sem));
	} else {
		display_error("Lcd::Clear Error:", err);
	}
}

/* Name: Home
 * Description: Resets the current cursor position to 0 (upper left)
 * on both screens. The semaphores guarantee atomic instructions.
 * Inputs: unsigned char screen should be one of LCD_UPPER_SCR, LCD_LOWER_SCR
 * or LCD_LOWER_SCR
 * Outputs: none
 */
void Lcd::Home(unsigned char screen)
{
	BYTE err = OS_NO_ERR;

	err = OSSemPend(&sem,WAIT_FOREVER);
	if (err == OS_NO_ERR )
	{
		send_cmd(screen,CMD_HOME);
		if (screen == LCD_UPPER_SCR){
			position_upper = 0;
		} else if (screen == LCD_LOWER_SCR) {
			position_lower = 0;
		} else if (screen == LCD_BOTH_SCR) {
			position_upper=0;
			position_lower=0;
		} else {
			LCD_DEBUG(iprintf("Lcd::Home unknown screen"));
		}
		OSTimeDly(TICKS_PER_SECOND/10);
		display_error ("Lcd::Home Error:", OSSemPost(&sem));
	} else {
		display_error("Lcd::Clear Error:", err);
	}
}

/* Name: MoveCursor
 * Description: Moves the current position of the cursor to the supplied
 * screen and position. The semaphores guarantee atomic instructions.
 * Inputs: unsigned char screen and position. screen should be
 * one of LCD_UPPER_SCR, LCD_LOWER_SCR, LCD_BOTH_SCR
 * position should be between 0-79 for all screens
 * Outputs: none
 */
void Lcd::MoveCursor(unsigned char screen, unsigned char position)
{

	BYTE err = OS_NO_ERR;

	err = OSSemPend(&sem,WAIT_FOREVER);
	if( err == OS_NO_ERR)
	{
		move(screen,position);
		display_error ("Lcd::MoveCursor Error:", OSSemPost(&sem));
	} else {
		display_error ("Lcd::MoveCursor Error:", err);
	}
}

/* Name: PrintChar
 * Description: Prints a single character to the LCD at the current
 * cursor position. The semaphores guarantee atomic instructions.
 * Inputs:
 * unsigned char screen should be one of LCD_UPPER_SCR, LCD_LOWER_SCR
 * Note that LCD_BOTH_SCR and LCD_SINGLE_SCR are unimplemented
 * char c must be a printable ascii character, a carriage
 * return or a newline character
 * Outputs: none
 */
void Lcd::PrintChar(unsigned char screen, char c)
{
	BYTE err = OS_NO_ERR;

	err = OSSemPend(&sem,WAIT_FOREVER);
	if(err == OS_NO_ERR)
	{
		print_char(screen, c);
		display_error ("Lcd::PrintChar Error:", OSSemPost(&sem));
	} else {
		display_error ("Lcd::PrintChar Error:", err);
	}
}

/* Name: PrintString
 * Description: Prints a c-style string to the LCD at the current cursor
 * position. The semaphores guarantee atomic instructions.
 * Inputs:
 * unsigned char screen must be LCD_UPPER_SCR, LCD_LOWER_SCR
 * const char * points to the first letter of the string.
 * Unpredictable results will occur if the string is not
 * null-terminated or if a NULL pointer is passed in.
 * Outputs: none
 */
void Lcd::PrintString(unsigned char screen, const char * str)
{
	BYTE err = OS_NO_ERR;
	int i = 0;

	err = OSSemPend(&sem,WAIT_FOREVER);
	if(err == OS_NO_ERR)
	{
		while (str[i] != '\0') {
			print_char(screen, str[i]);
			i++;
		}
		display_error ("LCD::PrintString Error:", OSSemPost(&sem));
	} else {
		display_error ("LCD::PrintChar Error:",err);
	}
}

/* Name: ShiftDisplayRight
 * Description:
 * Inputs:
 * Outputs:
 */
void Lcd::ShiftDisplayRight(unsigned char screen){
}

/* Name: ShiftDisplayLeft
 * Description:
 * Inputs:
 * Outputs:
 */
void Lcd::ShiftDisplayLeft(unsigned char screen){
}


