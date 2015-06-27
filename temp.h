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

#ifndef TEMP_H
#define TEMP_H


extern void SE95_check_i2c_state_machine(void);
extern void temp_init(void);

extern volatile unsigned char SE95_detected;
extern volatile signed int SE95_temp;
extern volatile signed int SE95_temp_frac;
extern volatile signed char I2C_temp;
extern volatile unsigned char I2C_temp_frac;
extern volatile unsigned char SE95_state;
#define SE95_get_temp 1
extern volatile unsigned char SE95_cmd;




#endif