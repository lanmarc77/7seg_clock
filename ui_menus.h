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

#ifndef UI_MENUS_H
#define UI_MENUS_H

extern unsigned int ui_menues_main_menu_input(void);
extern unsigned char ui_menues_check_alarm(void);
extern unsigned char ui_menues_check_schedule(void);
extern void ui_menues_load_schedules(void);

#define UI_MENUES_STOP_BEEP_LONG 0
#define UI_MENUES_STOP_BEEP_SHORT 1
extern void ui_menues_set_stop_beep_mode(unsigned char m);


#define UI_MENUES_ALARM_OFF 0
#define UI_MENUES_ALARM_ON 1
#define UI_MENUES_ALARM_ON_MP3 2
#define UI_MENUES_ALARM_ON_RADIO 3
extern void ui_menues_set_alarm_mode(unsigned char m);
extern void ui_menues_set_alarm_hour(unsigned char m);
extern void ui_menues_set_alarm_minute(unsigned char m);
extern void ui_menues_set_alarm_mp3_track(unsigned char m);



#define UI_MENUES_CONT_MODE_ON 0
#define UI_MENUES_CONT_MODE_OFF 1
extern void ui_menues_set_cont_mode(unsigned char m);
extern unsigned char ui_menues_get_cont_mode(void);

#endif