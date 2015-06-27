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

#ifndef SEG_FUNC_H
#define SEG_FUNC_H

extern unsigned char segments_ISR(void);
extern void segments_init(void);
extern void refresh_display_content(void);

#define SEG_BRIGHT 0
#define SEG_DIM 1
#define SEG_FLICKER 2
#define SEG_OFF 3
#define SEG_FLASH 4
#define SEG_DIM_FLICKER 5
#define SEG_ZOOM 6
extern unsigned char I_SEG_MODE;
#define COLON_ON 0
#define COLON_OFF 1
extern unsigned char I_COLON_MODE;
#define BEEPER_ON 0
#define BEEPER_OFF 1
#define BEEPER_ON_FULL 2
extern unsigned char I_BEEPER_MODE;

extern unsigned char segment_mode;

extern volatile unsigned char dimm_value;
extern volatile unsigned char bright_value;

extern unsigned char I_digits[4];
#define L_0 0
#define L_1 1
#define L_2 2
#define L_3 3
#define L_4 4
#define L_5 5
#define L_6 6
#define L_7 7
#define L_8 8
#define L_9 9
#define L_NOTHING 10
#define L_C 11
#define L_- 12
#define L_L 13
#define L_b 14
#define L_d 15
#define L_o 16
#define L_E 17
#define L__ 18
#define L_F 19
#define L_A 20
#define L_I 21
#define L_S 5
#define L_t 22
#define L_P 23
#define L_M 24
#define L_Y 25
#define L_n 26
#define L_r 27
#define L_W 28
#define L_H 29
#define L_U 30
#define L_v 31

#define L_b0 32
#define L_b1 33
#define L_b2 34
#define L_b3 35
#define L_b4 36
#define L_b5 37
#define L_b6 38
#define L_b7 39
#define L_b8 40
#define L_b9 41


#endif