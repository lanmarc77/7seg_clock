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
	 

TODO:
- Prüfungszeitmodus für STA Abschlußpräsentationen
- DCF Emfang/Sync per Fernbedienung deaktivieren, im EEPROM speichern
- Blockzeiten per Fernbedienung einstellen, im EEPROM speichern
- Präsentationstimer in sekunden einstellbar
- alarm clock with I am awake function and No ring on saturday/sunday mode
- i²c audio player hinzufügen, danach: meditations timer, Schlafphasen wecker nach Vorbild der App Sanfter Wecker, stundensignal
*/

/*
EEPROM usage (addresses in decimal notation):
10: which mode is displayed
11: dimmed brightness value
12: bright brightness value
13: alarm mode
14: alarm hour
15: alarm minute
16: segment mode (CA/CC)
17: fixed mode
18: inverted dcf77 signal

100:schedule setup 1
105:schedule setup 2
110:schedule setup 3
....
195:schedule setup 20

*/

#define VERSION_1 0
#define VERSION_2 1
#define VERSION_3 0


#endif
