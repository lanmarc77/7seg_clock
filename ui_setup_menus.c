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

#ifndef UI_SETUP_MENUS_C
#define UI_SETUP_MENUS_C
#include "config.h"
#include "ui_display_modes.h"
#include "ui_setup_menus.h"
#include "ui_ir.h"
#include "clock.h"
#include "7seg_func.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>



unsigned char dimm_display_mode=0;
unsigned char dimm_save_value=0;
unsigned char bright_save_value=0;
//0 if the menu was exited
//1 otherwise
unsigned int setup_dimm(void){

	switch(dimm_display_mode){
		case 0:	dimm_save_value=dimm_value;bright_save_value=bright_value;stop_stop_watch();start_stop_watch();
				I_SEG_MODE=SEG_DIM;
				I_COLON_MODE=COLON_ON;
				I_digits[0]=L_d;
				I_digits[1]=L_NOTHING;
				I_digits[2]=(25-dimm_value/10)/10;
				I_digits[3]=(25-dimm_value/10)%10;
				dimm_display_mode++;
				break;
		case 1:	if(get_stop_watch()>(60000/4)*2){//120seconds waiting
					dimm_display_mode=0;
					return 0;
				}else{
					switch(get_ir_code()){
						case IR_POWER: dimm_display_mode=0;
										dimm_value=dimm_save_value;
										bright_value=bright_save_value;
										I_SEG_MODE=SEG_BRIGHT;
										I_COLON_MODE=COLON_OFF;
										return 0;
										break;
						case IR_CH_MINUS: 	if(dimm_value<=240){
												dimm_value+=10;
											}
											stop_stop_watch();start_stop_watch();
											break;
						case IR_CH_PLUS: 	if(dimm_value>=10){
												dimm_value-=10;
											}
											stop_stop_watch();start_stop_watch();
											break;
						case IR_MUTE: 	dimm_display_mode=2;
										I_digits[0]=L_b;
										I_digits[1]=L_NOTHING;
										I_digits[2]=(25-bright_value/10)/10;
										I_digits[3]=(25-bright_value/10)%10;
										I_SEG_MODE=SEG_BRIGHT;
										stop_stop_watch();start_stop_watch();
										return 1;
										break;
					}
					I_digits[0]=L_d;
					I_digits[1]=L_NOTHING;
					I_digits[2]=(25-dimm_value/10)/10;
					I_digits[3]=(25-dimm_value/10)%10;
				}
				break;
		case 2:	if(get_stop_watch()>(60000/4)*2){//120seconds waiting
					dimm_display_mode=0;
					return 0;
				}else{
					switch(get_ir_code()){
						case IR_POWER: dimm_display_mode=0;
										dimm_value=dimm_save_value;
										bright_value=bright_save_value;
										I_SEG_MODE=SEG_BRIGHT;
										I_COLON_MODE=COLON_OFF;
										return 0;
										break;
						case IR_CH_MINUS: 	if(bright_value<=240){
												bright_value+=10;
											}
											stop_stop_watch();start_stop_watch();
											break;
						case IR_CH_PLUS: 	if(bright_value>=10){
												bright_value-=10;
											}
											stop_stop_watch();start_stop_watch();
											break;
						case IR_MUTE: 	dimm_display_mode=0;
										eeprom_write_byte ((uint8_t*)11, dimm_value);
										eeprom_write_byte ((uint8_t*)12, bright_value);
										I_SEG_MODE=SEG_BRIGHT;
										I_COLON_MODE=COLON_OFF;
										stop_stop_watch();start_stop_watch();
										return 0;
										break;
					}
					I_digits[0]=L_b;
					I_digits[1]=L_NOTHING;
					I_digits[2]=(25-bright_value/10)/10;
					I_digits[3]=(25-bright_value/10)%10;
				}
				break;


	}
	return 1;
}



unsigned char stop_display_mode=0;
unsigned char stop_beep_mode=STOP_BEEP_SHORT;
unsigned char stop_min_time=0;
unsigned char stop_trigger=0;
//0 if the menu was exited
//1 otherwise
unsigned int stop_time(void){

	switch(stop_display_mode){
		case 0:	stop_min_time=90;stop_stop_watch();start_stop_watch();
				I_digits[0]=12;
				I_digits[1]=L_NOTHING;
				I_digits[2]=stop_min_time/10;
				I_digits[3]=stop_min_time%10;
				stop_display_mode++;
				stop_trigger=0;
				break;
		case 1:	if(get_stop_watch()>(60000/4)*2){//120seconds waiting
					stop_display_mode=0;
					return 0;
				}else{
					switch(get_ir_code()){
						case IR_POWER: stop_display_mode=0;
										return 0;
										break;
						case IR_VOL_PLUS: 	stop_min_time+=1;stop_stop_watch();start_stop_watch();
											break;
						case IR_VOL_MINUS: 	stop_min_time-=1;stop_stop_watch();start_stop_watch();
											break;
						case IR_CH_PLUS: 	if(stop_min_time%5==0){
												stop_min_time+=5;
											}else{
												stop_min_time+=(5-stop_min_time%5);
											}
											stop_stop_watch();start_stop_watch();
											break;
						case IR_CH_MINUS: 	if(stop_min_time>5){
												if(stop_min_time%5==0){
													stop_min_time-=5;
												}else{
													stop_min_time-=stop_min_time%5;
												}
											}
											stop_stop_watch();start_stop_watch();
											break;
						case IR_MUTE: 	stop_display_mode=2;stop_stop_watch();start_stop_watch();
											break;
					}
					if(stop_min_time>240){
						stop_min_time=240;
					}
					if(stop_min_time<1){
						stop_min_time=1;
					}
					if(stop_min_time<100){
						I_digits[0]=12;
						I_digits[1]=L_NOTHING;
						I_digits[2]=stop_min_time/10;
						I_digits[3]=stop_min_time%10;
					}else{
						I_digits[0]=12;
						I_digits[1]=stop_min_time/100;
						I_digits[2]=(stop_min_time%100)/10;
						I_digits[3]=stop_min_time%10;
					}
				}
				break;
		case 2:	switch(get_ir_code()){
					case IR_POWER: //stop_display_mode=2;I_SEG_MODE=SEG_BRIGHT;
									stop_trigger=1;
									//return 1;
									break;
					case IR_MUTE: 	if(stop_trigger){
										stop_display_mode=0;I_SEG_MODE=SEG_BRIGHT;I_BEEPER_MODE=BEEPER_OFF;
										return 0;
									}else{//pause the clock
										pause_stop_watch();
										stop_display_mode=3;
										return 1;
									}
									stop_trigger=0;
									break;
				}

				if(get_stop_watch()>=(60000/4)){//one minute over?
					stop_min_time--;
					stop_stop_watch();start_stop_watch();
				}
				if((get_stop_watch()%250)<=125){
					I_digits[0]=12;
					if(stop_trigger%2){
						stop_trigger++;
						if(stop_trigger>5){
							stop_trigger=0;
						}
					}
				}else{
					I_digits[0]=12|0x80;
					if(stop_trigger && !(stop_trigger%2)){stop_trigger++;}
				}
				if(stop_beep_mode==STOP_BEEP_SHORT){
					if(stop_min_time==1){
						if(get_stop_watch()<(1000/4)){
							I_BEEPER_MODE=BEEPER_ON_FULL;
						}else if(get_stop_watch()>(57000/4)){
							I_BEEPER_MODE=BEEPER_ON;
						}else{
							I_BEEPER_MODE=BEEPER_OFF;
						}
					}
				}else if(stop_beep_mode==STOP_BEEP_LONG){
					if(stop_min_time==1){
						I_BEEPER_MODE=BEEPER_ON;
					}else{
						I_BEEPER_MODE=BEEPER_OFF;
					}
				}
				if(stop_min_time<=5){
					I_SEG_MODE=SEG_ZOOM;
				}else{
					I_SEG_MODE=SEG_BRIGHT;
				}
				if(stop_min_time<100){
					I_digits[1]=L_NOTHING;
					I_digits[2]=stop_min_time/10;
					I_digits[3]=stop_min_time%10;
				}else{
					I_digits[1]=stop_min_time/100;
					I_digits[2]=(stop_min_time%100)/10;
					I_digits[3]=stop_min_time%10;
				}
				if(stop_trigger){
					I_digits[3]|=0x80;
				}
				if(stop_min_time==0){
					stop_display_mode=0;
					I_SEG_MODE=SEG_BRIGHT;
					I_BEEPER_MODE=BEEPER_OFF;
					return 0;
				}
				break;

		case 3:	switch(get_ir_code()){
					case IR_POWER:
					case IR_MUTE: 	stop_display_mode=2;I_SEG_MODE=SEG_BRIGHT;unpause_stop_watch();
									break;
				}
				I_SEG_MODE=SEG_ZOOM;
				I_digits[0]=L_P;
				if(stop_min_time<100){
					I_digits[1]=L_NOTHING;
					I_digits[2]=stop_min_time/10;
					I_digits[3]=stop_min_time%10;
				}else{
					I_digits[1]=stop_min_time/100;
					I_digits[2]=(stop_min_time%100)/10;
					I_digits[3]=stop_min_time%10;
				}
				break;
	}
	return 1;
}



unsigned char t_show_mode=0;
unsigned char show_mode_display_mode=0;

//0 if the menu was exited
//1 still busy
//2 if the mode was changed
unsigned int set_show_mode(void){

	switch(show_mode_display_mode){
		case 0:	stop_stop_watch();start_stop_watch();
				show_mode_display_mode=1;
				t_show_mode=show_mode;
				break;	
		case 1:	if(get_stop_watch()*4>10000){//10seconds waiting
					show_mode_display_mode=0;
					return 0;
				}else{
					switch(get_ir_code()){
						case IR_POWER: show_mode_display_mode=0;
										return 0;
										break;
						case IR_CH_MINUS: 	if(t_show_mode<7){t_show_mode++;};stop_stop_watch();start_stop_watch();
											break;
						case IR_CH_PLUS: 	if(t_show_mode!=0){t_show_mode--;};stop_stop_watch();start_stop_watch();
											break;
						case IR_MUTE: 	show_mode_display_mode=0;show_mode=t_show_mode;stop_stop_watch();start_stop_watch();eeprom_write_byte ((uint8_t*)10, show_mode);
											return 2;
											break;
					}
					switch(t_show_mode){
						case 0:	I_digits[0]=L_NOTHING;
								I_digits[1]=L_NOTHING;
								I_digits[2]=L_C;
								I_digits[3]=L_1;
								break;
						case 1:	I_digits[0]=L_NOTHING;
								I_digits[1]=L_S;
								I_digits[2]=L_t;
								I_digits[3]=L_A;
								break;
						case 2:	I_digits[0]=L_NOTHING;
								I_digits[1]=L_NOTHING;
								I_digits[2]=L_P;
								I_digits[3]=L_A;
								break;
						case 3:	I_digits[0]=L_NOTHING;
								I_digits[1]=L_W;
								I_digits[2]=L_b;
								I_digits[3]=L_S;
								break;
						case 4:	I_digits[0]=L_NOTHING;
								I_digits[1]=L_NOTHING;
								I_digits[2]=L_t;
								I_digits[3]=L_E;
								break;
						case 5:	I_digits[0]=L_NOTHING;
								I_digits[1]=L_b;
								I_digits[2]=L_I;
								I_digits[3]=L_n;
								break;
						case 6:	I_digits[0]=L_NOTHING;
								I_digits[1]=L_NOTHING;
								I_digits[2]=L_C;
								I_digits[3]=L_2;
								break;
						case 7:	I_digits[0]=L_NOTHING;
								I_digits[1]=L_NOTHING;
								I_digits[2]=L_C;
								I_digits[3]=L_3;
								break;
						default:I_digits[0]=L__;
								I_digits[1]=L__;
								I_digits[2]=L__;
								I_digits[3]=L__;
								break;
					}
				}
			break;
			
			
		default: break;

	}
	return 1;
}


unsigned char setup_time_display_mode=0;
unsigned char st_new_day=0,st_new_mon=0,st_new_year=0,st_new_dow=0,st_new_hour=0,st_new_min=0;
//0 if the menu was exited
//1 otherwise
unsigned int setup_time(void){

	switch(setup_time_display_mode){
		case 0:	switch(get_ir_input(2,0,10000,L_d,L_A,L_Y,L_NOTHING)){//number of keys
					case 0: setup_time_display_mode=0;
									return 0;
									break;
					case 2: setup_time_display_mode=0;
									return 0;
									break;
					case 5:	if((last_ir_code>0) && (last_ir_code<=31)){
								st_new_day=last_ir_code;
								setup_time_display_mode++;
							}else{
								setup_time_display_mode=1;
							}
							break;
				}
				break;
		case 1:	switch(get_ir_input(2,0,10000,L_M,L_o,L_n,L_NOTHING)){//number of keys
					case 0: setup_time_display_mode=0;
									return 0;
									break;
					case 2: setup_time_display_mode=0;
									return 0;
									break;
					case 5:	if((last_ir_code>0) && (last_ir_code<=12)){
								st_new_mon=last_ir_code;
								setup_time_display_mode++;
							}else{
								setup_time_display_mode=1;
							}
							break;
				}
				break;
		case 2:	switch(get_ir_input(2,0,10000,L_Y,L_E,L_A,L_r)){//number of keys
					case 0: setup_time_display_mode=0;
									return 0;
									break;
					case 2: setup_time_display_mode=0;
									return 0;
									break;
					case 5:	if((last_ir_code>=0) && (last_ir_code<=99)){
								st_new_year=last_ir_code;
								setup_time_display_mode++;
							}else{
								setup_time_display_mode=1;
							}
							break;
				}
				break;
		case 3:	switch(get_ir_input(1,0,10000,L_d,L_o,L_W,L_NOTHING)){//number of keys
					case 0: setup_time_display_mode=0;
									return 0;
									break;
					case 2: setup_time_display_mode=0;
									return 0;
									break;
					case 5:	if((last_ir_code>=1) && (last_ir_code<=7)){
								st_new_dow=last_ir_code;
								setup_time_display_mode++;
							}else{
								setup_time_display_mode=1;
							}
							break;
				}
				break;
		case 4:	switch(get_ir_input(2,0,10000,L_H,L_o,L_U,L_r)){//number of keys
					case 0: setup_time_display_mode=0;
									return 0;
									break;
					case 2: setup_time_display_mode=0;
									return 0;
									break;
					case 5:	if((last_ir_code>=0) && (last_ir_code<=23)){
								st_new_hour=last_ir_code;
								setup_time_display_mode++;
							}else{
								setup_time_display_mode=1;
							}
							break;
				}
				break;
		case 5:	switch(get_ir_input(2,0,10000,L_M,L_I,L_n,L_NOTHING)){//number of keys
					case 0: setup_time_display_mode=0;
									return 0;
									break;
					case 2: setup_time_display_mode=0;
									return 0;
									break;
					case 5:	if((last_ir_code>=0) && (last_ir_code<=59)){
								st_new_min=last_ir_code;
								setup_time_display_mode++;
							}else{
								setup_time_display_mode=1;
							}
							break;
				}
				break;
		case 6:	cli();
				new_time_flag=1;
				second=0;
				hour=((st_new_hour/10)<<4|(st_new_hour%10));
				min=((st_new_min/10)<<4|(st_new_min%10));
				day=((st_new_day/10)<<4|(st_new_day%10));
				month=((st_new_mon/10)<<4|(st_new_mon%10));
				year=((st_new_year/10)<<4|(st_new_year%10));
				dow=st_new_dow;
				sei();
				setup_time_display_mode=0;
				return 0;
				break;
				

	}
	return 1;
}



unsigned char alarm_mode=ALARM_OFF;
unsigned char alarm_hour=0;
unsigned char alarm_minute=0;
unsigned char digits_save[4];


unsigned char check_alarm(void){
	switch(alarm_mode){
		case ALARM_OFF:	break;
		case ALARM_ON:	if((alarm_hour==I_hour)&&(alarm_minute==I_minute)){
							alarm_mode=ALARM_RINGING;
							digits_save[0]=I_digits[0];
							digits_save[1]=I_digits[1];
							digits_save[2]=I_digits[2];
							digits_save[3]=I_digits[3];
						}
						break;
		case ALARM_RINGING:	I_BEEPER_MODE=BEEPER_ON;
							I_SEG_MODE=SEG_ZOOM;
							I_digits[0]=L_A;
							I_digits[1]=L_L;
							I_digits[2]=L_M;
							I_digits[3]=L_NOTHING;
							if(get_ir_code()!=0xFF){
								alarm_mode=ALARM_PAUSED;
								I_BEEPER_MODE=BEEPER_OFF;
								I_digits[0]=digits_save[0];
								I_digits[1]=digits_save[1];
								I_digits[2]=digits_save[2];
								I_digits[3]=digits_save[3];
							}
							if(!((alarm_hour==I_hour)&&(alarm_minute==I_minute))){
								alarm_mode=ALARM_ON;
								I_BEEPER_MODE=BEEPER_OFF;
								I_digits[0]=digits_save[0];
								I_digits[1]=digits_save[1];
								I_digits[2]=digits_save[2];
								I_digits[3]=digits_save[3];
							}
							return 1;
							break;
		case ALARM_PAUSED:	if(!((alarm_hour==I_hour)&&(alarm_minute==I_minute))){
								alarm_mode=ALARM_ON;
							}
							break;
	}
	return 0;
}


unsigned char setup_alm_time_display_mode=0;
unsigned char st_new_alm_hour=0,st_new_alm_min=0;
//0 if the menu was exited
//1 otherwise
unsigned int setup_alm_time(void){

	switch(setup_alm_time_display_mode){
		case 0:	setup_alm_time_display_mode++;
				stop_stop_watch();start_stop_watch();
				I_digits[0]=alarm_hour/10;
				I_digits[1]=alarm_hour%10|0x80;
				I_digits[2]=alarm_minute/10;
				I_digits[3]=alarm_minute%10;
				I_COLON_MODE=COLON_ON;
				break;
		case 1:	if(get_stop_watch()*4>1000){//1seconds waiting
					setup_alm_time_display_mode++;
					stop_stop_watch();
					I_COLON_MODE=COLON_OFF;
				}
				break;

		case 2:	switch(get_ir_input(2,0,10000,L_H,L_o,L_U,L_r)){//number of keys
					case 0: setup_alm_time_display_mode=4;alarm_mode=ALARM_OFF;stop_stop_watch();start_stop_watch();
									break;
					case 2: setup_alm_time_display_mode=4;stop_stop_watch();start_stop_watch();
									break;
					case 5:	if((last_ir_code>=0) && (last_ir_code<=23)){
								st_new_alm_hour=last_ir_code;
								setup_alm_time_display_mode++;
							}else{
								setup_alm_time_display_mode=0;
							}
							break;
				}
				break;
		case 3:	switch(get_ir_input(2,0,10000,L_M,L_I,L_n,L_NOTHING)){//number of keys
					case 0: setup_alm_time_display_mode=4;alarm_mode=ALARM_OFF;stop_stop_watch();start_stop_watch();
									break;
					case 2: setup_alm_time_display_mode=4;stop_stop_watch();start_stop_watch();
									break;
					case 5:	if((last_ir_code>=0) && (last_ir_code<=59)){
								st_new_alm_min=last_ir_code;
								
								alarm_hour=st_new_alm_hour;
								alarm_minute=st_new_alm_min;
								alarm_mode=ALARM_ON;
								setup_alm_time_display_mode++;
							}else{
								setup_alm_time_display_mode=0;
							}
							break;
				}
				break;
		case 4:	setup_alm_time_display_mode++;
				stop_stop_watch();start_stop_watch();
				if(alarm_mode==ALARM_ON){
					I_digits[0]=L_o;
					I_digits[1]=L_n;
					I_digits[2]=L_NOTHING;
					I_digits[3]=L_NOTHING;
				}else{
					I_digits[0]=L_o;
					I_digits[1]=L_F;
					I_digits[2]=L_F;
					I_digits[3]=L_NOTHING;
				}
				break;
				
		case 5:	if(get_stop_watch()*4>2500){//2.5seconds waiting
					eeprom_write_byte ((uint8_t*)13, alarm_mode);
					eeprom_write_byte ((uint8_t*)14, alarm_hour);
					eeprom_write_byte ((uint8_t*)15, alarm_minute);
					setup_alm_time_display_mode=0;
					return 0;
				}
				break;

	}
	return 1;
}

/*schedule storage space layout:
0: time hour
1: time minute
2: beep mode (0=off, 1=short, 2=long, 3=mp3)
3: dow mode (0=always, 1=weekdays only)
4: later if beep mode is mp3 the number of the track e.g. 04 for 04.mp3
*/

#define MAX_SCHEDULES 20
#define SCHEDULE_OFFSET 100
unsigned char schedules[5*MAX_SCHEDULES]={0};

void load_schedules(void){
	unsigned int i=0;
	for(i=0;i<sizeof(schedules);i++){
		schedules[i]=eeprom_read_byte((uint8_t*)i+SCHEDULE_OFFSET);
	}
	for(i=0;i<MAX_SCHEDULES;i++){
		if( (schedules[i*5]>23)||(schedules[(i*5)+1]>59)||(schedules[(i*5)+2]>3)||(schedules[(i*5)+3]>1) ){
			schedules[i*5]=0;schedules[(i*5)+1]=0;schedules[(i*5)+2]=0;schedules[(i*5)+3]=0;schedules[(i*5)+4]=0;
		}
		
	}
}

void save_schedule(unsigned char sched){
	unsigned int i=0;
	for(i=sched*5;i<sched*5+5;i++){
		cli();
		eeprom_write_byte ((uint8_t*)i+SCHEDULE_OFFSET, schedules[i]);
		sei();
	}
}

unsigned char schedule_beep_mode=0;
signed char schedule_min_ring=-1;
//0 no beep/schedule in action
//1 beep/schedule in action
unsigned char check_schedule(void){
	unsigned int i=0;
	switch(schedule_beep_mode){
		case 0:	for(i=0;i<MAX_SCHEDULES;i++){
					if(schedules[(i*5)+2]!=0){
						if((schedules[(i*5)+0]==I_hour)&&(schedules[(i*5)+1]==I_minute)){
							if((schedules[(i*5)+3]==0) || ((schedules[(i*5)+3]==1)&&(I_dow<=5)) ){
								if(schedules[(i*5)+2]==1){
									schedule_beep_mode=10;//short beep
									schedule_min_ring=I_minute;
									return 1;
								}else if(schedules[(i*5)+2]==2){
									schedule_beep_mode=20;//long beep
									schedule_min_ring=I_minute;
									return 1;
								}else if(schedules[(i*5)+2]==3){//mp3 mode not supported yet
								}
							}
						}
					}
				}
				break;
		case 10: I_BEEPER_MODE=BEEPER_ON;schedule_beep_mode=11;stop_stop_watch();start_stop_watch();return 1;break;
		case 11:if(get_stop_watch()*4>3000){
					I_BEEPER_MODE=BEEPER_OFF;
					stop_stop_watch();
					schedule_beep_mode=30;//wait for next minute
				}
				return 1;
				break;

		case 20: I_BEEPER_MODE=BEEPER_ON_FULL;schedule_beep_mode=21;stop_stop_watch();start_stop_watch();return 1;break;
		case 21:if(get_stop_watch()*4>3000){
					I_BEEPER_MODE=BEEPER_OFF;
					stop_stop_watch();
					schedule_beep_mode=30;//wait for next minute
				}
				return 1;
				break;
		
		case 30: if(I_minute!=schedule_min_ring){
					schedule_beep_mode=0;
					schedule_min_ring=-1;
				 }
				 break;
		
	}
	return 0;
}

unsigned char setup_schedule_display_mode=0;
unsigned char setup_schedule_place=1;

unsigned char schedule_new_hour=0;
unsigned char schedule_new_minute=0;
unsigned char schedule_new_beep=0;
unsigned char schedule_new_dow=0;


//0 if the menu was exited
//1 otherwise
unsigned int setup_schedule(void){
	switch(setup_schedule_display_mode){
		case 0:setup_schedule_display_mode++;
				stop_stop_watch();start_stop_watch();
				setup_schedule_place=1;
				I_COLON_MODE=COLON_OFF;
				break;
		case 1:	if(get_stop_watch()*4>10000){//10seconds waiting
						setup_schedule_display_mode=0;display_update=1;
						return 0;
				}
				switch(get_ir_code()){
					case IR_POWER: 	setup_schedule_display_mode=0;display_update=1;
									return 0;
									break;
					case IR_VOL_PLUS: 	if(schedules[(setup_schedule_place-1)*5+2]!=0){
											setup_schedule_display_mode=30;
										}
										stop_stop_watch();start_stop_watch();
										break;
					case IR_CH_PLUS: 	if(setup_schedule_place>1){
											setup_schedule_place--;
										}
										stop_stop_watch();start_stop_watch();
										break;
					case IR_CH_MINUS: 	if(setup_schedule_place<MAX_SCHEDULES){
											setup_schedule_place++;
										}
										stop_stop_watch();start_stop_watch();
										break;
					case IR_MUTE: 	setup_schedule_display_mode=2;stop_stop_watch();start_stop_watch();
									schedule_new_hour=schedules[(setup_schedule_place-1)*5+0];
									schedule_new_minute=schedules[(setup_schedule_place-1)*5+1];
									schedule_new_beep=schedules[(setup_schedule_place-1)*5+2];
									if(schedule_new_beep==0) schedule_new_beep=1;
									schedule_new_dow=schedules[(setup_schedule_place-1)*5+3];
									break;
				}
				I_digits[0]=setup_schedule_place/10;
				I_digits[1]=setup_schedule_place%10;
				I_digits[2]=L_NOTHING;
				switch(schedules[(setup_schedule_place-1)*5+2]){
					case 0:I_digits[3]=12;break;
					case 1:I_digits[3]=L_S;break;
					case 2:I_digits[3]=L_L;break;
					case 3:I_digits[3]=L_M;break;
				}
				break;
		case 2:	switch(get_ir_input(2,0,10000,L_H,L_o,L_U,L_r)){//number of keys
					case 0: 	setup_schedule_display_mode=10;
								stop_stop_watch();start_stop_watch();
								break;
					case 2: 	setup_schedule_display_mode=1;
								stop_stop_watch();start_stop_watch();
								break;
					case 5:	if((last_ir_code>=0) && (last_ir_code<=23)){
								schedule_new_hour=last_ir_code;
								setup_schedule_display_mode++;
							}else{
								setup_schedule_display_mode=0;
							}
							break;
				}
				break;
		case 3:	switch(get_ir_input(2,0,10000,L_M,L_I,L_n,L_NOTHING)){//number of keys
					case 0: 	setup_schedule_display_mode=10;
								stop_stop_watch();start_stop_watch();
								break;
					case 2: 	setup_schedule_display_mode=1;
								stop_stop_watch();start_stop_watch();
								break;
					case 5:	if((last_ir_code>=0) && (last_ir_code<=59)){
								schedule_new_minute=last_ir_code;
								setup_schedule_display_mode++;
							}else{
								setup_schedule_display_mode=0;
							}
							stop_stop_watch();start_stop_watch();
							break;
				}
				break;

		case 4:	if(get_stop_watch()*4>10000){//10seconds waiting
						setup_schedule_display_mode=1;stop_stop_watch();start_stop_watch();
				}
				switch(get_ir_code()){
					case IR_POWER: 	setup_schedule_display_mode=10;
									stop_stop_watch();start_stop_watch();
									break;
					case IR_CH_PLUS: 	if(schedule_new_beep>1){
											schedule_new_beep--;
										}
										stop_stop_watch();start_stop_watch();
										break;
					case IR_CH_MINUS: 	if(schedule_new_beep<2){
											schedule_new_beep++;
										}
										stop_stop_watch();start_stop_watch();
										break;
					case IR_MUTE: 	setup_schedule_display_mode++;stop_stop_watch();start_stop_watch();
									break;
				}
				switch(schedule_new_beep){
					case 0:I_digits[0]=L_NOTHING;I_digits[1]=L_o;I_digits[2]=L_F;I_digits[3]=L_F;
							break;
					case 1:I_digits[0]=L_NOTHING;I_digits[1]=L_NOTHING;I_digits[2]=L_NOTHING;I_digits[3]=L_S;
							break;
					case 2:I_digits[0]=L_NOTHING;I_digits[1]=L_NOTHING;I_digits[2]=L_NOTHING;I_digits[3]=L_L;
							break;
					case 3:I_digits[0]=L_M;I_digits[1]=L_P;I_digits[2]=L_3;I_digits[3]=L_NOTHING;
							break;
				}
				break;

		case 5:	if(get_stop_watch()*4>10000){//10seconds waiting
						setup_schedule_display_mode=1;stop_stop_watch();start_stop_watch();
				}
				switch(get_ir_code()){
					case IR_POWER: 	setup_schedule_display_mode=10;
									stop_stop_watch();start_stop_watch();
									break;
					case IR_CH_PLUS: 	if(schedule_new_dow>0){
											schedule_new_dow--;
										}
										stop_stop_watch();start_stop_watch();
										break;
					case IR_CH_MINUS: 	if(schedule_new_dow<1){
											schedule_new_dow++;
										}
										stop_stop_watch();start_stop_watch();
										break;
					case IR_MUTE: 	setup_schedule_display_mode++;stop_stop_watch();start_stop_watch();
									break;
				}
				switch(schedule_new_dow){
					case 0:I_digits[0]=L_NOTHING;I_digits[1]=L_1;I_digits[2]=12;I_digits[3]=L_7;
							break;
					case 1:I_digits[0]=L_NOTHING;I_digits[1]=L_1;I_digits[2]=12;I_digits[3]=L_5;
							break;
				}
				break;		

		case 6: setup_schedule_display_mode=1;
				schedules[(setup_schedule_place-1)*5+0]=schedule_new_hour;
				schedules[(setup_schedule_place-1)*5+1]=schedule_new_minute;
				schedules[(setup_schedule_place-1)*5+2]=schedule_new_beep;
				schedules[(setup_schedule_place-1)*5+3]=schedule_new_dow;
				save_schedule(setup_schedule_place-1);
				stop_stop_watch();start_stop_watch();
				break;
		
		case 10: setup_schedule_display_mode=1;
				schedules[(setup_schedule_place-1)*5+2]=0;
				stop_stop_watch();start_stop_watch();
				save_schedule(setup_schedule_place-1);
				break; //disable schedule
		
		case 30:I_COLON_MODE=COLON_ON;
				I_digits[0]=schedules[(setup_schedule_place-1)*5+0]/10;
				I_digits[1]=schedules[(setup_schedule_place-1)*5+0]%10;
				I_digits[2]=schedules[(setup_schedule_place-1)*5+1]/10;
				I_digits[3]=schedules[(setup_schedule_place-1)*5+1]%10;
				if(get_stop_watch()*4>2000){
					setup_schedule_display_mode=31;
					stop_stop_watch();start_stop_watch();
				}
				break;
		case 31:I_COLON_MODE=COLON_OFF;
				switch(schedules[(setup_schedule_place-1)*5+2]){
					case 0:I_digits[0]=L_NOTHING;I_digits[1]=L_o;I_digits[2]=L_F;I_digits[3]=L_F;
							break;
					case 1:I_digits[0]=L_NOTHING;I_digits[1]=L_NOTHING;I_digits[2]=L_NOTHING;I_digits[3]=L_S;
							break;
					case 2:I_digits[0]=L_NOTHING;I_digits[1]=L_NOTHING;I_digits[2]=L_NOTHING;I_digits[3]=L_L;
							break;
					case 3:I_digits[0]=L_M;I_digits[1]=L_P;I_digits[2]=L_3;I_digits[3]=L_NOTHING;
							break;
				}
				if(get_stop_watch()*4>1000){
					setup_schedule_display_mode=32;
					stop_stop_watch();start_stop_watch();
				}
				break;
		case 32:
				switch(schedules[(setup_schedule_place-1)*5+3]){
					case 0:I_digits[0]=L_NOTHING;I_digits[1]=L_1;I_digits[2]=12;I_digits[3]=L_7;
							break;
					case 1:I_digits[0]=L_NOTHING;I_digits[1]=L_1;I_digits[2]=12;I_digits[3]=L_5;
							break;
				}
				if(get_stop_watch()*4>1000){
					setup_schedule_display_mode=1;
					stop_stop_watch();start_stop_watch();
				}
				break;
	}
	return 1;
}


volatile unsigned char main_menu_display_mode=0;
//0 if the menu was exited
//1 still busy
//2 if the show mode was changed
unsigned int main_menu_input(void){
	unsigned char c=0;
	switch(main_menu_display_mode){
		case 0:	stop_stop_watch();start_stop_watch();
				main_menu_display_mode=1;I_SEG_MODE=SEG_BRIGHT;
				break;
		case 1:	if(get_stop_watch()*4>10000){//10seconds waiting
						main_menu_display_mode=0;display_update=1;
						return 0;
				}
				switch(get_ir_code()){
					case IR_POWER: 	main_menu_display_mode=0;display_update=1;
									return 0;
									break;
					case IR_CH_PLUS: 	main_menu_display_mode=2;stop_stop_watch();start_stop_watch();
										break;
					case IR_CH_MINUS: 	if(fixed_mode){
											main_menu_display_mode=5;
										}else{
											main_menu_display_mode=10;
										}
										stop_stop_watch();start_stop_watch();
										break;
					case IR_MUTE: 	main_menu_display_mode=40;stop_stop_watch();start_stop_watch();
										break;
				}
				I_digits[0]=L_S;
				I_digits[1]=L_t;
				I_digits[2]=L_o;
				I_digits[3]=L_P;
				break;
		case 2:	if(get_stop_watch()*4>10000){//10seconds waiting
						main_menu_display_mode=0;display_update=1;
						return 0;
				}
				switch(get_ir_code()){
					case IR_POWER: 	main_menu_display_mode=0;display_update=1;
									return 0;
									break;
					case IR_CH_PLUS: 	main_menu_display_mode=3;stop_stop_watch();start_stop_watch();
										break;
					case IR_CH_MINUS: 	main_menu_display_mode=1;stop_stop_watch();start_stop_watch();
										break;
					case IR_MUTE: 		main_menu_display_mode=80;
										stop_stop_watch();start_stop_watch();
										break;
				}
				I_digits[0]=L_A;
				I_digits[1]=L_L;
				I_digits[2]=L_M;
				I_digits[3]=L_NOTHING;
				break;

		case 3:	if(get_stop_watch()*4>10000){//10seconds waiting
						main_menu_display_mode=0;display_update=1;
						return 0;
				}
				switch(get_ir_code()){
					case IR_POWER: 	main_menu_display_mode=0;display_update=1;
									return 0;
									break;
					case IR_CH_PLUS: 	main_menu_display_mode=4;stop_stop_watch();start_stop_watch();
										break;
					case IR_CH_MINUS: 	main_menu_display_mode=2;stop_stop_watch();start_stop_watch();
										break;
					case IR_MUTE: 	main_menu_display_mode=50;stop_stop_watch();start_stop_watch();
										break;
				}
				I_digits[0]=L_t;
				I_digits[1]=L_I;
				I_digits[2]=L_M;
				I_digits[3]=L_E;
				break;

		case 4:	if(get_stop_watch()*4>10000){//10seconds waiting
						main_menu_display_mode=0;display_update=1;
						return 0;
				}
				switch(get_ir_code()){
					case IR_POWER: 	main_menu_display_mode=0;display_update=1;
									return 0;
									break;
					case IR_CH_PLUS: 	main_menu_display_mode=5;stop_stop_watch();start_stop_watch();
										break;
					case IR_CH_MINUS: 	main_menu_display_mode=3;stop_stop_watch();start_stop_watch();
										break;
					case IR_MUTE: 	main_menu_display_mode=90;stop_stop_watch();start_stop_watch();
										break;
				}
				I_digits[0]=L_S;
				I_digits[1]=L_C;
				I_digits[2]=L_H;
				I_digits[3]=L_d;
				break;

		case 5:	if(get_stop_watch()*4>10000){//10seconds waiting
						main_menu_display_mode=0;display_update=1;
						return 0;
				}
				switch(get_ir_code()){
					case IR_POWER: 	main_menu_display_mode=0;display_update=1;
									return 0;
									break;
					case IR_CH_PLUS: 	stop_stop_watch();start_stop_watch();
										if(fixed_mode){
											main_menu_display_mode=1;
										}else{
											main_menu_display_mode=10;
										}
										break;
					case IR_CH_MINUS: 	main_menu_display_mode=4;stop_stop_watch();start_stop_watch();
										break;
					case IR_MUTE: 	main_menu_display_mode=70;stop_stop_watch();start_stop_watch();
										break;
				}
				I_digits[0]=L_I;
				I_digits[1]=L_L;
				I_digits[2]=L_L;
				I_digits[3]=L_U;
				break;

		case 10:	if(get_stop_watch()*4>10000){//10seconds waiting
						main_menu_display_mode=0;display_update=1;
						return 0;
				}
				switch(get_ir_code()){
					case IR_POWER: 	main_menu_display_mode=0;display_update=1;
									return 0;
									break;
					case IR_CH_PLUS: 	main_menu_display_mode=1;stop_stop_watch();start_stop_watch();
										break;
					case IR_CH_MINUS: 	main_menu_display_mode=5;stop_stop_watch();start_stop_watch();
										break;
					case IR_MUTE: 	main_menu_display_mode=60;stop_stop_watch();start_stop_watch();
										break;
				}
				I_digits[0]=L_M;
				I_digits[1]=L_o;
				I_digits[2]=L_d;
				I_digits[3]=L_E;
				break;

		case 40:	if(stop_time()==0){
						main_menu_display_mode=1;stop_stop_watch();start_stop_watch();
					}
					break;
		case 50:	if(setup_time()==0){
						main_menu_display_mode=3;stop_stop_watch();start_stop_watch();
					}
					break;
		case 60:	c=set_show_mode();
					if(c==0){
						main_menu_display_mode=10;stop_stop_watch();start_stop_watch();
					}else if(c==2){
						main_menu_display_mode=0;stop_stop_watch();start_stop_watch();display_update=1;
						return 2;
					}
					break;
		case 70:	if(setup_dimm()==0){
						main_menu_display_mode=5;stop_stop_watch();start_stop_watch();
					}
					break;
		case 80:	if(setup_alm_time()==0){
						main_menu_display_mode=2;stop_stop_watch();start_stop_watch();
					}
					break;
		case 90:	if(setup_schedule()==0){
						main_menu_display_mode=4;stop_stop_watch();start_stop_watch();
					}
					break;

		default: main_menu_display_mode=0;display_update=1;return 0;break;
	}
	return 1;
}










#endif