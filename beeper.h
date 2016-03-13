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

#ifndef BEEPER_H
#define BEEPER_H

extern void beeper_init(void);
extern void beeper_ISR(void);
extern void beeper_set_mode(unsigned char mode);

#define BEEPER_ON 0
#define BEEPER_OFF 1
#define BEEPER_ON_FULL 2

#endif