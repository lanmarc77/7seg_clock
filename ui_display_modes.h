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

#ifndef UI_DISPLAY_MODES_H
#define UI_DISPLAY_MODES_H


extern volatile unsigned char show_mode;
extern volatile unsigned char display_update;
extern unsigned char fixed_mode;

extern void TA_default(void);
extern void WBS_default(void);
extern void bin_default(void);
extern void TE_default(void);
extern void c1_default(void);
extern void C2_default(void);
extern void C3_default(void);
extern void simple_default(void);
extern unsigned char version_default(void);


#endif
