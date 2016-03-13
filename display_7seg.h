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

#ifndef DISPLAY_7SEG_H
#define DISPLAY_7SEG_H

extern void display_7seg_init(void);
extern unsigned char display_7seg_ISR(void);

#define DISPLAY_7SEG_ANIM_OFF 0
#define DISPLAY_7SEG_ANIM_ODO 1
extern void display_7seg_set_anim_mode(unsigned char c);
extern unsigned char display_7seg_get_anim_mode(void);
extern void display_7seg_anim_on(void);

#define DISPLAY_7SEG_POLARITY_NORMAL 0
#define DISPLAY_7SEG_POLARITY_INV 1
extern void display_7seg_set_polarity(unsigned char c);

#define DISPLAY_7SEG_BRIGHT 0
#define DISPLAY_7SEG_DIM 1
#define DISPLAY_7SEG_FLICKER 2
#define DISPLAY_7SEG_OFF 3
#define DISPLAY_7SEG_FLASH 4
#define DISPLAY_7SEG_DIM_FLICKER 5
#define DISPLAY_7SEG_ZOOM 6
extern void display_7seg_set_mode(unsigned char c);

extern void display_7seg_set_time(char *c);
extern void display_7seg_set_text(char *c);

extern void display_7seg_set_bright_level(unsigned char c);
extern void display_7seg_set_dark_level(unsigned char c);

extern unsigned char display_7seg_get_bright_level(void);
extern unsigned char display_7seg_get_dark_level(void);


#endif