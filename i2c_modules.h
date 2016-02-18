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

#ifndef I2C_MODULES_H
#define I2C_MODULES_H

extern unsigned char I2C_RTC_detected;
extern unsigned char I2C_TEMP_detected;
extern unsigned char I2C_TEMP_OUT_detected;
extern unsigned char I2C_RADIO_detected;
extern unsigned char I2C_MP3_detected;
extern unsigned char I2C_MOTION_detected;
extern unsigned char I2C_LIGHT_detected;
volatile unsigned char I2C_busy;


/* global functions */
extern void I2C_init_modules(void);
extern void I2C_check_state_machines(void);


/* RTC functions */
extern char I2C_RTC_getTime(void);
extern char I2C_RTC_setTime(void);

/* Temperature functions */
extern char I2C_getTemp(unsigned int *temp);


/* MP3 functions */
extern char I2C_MP3_talkTime(void);
extern char I2C_MP3_playCont(void);
extern char I2C_MP3_stopPlaying(void);
extern char I2C_MP3_playAlarm(unsigned char s);
extern char I2C_MP3_playSched(unsigned char s);
extern char I2C_MP3_playEvent(unsigned char s);
extern char I2C_MP3_playAmb(unsigned char s);


#endif