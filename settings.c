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


#ifndef SETTINGS_C
#define SETTINGS_C
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>


void settings_save(unsigned int addr,unsigned char value){
	eeprom_write_byte ((uint8_t*)addr, value);
}


unsigned char settings_get(unsigned int addr){
	return eeprom_read_byte((uint8_t*)addr);
}


#endif
