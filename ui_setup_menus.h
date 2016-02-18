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

#ifndef UI_SETUP_MENUS_H
#define UI_SETUP_MENUS_H

extern unsigned int main_menu_input(void);
extern unsigned char check_alarm(void);
extern unsigned char check_schedule(void);
extern unsigned int setup_schedule(void);
extern void load_schedules(void);


#define ALARM_OFF 0
#define ALARM_ON 1
#define ALARM_ON_MP3 2
#define ALARM_ON_RADIO 3
#define ALARM_RINGING 4
#define ALARM_PAUSED 5
extern unsigned char alarm_mode;
extern unsigned char alarm_hour;
extern unsigned char alarm_minute;
extern unsigned char alarm_track;

#define STOP_BEEP_LONG 0
#define STOP_BEEP_SHORT 1
extern unsigned char stop_beep_mode;


#define CONT_MODE_ON 0
#define CONT_MODE_OFF 1
extern unsigned char cont_mode;



#endif