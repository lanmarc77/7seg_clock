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

#ifndef UI_IR_H
#define UI_IR_H

extern unsigned int get_ir_input(unsigned char keys, unsigned char pw_mode, unsigned int timeout,unsigned char i0, unsigned char i1, unsigned char i2, unsigned char i3);
extern unsigned int code_input(void);
extern unsigned char number_input(unsigned char *number,unsigned char min,unsigned char max,unsigned int timeout);
extern unsigned char get_ir_code(void);
extern unsigned int last_ir_code;
extern void ir_init(void);

/*
	0xff: no key pressed
	other see defines below
*/
#define IR_POWER 0x12
/*#define IR_VOL_PLUS 0x13
#define IR_VOL_MINUS 0x14
#define IR_0 0x15
#define IR_1 0x16
#define IR_2 0x17
#define IR_3 0x18
#define IR_4 0x19
#define IR_5 0x1A
#define IR_6 0x1B
#define IR_7 0x1C
#define IR_8 0x1D
#define IR_9 0x1E*/
#define IR_CH_PLUS 0x1F
#define IR_CH_MINUS 0x20
#define IR_MUTE 0x21

#endif