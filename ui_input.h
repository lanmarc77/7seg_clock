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

#ifndef UI_INPUT_H
#define UI_INPUT_H

extern unsigned char ui_input_number(unsigned char *number,unsigned char min,unsigned char max,unsigned int timeout);

#define UI_INPUT_KEY_NONE 0
#define UI_INPUT_KEY_OK 1
#define UI_INPUT_KEY_BACK 2
#define UI_INPUT_KEY_UP 3
#define UI_INPUT_KEY_DOWN 4
extern unsigned char ui_input_get_key(void);
extern void ui_input_simulate(unsigned char k);

extern void ui_input_init(void);

extern unsigned char ui_input_get_digits(unsigned char keys, unsigned char pw_mode, unsigned int timeout,char *t,unsigned int *res);
extern unsigned int ui_input_code(void);


#endif