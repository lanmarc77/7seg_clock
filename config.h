/*
Copyright (C) 2015  Marcel Langner (langner.marcel@myiq.de)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CONFIG_H
#define CONFIG_H

/*Version history
001: initial start of version as part of redesigning the clocks, IR remote with RC5 implemented, FIXED_MODE possibility
002: added bernd private mode as CTYPE_8
003: fixed logic error in bernd private mode, made dimmed mode for CTYPE_5 even dimmer
004: fixed IR remote input for CTYPE_7
005: added setting to setup brightness, added protection for acidentically exiting the stop watch mode, implemented TE mode, remove CTYPE_5 dimmed mode from v003
006: added ILLU mode for fixed mode clocks, added . in date display in all display modes, renamed TE mode to WBS mode
007: added text for day of week instead of number in date display, PD7 is now configured as output and delivers high,added TE mode, added binary mode
008: added new flicker mode zoom for testing currently in stop mode only, colon only active in CTYPE_8 and Bernd mode and during time display
     colon now also honors brightness settings, change display logic of single dot in time display for bernd mode
009: added colon in all display modes for time display, exchanged FLICKER with ZOOM visualisation to avoid epilepsy with sensitive persons
	 configured PC2 as beeper port off/low by default
00A: added beeper in stop clock mode during the last minute
00B: stop clock can be setup in 1 minute steps as well, dcf77 status blink more usefull now reflects the filtered signal
	 added alarm clock mode
	 added setup for common anode/cathode display (press the ir power key during version display after power up toggles between the modes)
	 added setup for fixed mode common (press the ir mute key during version display after power up toggles between fixed mode and not)
	 added setup for inverted DCFF77 input signal (press the ir channel down key during version display after power up toggles between inverted non inverted signal)
	 added Bernd private (as C2) to the general modes
	 removed all clock types (CTYPE_*) as everything can be setup now in software (assuming/requiring an IR receiver on PC6)
	 total code cleanup and .c/.h separation
00C: default values for setups adjusted to fit HW4.0,
	 changed beeper behaviour for stop clock
	 clock only mode C3 added
	 ringing alarm clock can be disabled via any IR remote key
00D: brightness dimmed to minimum will switch off the display
	 stop clock mode beep type can be fixed based on display mode
	 stop clock mode brightness changed to bright value to avoid invisible clock if dimmed value is 0
	 stop clock can be pause/unpaused via mute key
	 changed timing of date display in TE mode
00E: C1 beep type changed for stop clock mode
00F: added serial time output, changed DCF77 decoding, now finds a signal better under noisy conditions, changed license to GPLv3, added schedule based beeps
010: removed automatic blink/zoom schedules for mode C1, fixed bug in all modes for first block zoom
011: modularizes all I2C devices as drivers
     bumped the number of schedules to 50
	 added I2C mp3 player support
	 fixed DCF decoding error which could lead to a one minute delay
     changed every menu and input to only need a 4 button input (up, down, ok, back/cancel)
	 better control of setting brightness
012: more modularization of all components for future extendability and exchangebility, code cleanup
	 added clock odometer animation
	 added automatic day light saving time adjustments for EU and US area in case of no dcf signal reception
	 added setup for time display dot/colon behaviour
	 added volume setup for ambient and sleep mp3 tracks
	 changed initial setup screen for inverted dcf, inverted 7segment and fixed mode
	 added support for I2C button/ir module
013: code can now be changed and is stored in eeprom
	 new way of entering code to avoid eves dropping
014: added more modes, started moving to compile based define features
015: fixed errors in VHT and TFV modes
016: modes are now individually compiled
017: textual changes for better understanding of compile options, removed info screen during startup as options are now compiled in, code cleanup
*/

#define VERSION "v017"

#include "compile.h"

#endif
/*

//Compile option=multilanguage description of the compile option
//Compile option:value or empty:dependency of other compile options or empty:multilanguage description

UI_DISPLAY_MODE=This option determines the way the display shows the time. It defines it's main operating mode.
UI_DISPLAY_MODE:1::TA BERLIN - This mode is meant to visualize lesson hour starts and breaks for the vocational school Technik Akademie in Berlin, Germany
UI_DISPLAY_MODE:2::WBS - This mode is meant to visualize lesson hour starts and breaks for the vocational school Werner von Siemens Werksberufschule in Berlin, Germany
UI_DISPLAY_MODE:3::BINARY - This mode displays the time in a binary format, using the segments as bits
UI_DISPLAY_MODE:4::TB - This mode is meant to visualize lesson hour starts and breaks for the vocational school Technische Bildung in Berlin, Germany
UI_DISPLAY_MODE:5::C1 - Shows the current time usually dimmed but switches to full brightness every full hour for one minute
UI_DISPLAY_MODE:6::C2 - Shows the current time and switches automatically to display date and temperature for 5 seconds every 20 seconds
UI_DISPLAY_MODE:7::C3 - Shows only the current time. Dimms the brightness between 22:00-07:00
UI_DISPLAY_MODE:8::SIMPLE - Shows the current time and switches automatically to temperature for 1 seconds every 10 seconds
UI_DISPLAY_MODE:9::BEUTH - This mode is meant to visualize lesson hour starts and breaks for the Beuth University in Berlin, Germany
UI_DISPLAY_MODE:10::TFV - This mode is meant to support game play of the "Teltower Fussball Verein", a soccer club in Teltow, Germany

UI_DISPLAY_DOT_MODE=This option describes the behaviour of the seperator between hours and minutes which can be a dot and/or a colon
UI_DISPLAY_DOT_MODE:0::The seperator dot and the seperator colon between hour and minute are not shown at all
UI_DISPLAY_DOT_MODE:1::The seperator dot between hour and minute is always on, the colon is off
UI_DISPLAY_DOT_MODE:2::The seperator dot between hour and minute is reflecting the DCF77 signal and stays fully on, if a valid DCF77 signal was received during the last 24hours(wo RTC) or a week(if RTC is built in), the colon is off
UI_DISPLAY_DOT_MODE:3::The seperator colon between hour and minute is always in, the dot is off
UI_DISPLAY_DOT_MODE:4::The seperator colon between hour and minute is reflecting the DCF77 signal and stays fully on, if a valid DCF77 signal was received during the last 24hours(wo RTC) or a week(if RTC is built in), the dot is off

DISPLAY_7SEG_ANIM_MODE=Determines if the clock should show an animation if time changes.
DISPLAY_7SEG_ANIM_MODE:0::no animation is shown, when the clock changes to a new minute
DISPLAY_7SEG_ANIM_MODE:1::an odo meter like animation is shown, when the clock changes to a new minute

DISPLAY_7SEG_POLARITY=Determines in which way the 7 segments are connected to the display.
DISPLAY_7SEG_POLARITY:0::normal polarity of the LED 7 segments
DISPLAY_7SEG_POLARITY:1::inverted polarity of the LED 7 segments

DCF77_SIGNAL_TYPE=Determines if the DCF77 receiver delivers a low active signal pattern or not.
DCF77_SIGNAL_TYPE:0::The DCF77 outputs a normal high active signal (100/200ms pulses are transmitted as high)
DCF77_SIGNAL_TYPE:1::The DCF77 outputs a low active/inverted signal (100/200ms pulses are transmitted as low)


*/

