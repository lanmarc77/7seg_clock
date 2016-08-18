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


#ifndef SETTINGS_H
#define SETTINGS_H


#define SETTINGS_DARK_BRIGHTNESS 11
#define SETTINGS_LIGHT_BRIGHTNESS 12
#define SETTINGS_ALARM_MODE 13
#define SETTINGS_ALARM_HOUR 14
#define SETTINGS_ALARM_MINUTE 15



#define SETTINGS_ALARM_MP3_TRACK 19
#define SETTINGS_CONT_MODE 20
#define SETTINGS_DST_MODE 21

#define SETTINGS_UI_MENUES_CODE_10 24
#define SETTINGS_UI_MENUES_CODE_32 25
#define SETTINGS_MP3_VOLUME 26


#define SETTINGS_MAX_SCHEDULES 50
#define SETTINGS_SCHEDULE_OFFSET 100

/*
#define SETTINGS_DISPLAY_MODE 10

#define SETTINGS_SEGMENT_MODE 16
#define SETTINGS_DISPLAY_FIXED_MODE 17
#define SETTINGS_DCF77_SIGNAL_TYPE 18

#define SETTINGS_ANIM_MODE 22
#define SETTINGS_UI_DISPLAY_DOT_MODE 23

*/

extern void settings_save(unsigned int addr,unsigned char value);
extern unsigned char settings_get(unsigned int addr);


#endif
