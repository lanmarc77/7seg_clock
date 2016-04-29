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

extern unsigned char ui_display_modes_get_mode(void);
extern void ui_display_modes_set_mode(unsigned char c);
extern void ui_display_modes_set_fixed_mode(unsigned char c);
extern unsigned char ui_display_modes_get_fixed_mode(void);
#define UI_DISPLAY_MODES_DOT_MODE_OFF 0
#define UI_DISPLAY_MODES_DOT_MODE_DOT 1
#define UI_DISPLAY_MODES_DOT_MODE_DOT_DCF 2
#define UI_DISPLAY_MODES_DOT_MODE_COLON 3
#define UI_DISPLAY_MODES_DOT_MODE_COLON_DCF 4
extern void ui_display_modes_set_dot_mode(unsigned char c);
extern unsigned char ui_display_modes_get_dot_mode(void);


extern void ui_display_modes_TA(void);
extern void ui_display_modes_WBS(void);
extern void ui_display_modes_bin(void);
extern void ui_display_modes_TE(void);
extern void ui_display_modes_C1(void);
extern void ui_display_modes_C2(void);
extern void ui_display_modes_C3(void);
extern void ui_display_modes_BHT(void);
extern void ui_display_modes_TFV(void);
extern void ui_display_modes_simple(void);
extern unsigned char ui_display_modes_version(void);


#endif
