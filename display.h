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

#ifndef DISPLAY_H
#define DISPLAY_H

#include "display_7seg.h"


#define display_init() display_7seg_init()
#define display_ISR() display_7seg_ISR()
#define display_set_anim_mode(a) display_7seg_anim_mode(a)
#define display_set_polarity(a) display_7seg_set_polarity(a)
#define display_set_mode(a) display_7seg_set_mode(a)

#define display_anim_on() display_7seg_anim_on()
#define display_set_anim_mode(a) display_7seg_set_anim_mode(a)
#define display_get_anim_mode() display_7seg_get_anim_mode()
/*
String with the following format:
hxhxmxmx[sxsxDxDxMxMxYxYxYxYx]
x=a identifier specific for the display e.g. if a . should be shown

*/
#define display_set_time(a) display_7seg_set_time(a)
#define display_set_text(a) display_7seg_set_text(a)


#define display_set_bright_level(a) display_7seg_set_bright_level(a)
#define display_set_dark_level(a) display_7seg_set_dark_level(a)
#define display_get_bright_level() display_7seg_get_bright_level()
#define display_get_dark_level() display_7seg_get_dark_level()

#endif
