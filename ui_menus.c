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

#ifndef UI_MENUS_C
#define UI_MENUS_C
#include "config.h"
#include "ui_display_modes.h"
#include "ui_menus.h"
#include "ui_input.h"
#include "clock.h"
#include "display.h"
#include "beeper.h"
#include "i2c_modules.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include "settings.h"


signed int ui_menues_code=-1;
signed int ui_menues_get_code(void){
	return ui_menues_code;
}

void ui_menues_set_code(signed int code){
	if(code==0){
		code=-1;
	}
	ui_menues_code=code;
}

unsigned char cont_mode=UI_MENUES_CONT_MODE_ON;

void ui_menues_set_cont_mode(unsigned char m){
	if(m>UI_MENUES_CONT_MODE_OFF){
		m=UI_MENUES_CONT_MODE_OFF;
	}
	cont_mode=m;
}
unsigned char ui_menues_get_cont_mode(void){
	return cont_mode;
}


unsigned char setup_code_state=0;
signed  int setup_code_code_1=0;
signed  int setup_code_code_2=0;
unsigned int setup_code(void){
	char c[4];c[0]=' ';c[1]=':';c[2]=' ';c[3]=' ';
	switch(setup_code_state){
		case 0: setup_code_code_1=ui_input_code();
				if(setup_code_code_1==-1){//code insert canceled
					setup_code_state=0;
					return 0;
				}
				if(setup_code_code_1==-2){//code still entered
				}else if(setup_code_code_1>=0){//code entered
					setup_code_state=1;
				}
				break;
		case 1: setup_code_code_2=ui_input_code();
				if(setup_code_code_2==-1){//code insert canceled
					setup_code_state=0;
					return 0;
				}
				if(setup_code_code_2==-2){//code still entered
				}else if(setup_code_code_2>=0){//code entered
					if(setup_code_code_1==setup_code_code_2){
						if(setup_code_code_1==0){
							setup_code_code_1=-1;
						}
						ui_menues_set_code(setup_code_code_1);
						settings_save(SETTINGS_UI_MENUES_CODE_32,ui_menues_get_code()>>8);
						settings_save(SETTINGS_UI_MENUES_CODE_10,ui_menues_get_code()&0xFF);
						if(ui_menues_get_code()<0){
							setup_code_state=3;
						}else{
							setup_code_state=4;
						}
						clock_stop_stop_watch();clock_start_stop_watch();
					}else{
						setup_code_state=2;clock_stop_stop_watch();clock_start_stop_watch();
					}
				}
				break;
		case 2:if(clock_get_stop_watch()*4>2000){//2seconds waiting
					setup_code_state=0;
					return 0;
				}
				display_set_text("Err ");
				break;
		case 3:if(clock_get_stop_watch()*4>2000){//2seconds waiting
					setup_code_state=0;
					return 0;
				}
				display_set_text("off ");
				break;
		case 4:if(clock_get_stop_watch()*4>2000){//2seconds waiting
					setup_code_state=0;
					return 0;
				}
				display_set_text("ok  ");
				break;
	}
	return 1;
}

unsigned char dimm_display_mode=0;
unsigned char dimm_save_value=0;
unsigned char bright_save_value=0;
unsigned char brdimm_temp=0;

//0 if the menu was exited
//1 otherwise
unsigned int setup_dimm(void){
	char c[8];c[1]=' ';c[3]=':';c[5]=' ';c[7]=' ';
	switch(dimm_display_mode){
		case 0:	dimm_save_value=display_get_dark_level();bright_save_value=display_get_bright_level();clock_stop_stop_watch();clock_start_stop_watch();
				display_set_mode(DISPLAY_7SEG_DIM);
				brdimm_temp=250-display_get_dark_level();
				dimm_display_mode++;
				break;
		case 1:	switch(ui_input_number(&brdimm_temp,0,250,(30000/4))){
						case 3:
						case 0:		dimm_display_mode=0;
									display_set_dark_level(dimm_save_value);
									display_set_bright_level(bright_save_value);
									display_set_mode(DISPLAY_7SEG_BRIGHT);
									return 0;
									break;
						case 4: 	dimm_display_mode=2;
									brdimm_temp=250-display_get_bright_level();
									display_set_mode(DISPLAY_7SEG_BRIGHT);
									return 1;
									break;
					
					}
					display_set_dark_level(250-brdimm_temp);
					if(brdimm_temp>=100){
						c[0]='d';
						c[2]=((brdimm_temp)/100)+48;
						c[4]=((brdimm_temp)%100)/10+48;
						c[6]=(brdimm_temp)%10+48;
					}else{
						c[0]='d';
						c[2]=' ';
						c[4]=(brdimm_temp)/10+48;
						c[6]=(brdimm_temp)%10+48;
					}
					display_set_time(&c[0]);
					break;
		case 2:	switch(ui_input_number(&brdimm_temp,0,250,(30000/4))){
						case 3:
						case 0:		dimm_display_mode=0;
									display_set_dark_level(dimm_save_value);
									display_set_bright_level(bright_save_value);
									display_set_mode(DISPLAY_7SEG_BRIGHT);
									return 0;
									break;
						case 4: 	dimm_display_mode=0;
									settings_save(SETTINGS_DARK_BRIGHTNESS, display_get_dark_level());
									settings_save(SETTINGS_LIGHT_BRIGHTNESS, display_get_bright_level());
									display_set_mode(DISPLAY_7SEG_BRIGHT);
									return 0;
									break;
					
					}
					display_set_bright_level(250-brdimm_temp);
					if(brdimm_temp>=100){
						c[0]='b';
						c[2]=((brdimm_temp)/100)+48;
						c[4]=((brdimm_temp)%100)/10+48;
						c[6]=(brdimm_temp)%10+48;
					}else{
						c[0]='b';
						c[2]=L' ';
						c[4]=(brdimm_temp)/10+48;
						c[6]=(brdimm_temp)%10+48;
					}					
					display_set_time(&c[0]);
					break;
	}
	return 1;
}



unsigned char stop_display_mode=0;
unsigned char stop_beep_mode=UI_MENUES_STOP_BEEP_SHORT;
unsigned char stop_min_time=0;
unsigned char stop_trigger=0;

void ui_menues_set_stop_beep_mode(unsigned char m){
	stop_beep_mode=m;
}


//0 if the menu was exited
//1 otherwise
unsigned int stop_time(void){
	char c[8];

	switch(stop_display_mode){
		case 0:	stop_min_time=90;clock_stop_stop_watch();clock_start_stop_watch();
				c[0]='-';
				c[1]=' ';
				c[2]=' ';
				c[3]=' ';
				c[4]=stop_min_time/10+48;
				c[5]=' ';
				c[6]=stop_min_time%10+48;
				c[7]=' ';
				display_set_time(&c[0]);
				stop_display_mode++;
				stop_trigger=0;
				break;
		case 1:		switch(ui_input_number(&stop_min_time,1,240,2*(60000/4))){
						case 0:		stop_display_mode=0;
									return 0;
									break;
						case 3: 	stop_display_mode=0;
									return 0;
									break;
						case 4:		stop_display_mode=2;
									break;
					
					}
					if(stop_min_time<100){
						c[0]='-';
						c[1]=' ';
						c[2]=' ';
						c[3]=' ';
						c[4]=stop_min_time/10+48;
						c[5]=' ';
						c[6]=stop_min_time%10+48;
						c[7]=' ';
					}else{
						c[0]='-';
						c[1]=' ';
						c[2]=stop_min_time/100+48;
						c[3]=' ';
						c[4]=(stop_min_time%100)/10+48;
						c[5]=' ';
						c[6]=stop_min_time%10+48;
						c[7]=' ';
					}
					display_set_time(&c[0]);
				break;
		case 2:	switch(ui_input_get_key()){
					case UI_INPUT_KEY_BACK: //stop_display_mode=2;
									stop_trigger=1;
									//return 1;
									break;
					case UI_INPUT_KEY_OK: 	if(stop_trigger){
										stop_display_mode=0;display_set_mode(DISPLAY_7SEG_BRIGHT);beeper_set_mode(BEEPER_OFF);
										return 0;
									}else{//pause the clock
										clock_pause_stop_watch();
										stop_display_mode=3;
										return 1;
									}
									stop_trigger=0;
									break;
				}

				if(clock_get_stop_watch()>=(60000/4)){//one minute over?
					stop_min_time--;
					clock_stop_stop_watch();clock_start_stop_watch();
				}
				if((clock_get_stop_watch()%250)<=125){
					c[1]=' ';
					if(stop_trigger%2){
						stop_trigger++;
						if(stop_trigger>5){
							stop_trigger=0;
						}
					}
				}else{
					c[1]='.';
					if(stop_trigger && !(stop_trigger%2)){stop_trigger++;}
				}
				if(stop_beep_mode==UI_MENUES_STOP_BEEP_SHORT){
					if(stop_min_time==1){
						if(clock_get_stop_watch()<(1000/4)){
							beeper_set_mode(BEEPER_ON_FULL);
						}else if(clock_get_stop_watch()>(57000/4)){
							beeper_set_mode(BEEPER_ON);
						}else{
							beeper_set_mode(BEEPER_OFF);
						}
					}
				}else if(stop_beep_mode==UI_MENUES_STOP_BEEP_LONG){
					if(stop_min_time==1){
						beeper_set_mode(BEEPER_ON);
					}else{
						beeper_set_mode(BEEPER_OFF);
					}
				}
				if(stop_min_time<=5){
					display_set_mode(DISPLAY_7SEG_ZOOM);
				}else{
					display_set_mode(DISPLAY_7SEG_BRIGHT);
				}
				if(stop_min_time<100){
					c[0]='-';
					//c[1]=' ';
					c[2]=' ';
					c[3]=' ';
					c[4]=stop_min_time/10+48;
					c[5]=' ';
					c[6]=stop_min_time%10+48;
					c[7]=' ';
				}else{
					c[0]='-';
					//c[1]=' ';
					c[2]=stop_min_time/100+48;
					c[3]=' ';
					c[4]=(stop_min_time%100)/10+48;
					c[5]=' ';
					c[6]=stop_min_time%10+48;
					c[7]=' ';
				}				

				if(stop_trigger){
					c[7]='.';
				}
				if(stop_min_time==0){
					stop_display_mode=0;
					display_set_mode(DISPLAY_7SEG_BRIGHT);
					beeper_set_mode(BEEPER_OFF);
					return 0;
				}
				display_set_time(&c[0]);
				break;

		case 3:	switch(ui_input_get_key()){
					case UI_INPUT_KEY_BACK:
					case UI_INPUT_KEY_OK: 	stop_display_mode=2;display_set_mode(DISPLAY_7SEG_BRIGHT);clock_unpause_stop_watch();
									break;
				}
				display_set_mode(DISPLAY_7SEG_ZOOM);
				if(stop_min_time<100){
					c[0]='P';
					c[1]=' ';
					c[2]=' ';
					c[3]=' ';
					c[4]=stop_min_time/10+48;
					c[5]=' ';
					c[6]=stop_min_time%10+48;
					c[7]=' ';
				}else{
					c[0]='P';
					c[1]=' ';
					c[2]=stop_min_time/100+48;
					c[3]=' ';
					c[4]=(stop_min_time%100)/10+48;
					c[5]=' ';
					c[6]=stop_min_time%10+48;
					c[7]=' ';
				}
				display_set_time(&c[0]);
				break;
	}
	return 1;
}


unsigned char t_anim_mode=0;
unsigned char t_show_mode=0;
unsigned char show_mode_display_mode=0;

//0 if the menu was exited
//1 still busy
//2 if the mode was changed
unsigned int set_show_mode(void){
	switch(show_mode_display_mode){
		case 0:	clock_stop_stop_watch();clock_start_stop_watch();
				show_mode_display_mode=1;
				t_show_mode=ui_display_modes_get_mode();
				break;	
		case 1:	if(clock_get_stop_watch()*4>10000){//10seconds waiting
					show_mode_display_mode=0;
					return 0;
				}else{
					switch(ui_input_get_key()){
						case UI_INPUT_KEY_BACK: show_mode_display_mode=0;
										return 0;
										break;
						case UI_INPUT_KEY_DOWN: 	if(t_show_mode<9){t_show_mode++;};clock_stop_stop_watch();clock_start_stop_watch();
											break;
						case UI_INPUT_KEY_UP: 	if(t_show_mode!=0){t_show_mode--;};clock_stop_stop_watch();clock_start_stop_watch();
											break;
						case UI_INPUT_KEY_OK: 	show_mode_display_mode=0;
											show_mode_display_mode=2;
											clock_stop_stop_watch();clock_start_stop_watch();
											break;
					}
					switch(t_show_mode){
						case 0:	display_set_text("  C1");
								break;
						case 1:	display_set_text(" STA");
								break;
						case 2:	display_set_text("  PA");
								break;
						case 3:	display_set_text(" WBS");
								break;
						case 4:	display_set_text("  TE");
								break;
						case 5:	display_set_text(" BIN");
								break;
						case 6:	display_set_text("  C2");
								break;
						case 7:	display_set_text("  C3");
								break;
						case 8:	display_set_text(" BHT");
								break;
						case 9:	display_set_text(" TFV");
								break;
						default:display_set_text("____");
								break;
					}
				}
			break;
		case 2:	if(clock_get_stop_watch()*4>2000){//2seconds waiting
					clock_stop_stop_watch();clock_start_stop_watch();
					show_mode_display_mode=3;
					t_anim_mode=display_get_anim_mode();
				}
				display_set_text("ANIM ");
				break;

		case 3:	if(clock_get_stop_watch()*4>10000){//10seconds waiting
					show_mode_display_mode=0;
					return 0;
				}else{

					switch(ui_input_get_key()){
						case UI_INPUT_KEY_BACK: show_mode_display_mode=0;
										return 0;
										break;
						case UI_INPUT_KEY_DOWN:	if(t_anim_mode<DISPLAY_7SEG_ANIM_ODO){t_anim_mode++;};clock_stop_stop_watch();clock_start_stop_watch();
											break;
						case UI_INPUT_KEY_UP: 	if(t_anim_mode!=DISPLAY_7SEG_ANIM_OFF){t_anim_mode--;};clock_stop_stop_watch();clock_start_stop_watch();
											break;
						case UI_INPUT_KEY_OK: 	show_mode_display_mode=0;
											ui_display_modes_set_mode(t_show_mode);
											settings_save(SETTINGS_DISPLAY_MODE, ui_display_modes_get_mode());
											display_set_anim_mode(t_anim_mode);
											settings_save(SETTINGS_ANIM_MODE, display_get_anim_mode());
											show_mode_display_mode=0;
											clock_stop_stop_watch();clock_start_stop_watch();
											return 2;
											break;
					}
					switch(t_anim_mode){
						case DISPLAY_7SEG_ANIM_OFF:display_set_text(" off");break;
						case DISPLAY_7SEG_ANIM_ODO:display_set_text("  on");break;
					}
				}
				break;
			
		default: break;

	}
	return 1;
}


unsigned char t_dst_mode=0;
unsigned char setup_time_display_mode=0;
unsigned char st_new_day=0,st_new_mon=0,st_new_year=0,st_new_dow=0,st_new_hour=0,st_new_min=0;
//0 if the menu was exited
//1 otherwise
unsigned int setup_time(void){
	unsigned int code;
	switch(setup_time_display_mode){
		case 0:	switch(ui_input_get_digits(2,0,10000,"Day ",&code)){//number of keys
					case 0: setup_time_display_mode=0;
									return 0;
									break;
					case 2: setup_time_display_mode=0;
									return 0;
									break;
					case 5:	if((code>0) && (code<=31)){
								st_new_day=code;
								setup_time_display_mode++;
							}else{
								setup_time_display_mode=0;
							}
							break;
				}
				break;
		case 1:	switch(ui_input_get_digits(2,0,10000,"Mon ",&code)){//number of keys
					case 0: setup_time_display_mode=0;
									return 0;
									break;
					case 2: setup_time_display_mode=0;
									return 0;
									break;
					case 5:	if((code>0) && (code<=12)){
								st_new_mon=code;
								setup_time_display_mode++;
							}else{
								setup_time_display_mode=1;
							}
							break;
				}
				break;
		case 2:	switch(ui_input_get_digits(2,0,10000,"Year",&code)){//number of keys
					case 0: setup_time_display_mode=0;
									return 0;
									break;
					case 2: setup_time_display_mode=0;
									return 0;
									break;
					case 5:	if((code>=0) && (code<=99)){
								st_new_year=code;
								setup_time_display_mode++;
							}else{
								setup_time_display_mode=1;
							}
							break;
				}
				break;
		case 3:	switch(ui_input_get_digits(1,0,10000,"dow ",&code)){//number of keys
					case 0: setup_time_display_mode=0;
									return 0;
									break;
					case 2: setup_time_display_mode=0;
									return 0;
									break;
					case 5:	if((code>=1) && (code<=7)){
								st_new_dow=code;
								setup_time_display_mode++;
							}else{
								setup_time_display_mode=1;
							}
							break;
				}
				break;
		case 4:	switch(ui_input_get_digits(2,0,10000,"Hour",&code)){//number of keys
					case 0: setup_time_display_mode=0;
									return 0;
									break;
					case 2: setup_time_display_mode=0;
									return 0;
									break;
					case 5:	if((code>=0) && (code<=23)){
								st_new_hour=code;
								setup_time_display_mode++;
							}else{
								setup_time_display_mode=1;
							}
							break;
				}
				break;
		case 5:	switch(ui_input_get_digits(2,0,10000,"Min ",&code)){//number of keys
					case 0: setup_time_display_mode=0;
									return 0;
									break;
					case 2: setup_time_display_mode=0;
									return 0;
									break;
					case 5:	if((code>=0) && (code<=59)){
								st_new_min=code;
								setup_time_display_mode++;
								clock_stop_stop_watch();clock_start_stop_watch();
							}else{
								setup_time_display_mode=1;
							}
							break;
				}
				break;

		case 6:	if(clock_get_stop_watch()*4>2000){//2seconds waiting
					clock_stop_stop_watch();clock_start_stop_watch();
					setup_time_display_mode++;
					t_dst_mode=clock_get_dst_mode();
				}
				display_set_text("DST ");
				break;

		case 7:	if(clock_get_stop_watch()*4>10000){//10seconds waiting
					setup_time_display_mode=0;
					return 0;
				}else{

					switch(ui_input_get_key()){
						case UI_INPUT_KEY_BACK: setup_time_display_mode=0;
										return 0;
										break;
						case UI_INPUT_KEY_DOWN:	if(t_dst_mode<CLOCK_DST_MODE_US){t_dst_mode++;};clock_stop_stop_watch();clock_start_stop_watch();
											break;
						case UI_INPUT_KEY_UP: 	if(t_dst_mode!=CLOCK_DST_MODE_OFF){t_dst_mode--;};clock_stop_stop_watch();clock_start_stop_watch();
											break;
						case UI_INPUT_KEY_OK: 	setup_time_display_mode++;
											clock_stop_stop_watch();clock_start_stop_watch();
											break;
					}
					switch(t_dst_mode){
						case CLOCK_DST_MODE_OFF:display_set_text(" off");break;
						case CLOCK_DST_MODE_EU:display_set_text("  EU");break;
						case CLOCK_DST_MODE_US:display_set_text("  US");break;
					}
				}
				break;
		case 8:	clock_set_dst_mode(t_dst_mode);
				settings_save(SETTINGS_DST_MODE, clock_get_dst_mode());
				clock_set_time(st_new_min,st_new_hour,0,st_new_day,st_new_mon,st_new_year,st_new_dow,CLOCK_UPDATE_SOURCE_MANUAL);
				setup_time_display_mode=0;
				return 0;
				break;
				

	}
	return 1;
}


#define UI_MENUES_ALARM_RINGING 4
#define UI_MENUES_ALARM_PAUSED 5

unsigned char alarm_mode=UI_MENUES_ALARM_OFF;
unsigned char save_alarm_mode=UI_MENUES_ALARM_OFF;
unsigned char alarm_hour=0;
unsigned char alarm_minute=0;
unsigned char alarm_track=1;
unsigned char digits_save[4];

void ui_menues_set_alarm_mode(unsigned char m){
	if(m>UI_MENUES_ALARM_ON_MP3){m=UI_MENUES_ALARM_OFF;}
	alarm_mode=m;
}

void ui_menues_set_alarm_hour(unsigned char m){
	if(m>23){m=0;}
	alarm_hour=m;
}
void ui_menues_set_alarm_minute(unsigned char m){
	if(m>59){m=0;}
	alarm_minute=m;
}
void ui_menues_set_alarm_mp3_track(unsigned char m){
	if(m>99){m=0;}
	alarm_track=m;
}

unsigned char ui_menues_check_alarm(void){
	unsigned char min,hour,second,day,month,year,dow;
	clock_get_time(&min,&hour,&second,&day,&month,&year,&dow);
	switch(alarm_mode){
		case UI_MENUES_ALARM_OFF:	break;
		case UI_MENUES_ALARM_ON:
		case UI_MENUES_ALARM_ON_MP3:
		case UI_MENUES_ALARM_ON_RADIO:	if((alarm_hour==hour)&&(alarm_minute==min)){
									save_alarm_mode=alarm_mode;
									alarm_mode=UI_MENUES_ALARM_RINGING;
									if((I2C_MP3_detected)&&(save_alarm_mode==UI_MENUES_ALARM_ON_MP3)){
										display_set_mode(DISPLAY_7SEG_ZOOM);
										I2C_MP3_playAlarm(alarm_track|0x80);
									}else if((I2C_RADIO_detected)&&(save_alarm_mode==UI_MENUES_ALARM_ON_RADIO)){
									}else{
										beeper_set_mode(BEEPER_ON);
										display_set_mode(DISPLAY_7SEG_ZOOM);
									}
									return 1;
								}
								break;
		case UI_MENUES_ALARM_RINGING:	display_set_text("ALM ");
							if(ui_input_get_key()!=UI_INPUT_KEY_NONE){
								alarm_mode=UI_MENUES_ALARM_PAUSED;
								beeper_set_mode(BEEPER_OFF);
								if((I2C_MP3_detected)&&(save_alarm_mode==UI_MENUES_ALARM_ON_MP3)){
									I2C_MP3_stopPlaying();
								}
							}
							if(!((alarm_hour==hour)&&(alarm_minute==min))){
								alarm_mode=save_alarm_mode;
								beeper_set_mode(BEEPER_OFF);
								if((I2C_MP3_detected)&&(save_alarm_mode==UI_MENUES_ALARM_ON_MP3)){
									I2C_MP3_stopPlaying();
								}
							}
							return 1;
							break;
		case UI_MENUES_ALARM_PAUSED:	if(!((alarm_hour==hour)&&(alarm_minute==min))){
								alarm_mode=save_alarm_mode;
							}
							break;
	}
	return 0;
}


unsigned char setup_alm_time_display_mode=0;
unsigned char st_new_alm_hour=0,st_new_alm_min=0,st_new_alm_mode=0,st_new_alm_track=1;
//0 if the menu was exited
//1 otherwise
unsigned int setup_alm_time(void){
	char c[8];
	unsigned int code;
	switch(setup_alm_time_display_mode){
		case 0:	setup_alm_time_display_mode++;
				clock_stop_stop_watch();clock_start_stop_watch();
				c[0]=alarm_hour/10+48;
				c[1]=' ';
				c[2]=alarm_hour%10+48;
				c[3]='|';
				c[4]=alarm_minute/10+48;
				c[5]=' ';
				c[6]=alarm_minute%10+48;
				c[7]=' ';
				display_set_time(&c[0]);
				break;
		case 1:	if(clock_get_stop_watch()*4>1000){//1seconds waiting
					setup_alm_time_display_mode++;
					clock_stop_stop_watch();
				}
				break;
		case 2:	switch(ui_input_get_digits(2,0,10000,"Hour",&code)){//number of keys
					case 0: setup_alm_time_display_mode=6;alarm_mode=UI_MENUES_ALARM_OFF;clock_stop_stop_watch();clock_start_stop_watch();
									break;
					case 2: setup_alm_time_display_mode=6;clock_stop_stop_watch();clock_start_stop_watch();
									break;
					case 5:	if((code>=0) && (code<=23)){
								st_new_alm_hour=code;
								setup_alm_time_display_mode++;
							}else{
								setup_alm_time_display_mode=0;
							}
							break;
				}
				break;
		case 3:	switch(ui_input_get_digits(2,0,10000,"Min ",&code)){//number of keys
					case 0: setup_alm_time_display_mode=6;alarm_mode=UI_MENUES_ALARM_OFF;clock_stop_stop_watch();clock_start_stop_watch();
									break;
					case 2: setup_alm_time_display_mode=6;clock_stop_stop_watch();clock_start_stop_watch();
									break;
					case 5:	if((code>=0) && (code<=59)){
								st_new_alm_min=code;
								
								alarm_hour=st_new_alm_hour;
								alarm_minute=st_new_alm_min;
								st_new_alm_track=alarm_track;if(st_new_alm_track==0) st_new_alm_track=1;
								if(alarm_mode==UI_MENUES_ALARM_OFF){
									st_new_alm_mode=UI_MENUES_ALARM_ON;
								}else{
									st_new_alm_mode=alarm_mode;
								}
								setup_alm_time_display_mode++;
							}else{
								setup_alm_time_display_mode=0;
							}
							break;
				}
				break;
		case 4:	if(clock_get_stop_watch()*4>10000){//10seconds waiting
					setup_alm_time_display_mode=6;clock_stop_stop_watch();clock_start_stop_watch();
				}
				if((!I2C_MP3_detected)&&(!I2C_RADIO_detected)){
					setup_alm_time_display_mode=6;clock_stop_stop_watch();clock_start_stop_watch();
					break;
				}
				switch(ui_input_get_key()){
					case UI_INPUT_KEY_BACK: 	setup_alm_time_display_mode=6;alarm_mode=UI_MENUES_ALARM_OFF;
									clock_stop_stop_watch();clock_start_stop_watch();
									break;
					case UI_INPUT_KEY_UP:  	switch(st_new_alm_mode){
											case UI_MENUES_ALARM_OFF:	st_new_alm_mode=UI_MENUES_ALARM_ON;
															break;
											case UI_MENUES_ALARM_ON: if(I2C_RADIO_detected){
																st_new_alm_mode=UI_MENUES_ALARM_ON_RADIO;
															}else if(I2C_MP3_detected){
																st_new_alm_mode=UI_MENUES_ALARM_ON_MP3;
															}else{
																st_new_alm_mode=UI_MENUES_ALARM_ON;
															}
															break;
											case UI_MENUES_ALARM_ON_MP3:	st_new_alm_mode=UI_MENUES_ALARM_ON;
																break;
											case UI_MENUES_ALARM_ON_RADIO: 	if(I2C_MP3_detected){
																		st_new_alm_mode=UI_MENUES_ALARM_ON_MP3;
																	}else{
																		st_new_alm_mode=UI_MENUES_ALARM_ON;
																	}
																break;
										}
										clock_stop_stop_watch();clock_start_stop_watch();
										break;
					case UI_INPUT_KEY_DOWN: 	switch(st_new_alm_mode){
											case UI_MENUES_ALARM_OFF:	st_new_alm_mode=UI_MENUES_ALARM_ON;
															break;
											case UI_MENUES_ALARM_ON:	if(I2C_MP3_detected){
																st_new_alm_mode=UI_MENUES_ALARM_ON_MP3;
															}else if(I2C_RADIO_detected){
																st_new_alm_mode=UI_MENUES_ALARM_ON_RADIO;
															}else{
																st_new_alm_mode=UI_MENUES_ALARM_ON;
															}
															break;
											case UI_MENUES_ALARM_ON_MP3:	if(I2C_RADIO_detected){
																	st_new_alm_mode=UI_MENUES_ALARM_ON_RADIO;
																}else{
																	st_new_alm_mode=UI_MENUES_ALARM_ON;
																}
																break;
											case UI_MENUES_ALARM_ON_RADIO: st_new_alm_mode=UI_MENUES_ALARM_ON;
																break;
										}
										clock_stop_stop_watch();clock_start_stop_watch();
										break;
					case UI_INPUT_KEY_OK: 	
									if(st_new_alm_mode==UI_MENUES_ALARM_ON_MP3){
										setup_alm_time_display_mode=5;
									}else if(st_new_alm_mode==UI_MENUES_ALARM_ON_RADIO){
										setup_alm_time_display_mode=5;
									}else{
										alarm_mode=st_new_alm_mode;
										setup_alm_time_display_mode=6;
									}
									clock_stop_stop_watch();clock_start_stop_watch();
									break;
				}
				switch(st_new_alm_mode){
					case UI_MENUES_ALARM_ON:display_set_text("Beep");
							break;
					case UI_MENUES_ALARM_ON_MP3:display_set_text(" MP3");
							break;
					case UI_MENUES_ALARM_ON_RADIO:display_set_text("  Ra");
							break;
				}
				break;

		case 5:	switch(ui_input_number(&st_new_alm_track,0,99,30000/4)){
					case 3:
					case 0:	if(I2C_MP3_detected){
								I2C_MP3_stopPlaying();
							}
							setup_alm_time_display_mode=6;alarm_mode=UI_MENUES_ALARM_OFF;
							break;
					case 2:if(I2C_MP3_detected){
								I2C_MP3_playAlarm(st_new_alm_track|0x80);
							}
							break;
					case 4:	if(I2C_MP3_detected){
								I2C_MP3_stopPlaying();
							}
							alarm_track=st_new_alm_track;
							alarm_mode=st_new_alm_mode;
							setup_alm_time_display_mode=6;
							break;
				
				}
				c[0]=' ';
				c[1]=' ';
				c[2]=st_new_alm_track/10+48;
				c[3]=st_new_alm_track%10+48;
				display_set_text(&c[0]);
				break;
				
		case 6:	setup_alm_time_display_mode=7;
				clock_stop_stop_watch();clock_start_stop_watch();
				if((alarm_mode==UI_MENUES_ALARM_ON)||(alarm_mode==UI_MENUES_ALARM_ON_MP3)||(alarm_mode==UI_MENUES_ALARM_ON_RADIO)){
					display_set_text("on  ");
				}else{
					display_set_text("off ");
				}
				break;
				
		case 7:	if(clock_get_stop_watch()*4>2500){//2.5seconds waiting
					settings_save(SETTINGS_ALARM_MODE, alarm_mode);
					settings_save (SETTINGS_ALARM_HOUR, alarm_hour);
					settings_save (SETTINGS_ALARM_MINUTE, alarm_minute);
					settings_save (SETTINGS_ALARM_MP3_TRACK, alarm_track);
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
2: beep mode (0=off, 1=short, 2=long, 3=mp3, 4=radio)
3: dow mode (0=always, 1=weekdays only)
4: the number of the track e.g. 04 for 04.mp3
*/


unsigned char schedules[5*SETTINGS_MAX_SCHEDULES]={0};

void ui_menues_load_schedules(void){
	unsigned int i=0;
	for(i=0;i<sizeof(schedules);i++){
		schedules[i]=settings_get(i+SETTINGS_SCHEDULE_OFFSET);
	}
	for(i=0;i<SETTINGS_MAX_SCHEDULES;i++){
		if( (schedules[i*5]>23)||(schedules[(i*5)+1]>59)||(schedules[(i*5)+2]>4)||(schedules[(i*5)+3]>1) ){
			schedules[i*5]=0;schedules[(i*5)+1]=0;schedules[(i*5)+2]=0;schedules[(i*5)+3]=0;schedules[(i*5)+4]=1;
		}
		
	}
}

void save_schedule(unsigned char sched){
	unsigned int i=0;
	for(i=sched*5;i<sched*5+5;i++){
		cli();
		wdt_reset();
		settings_save(i+SETTINGS_SCHEDULE_OFFSET, schedules[i]);
		sei();
	}
}

unsigned char schedule_beep_mode=0;
signed char schedule_min_ring=-1;
//0 no beep/schedule in action
//1 beep/schedule in action
unsigned char ui_menues_check_schedule(void){
	unsigned int i=0;
	unsigned char min,hour,second,day,month,year,dow;
	clock_get_time(&min,&hour,&second,&day,&month,&year,&dow);
	switch(schedule_beep_mode){
		case 0:	for(i=0;i<SETTINGS_MAX_SCHEDULES;i++){
					if(schedules[(i*5)+2]!=0){
						if((schedules[(i*5)+0]==hour)&&(schedules[(i*5)+1]==min)){
							if((schedules[(i*5)+3]==0) || ((schedules[(i*5)+3]==1)&&(dow<=5)) ){
								if(schedules[(i*5)+2]==1){
									schedule_beep_mode=10;//short beep
									schedule_min_ring=min;
									return 1;
								}else if(schedules[(i*5)+2]==2){
									schedule_beep_mode=20;//long beep
									schedule_min_ring=min;
									return 1;
								}else if(schedules[(i*5)+2]==3){//mp3 mode
									if(I2C_MP3_detected){
										clock_stop_stop_watch();clock_start_stop_watch();
										schedule_beep_mode=30;//mp3 play
										I2C_MP3_playSched(schedules[(i*5)+4]);
									}else{
										schedule_beep_mode=10;//short beep
									}
									schedule_min_ring=min;
									return 1;
								}else if(schedules[(i*5)+2]==4){//radio mode not supported yet
								}
							}
						}
					}
				}
				break;
		case 10: beeper_set_mode(BEEPER_ON);schedule_beep_mode=11;clock_stop_stop_watch();clock_start_stop_watch();return 1;break;
		case 11:if(clock_get_stop_watch()*4>3000){
					beeper_set_mode(BEEPER_OFF);
					clock_stop_stop_watch();
					schedule_beep_mode=100;//wait for next minute
				}
				return 1;
				break;

		case 20: beeper_set_mode(BEEPER_ON_FULL);schedule_beep_mode=21;clock_stop_stop_watch();clock_start_stop_watch();return 1;break;
		case 21:if(clock_get_stop_watch()*4>3000){
					beeper_set_mode(BEEPER_OFF);
					clock_stop_stop_watch();
					schedule_beep_mode=100;//wait for next minute
				}
				return 1;
				break;
		case 30:if(clock_get_stop_watch()>(50000/4)){
					if(I2C_MP3_detected){
						I2C_MP3_stopPlaying();
					}
					clock_stop_stop_watch();
					schedule_beep_mode=100;//wait for next minute
				}
				switch(ui_input_get_key()){
					case UI_INPUT_KEY_OK: if(I2C_MP3_detected){
										I2C_MP3_stopPlaying();
									}
									clock_stop_stop_watch();
									schedule_beep_mode=100;//wait for next minute
									break;
				}
				return 1;
				break;
		
		case 100: if(min!=schedule_min_ring){
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
unsigned char schedule_new_track=0;

//0 if the menu was exited
//1 otherwise
unsigned int setup_schedule(void){
	char c[8];
	unsigned int code;
	switch(setup_schedule_display_mode){
		case 0:setup_schedule_display_mode++;
				clock_stop_stop_watch();clock_start_stop_watch();
				setup_schedule_place=1;
				break;
		case 1:	switch(ui_input_number(&setup_schedule_place,1,50,30000/4)){
					case 3:
					case 0:	setup_schedule_display_mode=0;
							return 0;
							break;
					case 4:	if(schedules[(setup_schedule_place-1)*5+2]!=0){
								setup_schedule_display_mode=30;
							}else{
								setup_schedule_display_mode=33;
							}
							clock_stop_stop_watch();clock_start_stop_watch();
							break;
					
				}
				c[0]=setup_schedule_place/10+48;
				c[1]=setup_schedule_place%10+48;
				c[2]=' ';
				switch(schedules[(setup_schedule_place-1)*5+2]){
					case 0:c[3]='-';break;
					case 1:c[3]='S';break;
					case 2:c[3]='L';break;
					case 3:c[3]='M';break;
					case 4:c[3]='R';break;
				}
				display_set_text(&c[0]);
				break;
		case 2:	switch(ui_input_get_digits(2,0,10000,"Hour",&code)){//number of keys
					case 0: 	setup_schedule_display_mode=10;
								clock_stop_stop_watch();clock_start_stop_watch();
								break;
					case 2: 	setup_schedule_display_mode=1;
								clock_stop_stop_watch();clock_start_stop_watch();
								break;
					case 5:	if((code>=0) && (code<=23)){
								schedule_new_hour=code;
								setup_schedule_display_mode++;
							}else{
								setup_schedule_display_mode=0;
							}
							break;
				}
				break;
		case 3:	switch(ui_input_get_digits(2,0,10000,"Min ",&code)){//number of keys
					case 0: 	setup_schedule_display_mode=10;
								clock_stop_stop_watch();clock_start_stop_watch();
								break;
					case 2: 	setup_schedule_display_mode=1;
								clock_stop_stop_watch();clock_start_stop_watch();
								break;
					case 5:	if((code>=0) && (code<=59)){
								schedule_new_minute=code;
								setup_schedule_display_mode++;
							}else{
								setup_schedule_display_mode=0;
							}
							clock_stop_stop_watch();clock_start_stop_watch();
							break;
				}
				break;

		case 4:	if(clock_get_stop_watch()*4>10000){//10seconds waiting
					setup_schedule_display_mode=1;clock_stop_stop_watch();clock_start_stop_watch();
				}
				switch(ui_input_get_key()){
					case UI_INPUT_KEY_BACK: 	setup_schedule_display_mode=10;
									clock_stop_stop_watch();clock_start_stop_watch();
									break;
					case UI_INPUT_KEY_UP:  	switch(schedule_new_beep){
											case 0: schedule_new_beep=1;
													break;
											case 1: if(I2C_RADIO_detected){
														schedule_new_beep=4;
													}else if(I2C_MP3_detected){
														schedule_new_beep=3;
													}else{
														schedule_new_beep=2;
													}
													break;
											case 2:	schedule_new_beep=1;
													break;
											case 3:	schedule_new_beep=2;
													break;
											case 4:	if(I2C_MP3_detected){
														schedule_new_beep=3;
													}else{
														schedule_new_beep=2;
													}
													break;
										}
										clock_stop_stop_watch();clock_start_stop_watch();
										break;
					case UI_INPUT_KEY_DOWN: 	switch(schedule_new_beep){
											case 0: schedule_new_beep=1;
													break;
											case 1: schedule_new_beep=2;
													break;
											case 2:	if(I2C_MP3_detected){
														schedule_new_beep=3;
													}else if(I2C_RADIO_detected){
														schedule_new_beep=4;
													}else{
														schedule_new_beep=1;
													}
													break;
											case 3: if(I2C_RADIO_detected){
														schedule_new_beep=4;
													}else{
														schedule_new_beep=1;
													}
													break;
											case 4: schedule_new_beep=1;
													break;
										}
										clock_stop_stop_watch();clock_start_stop_watch();
										break;
					case UI_INPUT_KEY_OK: 	if(schedule_new_beep==3){
										setup_schedule_display_mode=40;
										if(I2C_MP3_detected){
											I2C_MP3_playSched(schedule_new_track);
										}
									}else{
										setup_schedule_display_mode++;
									}
									clock_stop_stop_watch();clock_start_stop_watch();
									break;
				}
				switch(schedule_new_beep){
					case 0:	display_set_text(" off");
							break;
					case 1:	display_set_text("   S");
							break;
					case 2:	display_set_text("   L");
							break;
					case 3:	display_set_text(" MP3");
							break;
					case 4:	display_set_text("  RA");
							break;
				}
				break;

		case 40:	switch(ui_input_number(&schedule_new_track,0,99,30000/4)){
						case 3:
						case 0:	if(I2C_MP3_detected){
									I2C_MP3_stopPlaying();
								}
								setup_schedule_display_mode=0;
								break;
						case 2:if(I2C_MP3_detected){
									I2C_MP3_playSched(schedule_new_track);
								}
								break;
						case 4:	if(I2C_MP3_detected){
									I2C_MP3_stopPlaying();
								}
								setup_schedule_display_mode=5;
								break;
					
					}
					c[0]=' ';
					c[1]=' ';
					c[2]=schedule_new_track/10+48;
					c[3]=schedule_new_track%10+48;
					display_set_text(&c[0]);
					break;

		case 5:	if(clock_get_stop_watch()*4>10000){//10seconds waiting
						setup_schedule_display_mode=1;clock_stop_stop_watch();clock_start_stop_watch();
				}
				switch(ui_input_get_key()){
					case UI_INPUT_KEY_BACK: 	setup_schedule_display_mode=10;
									clock_stop_stop_watch();clock_start_stop_watch();
									break;
					case UI_INPUT_KEY_UP: 	if(schedule_new_dow>0){
											schedule_new_dow--;
										}
										clock_stop_stop_watch();clock_start_stop_watch();
										break;
					case UI_INPUT_KEY_DOWN: 	if(schedule_new_dow<1){
											schedule_new_dow++;
										}
										clock_stop_stop_watch();clock_start_stop_watch();
										break;
					case UI_INPUT_KEY_OK: 	setup_schedule_display_mode++;clock_stop_stop_watch();clock_start_stop_watch();
									break;
				}
				switch(schedule_new_dow){
					case 0:	display_set_text(" 1-7");
							break;
					case 1:	display_set_text(" 1-5");
							break;
				}
				break;		

		case 6: setup_schedule_display_mode=1;
				schedules[(setup_schedule_place-1)*5+0]=schedule_new_hour;
				schedules[(setup_schedule_place-1)*5+1]=schedule_new_minute;
				schedules[(setup_schedule_place-1)*5+2]=schedule_new_beep;
				schedules[(setup_schedule_place-1)*5+3]=schedule_new_dow;
				schedules[(setup_schedule_place-1)*5+4]=schedule_new_track;
				save_schedule(setup_schedule_place-1);
				clock_stop_stop_watch();clock_start_stop_watch();
				break;
		
		case 10: setup_schedule_display_mode=1;
				schedules[(setup_schedule_place-1)*5+2]=0;
				clock_stop_stop_watch();clock_start_stop_watch();
				save_schedule(setup_schedule_place-1);
				break; //disable schedule
		
		case 30:c[0]=schedules[(setup_schedule_place-1)*5+0]/10+48;
				c[1]=' ';
				c[2]=schedules[(setup_schedule_place-1)*5+0]%10+48;
				c[3]='|';
				c[4]=schedules[(setup_schedule_place-1)*5+1]/10+48;
				c[5]=' ';
				c[6]=schedules[(setup_schedule_place-1)*5+1]%10+48;
				c[7]=' ';
				display_set_time(&c[0]);
				if(clock_get_stop_watch()*4>2000){
					setup_schedule_display_mode=31;
					clock_stop_stop_watch();clock_start_stop_watch();
				}
				break;
		case 31:switch(schedules[(setup_schedule_place-1)*5+2]){
					case 0:display_set_text(" off");
							break;
					case 1:	display_set_text("   S");
							break;
					case 2:	display_set_text("   L");
							break;
					case 3:	display_set_text(" MP3");
							break;
					case 4:	display_set_text("  RA");
							break;
				}
				if(clock_get_stop_watch()*4>1000){
					setup_schedule_display_mode=32;
					clock_stop_stop_watch();clock_start_stop_watch();
				}
				break;
		case 32:
				switch(schedules[(setup_schedule_place-1)*5+3]){
					case 0:	display_set_text(" 1-7");
							break;
					case 1:	display_set_text(" 1-5");
							break;
				}
				if(clock_get_stop_watch()*4>1000){
					setup_schedule_display_mode=33;
					clock_stop_stop_watch();clock_start_stop_watch();
				}
				break;
		case 33:	if(clock_get_stop_watch()*4>3000){//3seconds waiting
						setup_schedule_display_mode=1;
					}
					switch(ui_input_get_key()){
						case UI_INPUT_KEY_BACK: 	setup_schedule_display_mode=1;
										break;
						case UI_INPUT_KEY_OK: 	setup_schedule_display_mode=2;clock_stop_stop_watch();clock_start_stop_watch();
										schedule_new_hour=schedules[(setup_schedule_place-1)*5+0];
										schedule_new_minute=schedules[(setup_schedule_place-1)*5+1];
										schedule_new_beep=schedules[(setup_schedule_place-1)*5+2];
										if(schedule_new_beep==0) schedule_new_beep=1;
										schedule_new_dow=schedules[(setup_schedule_place-1)*5+3];
										schedule_new_track=schedules[(setup_schedule_place-1)*5+4];if((schedule_new_track>99)||(schedule_new_track==0)) schedule_new_track=1;
										break;
					}
					display_set_text("Set ");
					break;
	}
	return 1;
}


unsigned char main_menu_display_mode=0;
unsigned char amb_track=1;
unsigned char slp_track=1;
unsigned char dot_setup=UI_DISPLAY_MODES_DOT_MODE_DOT;
unsigned char mp3_volume=55;
//0 if the menu was exited
//1 still busy
//2 if the show mode was changed
unsigned int ui_menues_main_menu_input(void){
	unsigned char c=0;
	signed int code=0;
	char text[8];
	switch(main_menu_display_mode){
		case 0:	clock_stop_stop_watch();clock_start_stop_watch();
				main_menu_display_mode=1;display_set_mode(DISPLAY_7SEG_BRIGHT);
				if(ui_menues_get_code()>=0){
					main_menu_display_mode=250;
				}
				break;
		case 1:	if(clock_get_stop_watch()*4>10000){//10seconds waiting
						main_menu_display_mode=0;
						return 0;
				}
				switch(ui_input_get_key()){
					case UI_INPUT_KEY_BACK: 	main_menu_display_mode=0;
									return 0;
									break;
					case UI_INPUT_KEY_UP: 	main_menu_display_mode=2;clock_stop_stop_watch();clock_start_stop_watch();
										break;
					case UI_INPUT_KEY_DOWN: if(ui_display_modes_get_fixed_mode()){
											if(I2C_MP3_detected){
												main_menu_display_mode=17;
											}else{
												main_menu_display_mode=5;
											}
										}else{
											main_menu_display_mode=18;
										}
										clock_stop_stop_watch();clock_start_stop_watch();
										break;
					case UI_INPUT_KEY_OK: 	main_menu_display_mode=40;clock_stop_stop_watch();clock_start_stop_watch();
										break;
				}
				display_set_text("Stop");
				break;
		case 2:	if(clock_get_stop_watch()*4>10000){//10seconds waiting
						main_menu_display_mode=0;
						return 0;
				}
				switch(ui_input_get_key()){
					case UI_INPUT_KEY_BACK: 	main_menu_display_mode=0;
									return 0;
									break;
					case UI_INPUT_KEY_UP: 	main_menu_display_mode=3;clock_stop_stop_watch();clock_start_stop_watch();
										break;
					case UI_INPUT_KEY_DOWN: 	main_menu_display_mode=1;clock_stop_stop_watch();clock_start_stop_watch();
										break;
					case UI_INPUT_KEY_OK: 		main_menu_display_mode=80;
										clock_stop_stop_watch();clock_start_stop_watch();
										break;
				}
				display_set_text("ALM ");
				break;

		case 3:	if(clock_get_stop_watch()*4>10000){//10seconds waiting
						main_menu_display_mode=0;
						return 0;
				}
				switch(ui_input_get_key()){
					case UI_INPUT_KEY_BACK: 	main_menu_display_mode=0;
									return 0;
									break;
					case UI_INPUT_KEY_UP: 	main_menu_display_mode=4;clock_stop_stop_watch();clock_start_stop_watch();
										break;
					case UI_INPUT_KEY_DOWN: 	main_menu_display_mode=2;clock_stop_stop_watch();clock_start_stop_watch();
										break;
					case UI_INPUT_KEY_OK: 	main_menu_display_mode=50;clock_stop_stop_watch();clock_start_stop_watch();
										break;
				}
				display_set_text("Time");
				break;

		case 4:	if(clock_get_stop_watch()*4>10000){//10seconds waiting
						main_menu_display_mode=0;
						return 0;
				}
				switch(ui_input_get_key()){
					case UI_INPUT_KEY_BACK: 	main_menu_display_mode=0;
									return 0;
									break;
					case UI_INPUT_KEY_UP: 	main_menu_display_mode=5;clock_stop_stop_watch();clock_start_stop_watch();
										break;
					case UI_INPUT_KEY_DOWN: 	main_menu_display_mode=3;clock_stop_stop_watch();clock_start_stop_watch();
										break;
					case UI_INPUT_KEY_OK: 	main_menu_display_mode=90;clock_stop_stop_watch();clock_start_stop_watch();
										break;
				}
				display_set_text("Schd");
				break;
		case 5:	if(clock_get_stop_watch()*4>10000){//10seconds waiting
						main_menu_display_mode=0;
						return 0;
				}
				switch(ui_input_get_key()){
					case UI_INPUT_KEY_BACK: 	main_menu_display_mode=0;
									return 0;
									break;
					case UI_INPUT_KEY_UP: 	main_menu_display_mode=6;clock_stop_stop_watch();clock_start_stop_watch();
										break;
					case UI_INPUT_KEY_DOWN: 	main_menu_display_mode=4;clock_stop_stop_watch();clock_start_stop_watch();
										break;
					case UI_INPUT_KEY_OK: 	dot_setup=ui_display_modes_get_dot_mode();main_menu_display_mode=140;clock_stop_stop_watch();clock_start_stop_watch();
										break;
				}
				display_set_text("Dot ");
				break;
		case 6:	if(clock_get_stop_watch()*4>10000){//10seconds waiting
						main_menu_display_mode=0;
						return 0;
				}
				switch(ui_input_get_key()){
					case UI_INPUT_KEY_BACK: 	main_menu_display_mode=0;
									return 0;
									break;
					case UI_INPUT_KEY_UP: 	main_menu_display_mode=7;clock_stop_stop_watch();clock_start_stop_watch();
										break;
					case UI_INPUT_KEY_DOWN: 	main_menu_display_mode=5;clock_stop_stop_watch();clock_start_stop_watch();
										break;
					case UI_INPUT_KEY_OK: 	dot_setup=ui_display_modes_get_dot_mode();main_menu_display_mode=150;clock_stop_stop_watch();clock_start_stop_watch();
										break;
				}
				display_set_text("PW  ");
				break;
				
		case 7:	if(clock_get_stop_watch()*4>10000){//10seconds waiting
						main_menu_display_mode=0;
						return 0;
				}
				switch(ui_input_get_key()){
					case UI_INPUT_KEY_BACK: 	main_menu_display_mode=0;
									return 0;
									break;
					case UI_INPUT_KEY_UP: 	clock_stop_stop_watch();clock_start_stop_watch();
										if(I2C_MP3_detected){
											main_menu_display_mode=15;
										}else if(ui_display_modes_get_fixed_mode()){
											main_menu_display_mode=1;
										}else{
											main_menu_display_mode=18;
										}
										break;
					case UI_INPUT_KEY_DOWN: 	main_menu_display_mode=6;clock_stop_stop_watch();clock_start_stop_watch();
										break;
					case UI_INPUT_KEY_OK: 	main_menu_display_mode=70;clock_stop_stop_watch();clock_start_stop_watch();
										break;
				}
				display_set_text("Illu");
				break;
		case 15:if(clock_get_stop_watch()*4>10000){//10seconds waiting
						main_menu_display_mode=0;
						return 0;
				}
				switch(ui_input_get_key()){
					case UI_INPUT_KEY_BACK: 	main_menu_display_mode=0;
									return 0;
									break;
					case UI_INPUT_KEY_UP: 	main_menu_display_mode=16;clock_stop_stop_watch();clock_start_stop_watch();
										break;
					case UI_INPUT_KEY_DOWN: 	main_menu_display_mode=7;clock_stop_stop_watch();clock_start_stop_watch();
										break;
					case UI_INPUT_KEY_OK: 	main_menu_display_mode=100;clock_stop_stop_watch();clock_start_stop_watch();
										break;
				}
				display_set_text("Cont");
				break;

		case 16:if(clock_get_stop_watch()*4>10000){//10seconds waiting
						main_menu_display_mode=0;
						return 0;
				}
				switch(ui_input_get_key()){
					case UI_INPUT_KEY_BACK: 	main_menu_display_mode=0;
									return 0;
									break;
					case UI_INPUT_KEY_UP: 	main_menu_display_mode=17;clock_stop_stop_watch();clock_start_stop_watch();
										break;
					case UI_INPUT_KEY_DOWN: 	main_menu_display_mode=15;clock_stop_stop_watch();clock_start_stop_watch();
										break;
					case UI_INPUT_KEY_OK: if(I2C_MP3_detected){
											I2C_MP3_setVol(mp3_volume);
										}
										main_menu_display_mode=110;clock_stop_stop_watch();clock_start_stop_watch();
										break;
				}
				display_set_text("Amb ");
				break;

		case 17:if(clock_get_stop_watch()*4>10000){//10seconds waiting
						main_menu_display_mode=0;
						return 0;
				}
				switch(ui_input_get_key()){
					case UI_INPUT_KEY_BACK: 	main_menu_display_mode=0;
									return 0;
									break;
					case UI_INPUT_KEY_UP: 	if(ui_display_modes_get_fixed_mode()){
											main_menu_display_mode=1;
										}else{
											main_menu_display_mode=18;
										}
										clock_stop_stop_watch();clock_start_stop_watch();
										break;
					case UI_INPUT_KEY_DOWN: 	main_menu_display_mode=16;clock_stop_stop_watch();clock_start_stop_watch();
										break;
					case UI_INPUT_KEY_OK: if(I2C_MP3_detected){
											I2C_MP3_setVol(mp3_volume);
										}
										main_menu_display_mode=120;clock_stop_stop_watch();clock_start_stop_watch();
										break;
				}
				display_set_text("Slp ");
				break;
		case 18:	if(clock_get_stop_watch()*4>10000){//10seconds waiting
						main_menu_display_mode=0;
						return 0;
				}
				switch(ui_input_get_key()){
					case UI_INPUT_KEY_BACK: 	main_menu_display_mode=0;
									return 0;
									break;
					case UI_INPUT_KEY_UP: 	main_menu_display_mode=1;clock_stop_stop_watch();clock_start_stop_watch();
										break;
					case UI_INPUT_KEY_DOWN: 	if(I2C_MP3_detected){
											main_menu_display_mode=17;
										}else{
											main_menu_display_mode=7;
										}
										clock_stop_stop_watch();clock_start_stop_watch();
										break;
					case UI_INPUT_KEY_OK: 	main_menu_display_mode=60;clock_stop_stop_watch();clock_start_stop_watch();
										break;
				}
				display_set_text("Mode");
				break;

		case 40:	if(stop_time()==0){
						main_menu_display_mode=1;clock_stop_stop_watch();clock_start_stop_watch();
					}
					break;
		case 50:	if(setup_time()==0){
						main_menu_display_mode=3;clock_stop_stop_watch();clock_start_stop_watch();
					}
					break;
		case 60:	c=set_show_mode();
					if(c==0){
						main_menu_display_mode=18;clock_stop_stop_watch();clock_start_stop_watch();
					}else if(c==2){
						main_menu_display_mode=0;clock_stop_stop_watch();clock_start_stop_watch();
						return 2;
					}
					break;
		case 70:	if(setup_dimm()==0){
						main_menu_display_mode=7;clock_stop_stop_watch();clock_start_stop_watch();
					}
					break;
		case 80:	if(setup_alm_time()==0){
						main_menu_display_mode=2;clock_stop_stop_watch();clock_start_stop_watch();
					}
					break;
		case 90:	if(setup_schedule()==0){
						main_menu_display_mode=4;clock_stop_stop_watch();clock_start_stop_watch();
					}
					break;
		case 100:	if(clock_get_stop_watch()*4>10000){//10seconds waiting
						main_menu_display_mode=15;clock_stop_stop_watch();clock_start_stop_watch();
						settings_save(SETTINGS_CONT_MODE, cont_mode);
					}
					switch(ui_input_get_key()){
						case UI_INPUT_KEY_BACK: 	main_menu_display_mode=15;
										break;
						case UI_INPUT_KEY_DOWN:
						case UI_INPUT_KEY_UP: if(cont_mode==UI_MENUES_CONT_MODE_ON){
											cont_mode=UI_MENUES_CONT_MODE_OFF;
										}else{
											cont_mode=UI_MENUES_CONT_MODE_ON;
										}
										clock_stop_stop_watch();clock_start_stop_watch();
										break;
						case UI_INPUT_KEY_OK: 	main_menu_display_mode=15;
										settings_save(SETTINGS_CONT_MODE, cont_mode);
										clock_stop_stop_watch();clock_start_stop_watch();
										break;
					}
					switch(cont_mode){
						case UI_MENUES_CONT_MODE_ON: display_set_text("on  ");
											break;
						case UI_MENUES_CONT_MODE_OFF: display_set_text("off ");
											break;
					}
					break;
		case 110:switch(ui_input_number(&amb_track,0,99,30000/4)){
					case 3:
					case 0:	if(I2C_MP3_detected){
								I2C_MP3_stopPlaying();
							}
							main_menu_display_mode=16;
							break;
					case 2:if(I2C_MP3_detected){
								I2C_MP3_playAmb(amb_track|0x80);
							}
							break;
					case 4:	if(I2C_MP3_detected){
								//I2C_MP3_playAmb(amb_track|0x80);
							}
							main_menu_display_mode=130;clock_stop_stop_watch();clock_start_stop_watch();
							break;
				
				}
				text[0]=' ';text[1]=' ';text[2]=amb_track/10+48;text[3]=amb_track%10+48;
				display_set_text(&text[0]);
				break;
		
		case 120:switch(ui_input_number(&slp_track,0,99,30000/4)){
					case 3:
					case 0:	if(I2C_MP3_detected){
								I2C_MP3_stopPlaying();
							}
							main_menu_display_mode=17;
							break;
					case 2:if(I2C_MP3_detected){
								I2C_MP3_playAmb(slp_track);
							}
							break;
					case 4:	if(I2C_MP3_detected){
								//I2C_MP3_playAmb(amb_track);
							}
							main_menu_display_mode=130;clock_stop_stop_watch();clock_start_stop_watch();
							break;
				}
				text[0]=' ';text[1]=' ';text[2]=slp_track/10+48;text[3]=slp_track%10+48;
				display_set_text(&text[0]);
				break;
		
		case 130:if(clock_get_stop_watch()*4>2000){//2 seconds waiting
						main_menu_display_mode=131;
						clock_stop_stop_watch();clock_start_stop_watch();
						if(I2C_MP3_detected){
							I2C_MP3_setVol(mp3_volume);
						}
					}
					display_set_text("vol ");
					break;
		case 131:switch(ui_input_number(&mp3_volume,0,255,30000/4)){
					case 3:
					case 0:	if(I2C_MP3_detected){
								I2C_MP3_stopPlaying();
							}
							main_menu_display_mode=0;
							break;
					case 2:if(I2C_MP3_detected){
								I2C_MP3_setVol(mp3_volume);
							}
							break;
					case 4:	main_menu_display_mode=0;clock_stop_stop_watch();clock_start_stop_watch();return 0;
							break;
				}
				if(255-mp3_volume>=100){
					text[0]=' ';text[1]=(255-mp3_volume)/100+48;text[2]=((255-mp3_volume)%100)/10+48;text[3]=(255-mp3_volume)%10+48;
				}else{
					text[0]=' ';text[1]=' ';text[2]=((255-mp3_volume)%100)/10+48;text[3]=(255-mp3_volume)%10+48;
				}
				display_set_text(&text[0]);
				break;
		case 140:if(clock_get_stop_watch()*4>10000){//10seconds waiting
					main_menu_display_mode=5;clock_stop_stop_watch();clock_start_stop_watch();
				}
				switch(ui_input_get_key()){
					case UI_INPUT_KEY_BACK: main_menu_display_mode=5;
											clock_stop_stop_watch();clock_start_stop_watch();
											break;
					case UI_INPUT_KEY_UP: if(dot_setup>0){
												dot_setup--;
											}
											clock_stop_stop_watch();clock_start_stop_watch();
											break;
					case UI_INPUT_KEY_DOWN:	if(dot_setup<UI_DISPLAY_MODES_DOT_MODE_COLON_DCF){
												dot_setup++;
											}
											clock_stop_stop_watch();clock_start_stop_watch();
											break;
					case UI_INPUT_KEY_OK: 	main_menu_display_mode=5;clock_stop_stop_watch();clock_start_stop_watch();
											ui_display_modes_set_dot_mode(dot_setup);
											settings_save(SETTINGS_UI_DISPLAY_DOT_MODE,ui_display_modes_get_dot_mode());
											break;
				}
				switch(dot_setup){
					case UI_DISPLAY_MODES_DOT_MODE_OFF:	display_set_text(" off");
							break;
					case UI_DISPLAY_MODES_DOT_MODE_DOT:	display_set_text("d on");
							break;
					case UI_DISPLAY_MODES_DOT_MODE_DOT_DCF:display_set_text("ddcf");
							break;
					case UI_DISPLAY_MODES_DOT_MODE_COLON:display_set_text("c on");
							break;
					case UI_DISPLAY_MODES_DOT_MODE_COLON_DCF:display_set_text("cdcf");
							break;
				}
				break;
		case 150:	if(setup_code()==0){
						main_menu_display_mode=6;clock_stop_stop_watch();clock_start_stop_watch();
					}
					break;
		case 250:	code=ui_input_code();
					if(code==-1){
						main_menu_display_mode=0;clock_stop_stop_watch();
						return 0;
					}else if(code == -2){
					}else if(code>=0){
						if(code==ui_menues_get_code()){
							main_menu_display_mode=1;clock_stop_stop_watch();clock_start_stop_watch();
						}else{
							main_menu_display_mode=0;clock_stop_stop_watch();
							return 0;
						}
					}
					break;
		default: main_menu_display_mode=0;return 0;break;
	}
	return 1;
}





#endif