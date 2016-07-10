/*This program is free software: you can redistribute it and/or modify
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

#ifndef DCF77_H
#define DCF77_H


extern void dcf77_init(void);
extern void dcf77_isr_part(void); //call every 1ms

#define DCF77_STATE_NONE 0
#define DCF77_STATE_FOUND_NO_SYNC 1
#define DCF77_STATE_FOUND_SYNC_1 2
#define DCF77_STATE_FOUND_SYNC_FULL 3

extern unsigned char dcf77_get_state(void);
extern unsigned char dcf77_get_signal(void);

#endif