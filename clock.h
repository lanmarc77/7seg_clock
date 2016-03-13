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

extern void clock_isr_part(void); //call every 4ms
extern void clock_init(void);
#define CLOCK_UPDATE_SOURCE_RTC 1
#define CLOCK_UPDATE_SOURCE_DCF77 2
#define CLOCK_UPDATE_SOURCE_MANUAL 3
extern void clock_set_time(unsigned char t_min,unsigned char t_hour,unsigned char t_second,unsigned char t_day,unsigned char t_month,unsigned char t_year,unsigned char t_dow,unsigned char source);
extern unsigned int clock_get_last_refresh(unsigned char source);
extern void clock_get_time(unsigned char *t_min,unsigned char *t_hour,unsigned char *t_second,unsigned char *t_day,unsigned char *t_month,unsigned char *t_year,unsigned char *t_dow);

extern signed int clock_get_stop_watch(void);
extern void clock_start_stop_watch(void);
extern void clock_stop_stop_watch(void);
extern void clock_pause_stop_watch(void);
extern void clock_unpause_stop_watch(void);

#define CLOCK_DST_MODE_OFF 0
#define CLOCK_DST_MODE_EU 1
#define CLOCK_DST_MODE_US 2
extern void clock_set_dst_mode(unsigned char mode);
extern unsigned char clock_get_dst_mode(void);

extern volatile unsigned char debug_char;

#endif