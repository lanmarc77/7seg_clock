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

#ifndef CLOCK_H
#define CLOCK_H

#define DCF77_PIN (PINC&0x80)

extern void dcf77_init(void);
extern void dcf77_isr_part(unsigned char dsd);
extern void get_time(void);
extern signed int get_stop_watch(void);
extern void start_stop_watch(void);
extern void stop_stop_watch(void);
extern void pause_stop_watch(void);
extern void unpause_stop_watch(void);


extern unsigned char I_second,I_hour,I_minute,I_year,I_dow,I_month,I_day;
extern volatile unsigned char hour,min,second,day,month,year,dow;
extern volatile unsigned int no_dcf_signal;
extern volatile unsigned char dcf77_inverted_flag;
extern volatile unsigned char filtered_dcf77;
extern volatile unsigned char dcf_state;
extern volatile unsigned char new_time_flag;



/* these variables give direct access to the internal clock system
   use carefully, better not at all
*/
extern volatile unsigned char INT_hour;//0
extern volatile unsigned char INT_minute;//0
extern volatile unsigned char INT_second;//0
extern volatile unsigned char INT_day;//1
extern volatile unsigned char INT_month;//1
extern volatile unsigned char INT_year;//0
extern volatile unsigned char INT_dow;//6
extern volatile signed int timer2_counter;


extern volatile unsigned char debug_char;

#endif