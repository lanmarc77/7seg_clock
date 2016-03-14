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

#ifndef UI_DISPLAY_MODES_C
#define UI_DISPLAY_MODES_C
#include "config.h"
#include "ui_display_modes.h"
#include "ui_menus.h"
#include "ui_input.h"
#include "dcf77.h"
#include "clock.h"
#include "display.h"
#include "settings.h"
#include "i2c_modules.h"
#include <avr/io.h>
#include <avr/interrupt.h>



unsigned char dot_mode=UI_DISPLAY_MODES_DOT_MODE_DOT;
void ui_display_modes_set_dot_mode(unsigned char c){
	if(c>UI_DISPLAY_MODES_DOT_MODE_COLON_DCF){
		c=UI_DISPLAY_MODES_DOT_MODE_DOT;
	}
	dot_mode=c;
}

unsigned char ui_display_modes_get_dot_mode(void){
	return dot_mode;
}


unsigned char display_mode=0;

unsigned char ui_display_modes_get_mode(void){
	return display_mode;
}

void ui_display_modes_set_mode(unsigned char c){
	cli();
	if(c>7){
		c=2;
	}
	display_mode=c;
	sei();
}


unsigned char fixed_mode=0;

void ui_display_modes_set_fixed_mode(unsigned char c){
	cli();
	if(c>7){
		c=2;
	}
	fixed_mode=c;
	sei();
}

unsigned char ui_display_modes_get_fixed_mode(void){
	return fixed_mode;
}

 
//unsigned char show_dot_flag=1;
//returns 0-off
//returns 1-on
char get_dcf77_signal_dot(void){
	if((!I2C_RTC_detected && (clock_get_last_refresh(CLOCK_UPDATE_SOURCE_DCF77)>=24*60))||(I2C_RTC_detected && (clock_get_last_refresh(CLOCK_UPDATE_SOURCE_DCF77)>=24*7*60))){
		if(dcf77_get_state()==DCF77_STATE_NONE){
			return 0;
		}else{
			if((dcf77_get_signal()) == 0){//logic 0 input
				return 0;
			}else{
				return 1;
			}
		}
	}else{
		if(ui_display_modes_get_mode()==6){
			return 0;
		}else{
			return 1;
		}
	}
}

void fill_date(void){
	char c[8];c[1]=' ';c[3]=' ';c[5]=' ';c[7]=' ';
	unsigned char min,hour,second,day,month,year,dow;
	clock_get_time(&min,&hour,&second,&day,&month,&year,&dow);
	c[0]=day/10+48;
	c[2]=(day%10)+48;
	c[3]='.';
	c[4]=month/10+48;
	c[6]=(month%10)+48;
	c[7]='.';
	display_set_time(&c[0]);
}

void fill_dow_year(unsigned char lang){
	char c[4];
	unsigned char min,hour,second,day,month,year,dow;
	clock_get_time(&min,&hour,&second,&day,&month,&year,&dow);
	if(lang==1){//1=german
		switch(dow){
				case 1:	c[0]='M';
						c[1]='o';
						break;
				case 2:	c[0]='D';
						c[1]='i';
						break;
				case 3:	c[0]='M';
						c[1]='i';
						break;
				case 4:	c[0]='D';
						c[1]='o';
						break;
				case 5:	c[0]='F';
						c[1]='r';
						break;
				case 6:	c[0]='S';
						c[1]='a';
						break;
				case 7:	c[0]='S';
						c[1]='o';
						break;
			}
	}else{
		switch(dow){
			case 1:	c[0]='M';
					c[1]='o';
					break;
			case 2:	c[0]='T';
					c[1]='u';
					break;
			case 3:	c[0]='W';
					c[1]='e';
					break;
			case 4:	c[0]='T';
					c[1]='h';
					break;
			case 5:	c[0]='F';
					c[1]='r';
					break;
			case 6:	c[0]='S';
					c[1]='a';
					break;
			case 7:	c[0]='S';
					c[1]='u';
					break;
		}
	}
	c[2]=year/10+48;
	c[3]=year%10+48;
	display_set_text(&c[0]);
}

void fill_time(void){
	char c[8];c[1]=' ';c[3]=' ';c[5]=' ';c[7]=' ';
	unsigned char min,hour,second,day,month,year,dow;
	clock_get_time(&min,&hour,&second,&day,&month,&year,&dow);
	c[0]=hour/10+48;
	c[2]=(hour%10)+48;
	c[4]=min/10+48;
	c[6]=min%10+48;	
	if(dot_mode==UI_DISPLAY_MODES_DOT_MODE_DOT){
			c[3]='.';
	}else if(dot_mode==UI_DISPLAY_MODES_DOT_MODE_COLON){
			c[3]=':';
	}else if (get_dcf77_signal_dot()){
		if(dot_mode==UI_DISPLAY_MODES_DOT_MODE_DOT_DCF){
			c[3]='.';
		}else if(dot_mode==UI_DISPLAY_MODES_DOT_MODE_COLON_DCF){
			c[3]=':';
		}
	}
	display_anim_on();
	display_set_time(&c[0]);
}

void fill_temp(void){
	signed int temp=0;
	char c[8];c[1]=' ';c[3]=' ';c[5]=' ';c[7]=' ';
	if(I2C_TEMP_detected){
		if(I2C_getTemp(&temp)){
			if(temp>=0){
				c[0]=temp/100+48;
				c[2]=((temp%100)/10)+48;
				c[3]='.';
				c[4]=temp%10+48;
				c[6]='C';
				if(temp<100){
					c[0]=' ';
				}
			}else{
				temp=-temp;
				c[0]='-';
				c[2]=((temp)/100)+48;
				c[4]=(((temp)%100)/10)+48;
				c[5]='.';
				c[6]=((temp)%10)+48;
				if(temp<100){
					c[2]=c[4];
					c[4]=c[6];
					c[6]='C';
					c[5]=' ';
					c[3]='.';
				}
			}
			display_set_time(&c[0]);
		}
	}else{
		c[0]='-';c[2]='-';c[4]='-';c[6]='-';
		display_set_time(&c[0]);
	}
}



unsigned char ta_display_mode=0;
unsigned int ta_i=0;
void ui_display_modes_TA(void){
	unsigned char min,hour,second,day,month,year,dow;
	char c[4];
	unsigned int c_time=0;
	unsigned char def=0;
	ui_menues_set_stop_beep_mode(UI_MENUES_STOP_BEEP_SHORT);
	switch(ta_display_mode){
		case 0:	clock_get_time(&min,&hour,&second,&day,&month,&year,&dow);
				c_time=min+hour*60;
				if((dow==1)||(dow==2)||(dow==3)||(dow==4)){
					if(	((min==59)&&(hour==7))||
						((min==44)&&(hour==9))||
						((min==29)&&(hour==11))||
						((min==44)&&(hour==13))
					   )
					{
						display_set_mode(DISPLAY_7SEG_ZOOM);
						def|=2;
					}
					if(((second%10==0)||((second+1)%10==0))&&(1)){
						if((c_time>=8*60+00)&&(c_time<9*60+30)){
							c[0]='L';
							c[1]='-';
							c[2]=(9*60+30-c_time)/10+48;
							c[3]=(9*60+30-c_time)%10+48;
							display_set_text(&c[0]);
						}else if((c_time>=9*60+30)&&(c_time<9*60+45)){
							c[0]='b';
							c[1]='-';
							c[2]=(9*60+45-c_time)/10+48;
							c[3]=(9*60+45-c_time)%10+48;
							display_set_text(&c[0]);
						}else if((c_time>=9*60+45)&&(c_time<11*60+15)){
							c[0]='L';
							c[1]='-';
							c[2]=(11*60+15-c_time)/10+48;
							c[3]=(11*60+15-c_time)%10+48;
							display_set_text(&c[0]);
						}else if((c_time>=11*60+15)&&(c_time<11*60+30)){
							c[0]='b';
							c[1]='-';
							c[2]=(11*60+30-c_time)/10+48;
							c[3]=(11*60+30-c_time)%10+48;
							display_set_text(&c[0]);
						}else if((c_time>=11*60+30)&&(c_time<13*60+00)){
							c[0]='L';
							c[1]='-';
							c[2]=(13*60+00-c_time)/10+48;
							c[3]=(13*60+00-c_time)%10+48;
							display_set_text(&c[0]);
						}else if((c_time>=13*60+00)&&(c_time<13*60+45)){
							c[0]='b';
							c[1]='-';
							c[2]=(13*60+45-c_time)/10+48;
							c[3]=(13*60+45-c_time)%10+48;
							display_set_text(&c[0]);
						}else if((c_time>=13*60+45)&&(c_time<15*60+15)){
							c[0]='L';
							c[1]='-';
							c[2]=(15*60+15-c_time)/10+48;
							c[3]=(15*60+15-c_time)%10+48;
							display_set_text(&c[0]);
						}else{
							def|=1;
						}
					}else{
						def|=1;
					}
				}else if(dow==5){
					if(	((min==29)&&(hour==7))||
						((min==14)&&(hour==9))||
						((min==59)&&(hour==10))
					   )
					{
						display_set_mode(DISPLAY_7SEG_ZOOM);
						def|=2;
					}
					if(((second%10==0)||((second+1)%10==0))&&(1)){
						if((c_time>=7*60+30)&&(c_time<9*60+00)){
							c[0]='L';
							c[1]='-';
							c[2]=(9*60+00-c_time)/10+48;
							c[3]=(9*60+00-c_time)%10+48;
							display_set_text(&c[0]);
						}else if((c_time>=9*60+00)&&(c_time<9*60+15)){
							c[0]='b';
							c[1]='-';
							c[2]=(9*60+15-c_time)/10+48;
							c[3]=(9*60+15-c_time)%10+48;
							display_set_text(&c[0]);
						}else if((c_time>=9*60+15)&&(c_time<10*60+45)){
							c[0]='L';
							c[1]='-';
							c[2]=(10*60+45-c_time)/10+48;
							c[3]=(10*60+45-c_time)%10+48;
							display_set_text(&c[0]);
						}else if((c_time>=10*60+45)&&(c_time<11*60+00)){
							c[0]='b';
							c[1]='-';
							c[2]=(11*60+00-c_time)/10+48;
							c[3]=(11*60+00-c_time)%10+48;
							display_set_text(&c[0]);
						}else if((c_time>=11*60+00)&&(c_time<12*60+30)){
							c[0]='L';
							c[1]='-';
							c[2]=(12*60+30-c_time)/10+48;
							c[3]=(12*60+30-c_time)%10+48;
							display_set_text(&c[0]);
						}else{
							def|=1;
						}
					}else{
						def|=1;
					}
				}else{//weekends
					def|=1;
				}
				if(!(def&2)){
					if((hour>=20)||(hour<7)){//energy save mode between 20:00-07:00
						display_set_mode(DISPLAY_7SEG_DIM);
					}else{
						display_set_mode(DISPLAY_7SEG_BRIGHT);
					}
				}
				if(def&1){
					fill_time();
				}
				if(ui_menues_check_schedule()==0){
					switch(ui_input_get_key()){
						case UI_INPUT_KEY_OK:	ta_display_mode=2;
										break;
						case UI_INPUT_KEY_UP: 	ta_display_mode=50;
											break;
						case UI_INPUT_KEY_DOWN: ta_display_mode=60;
											break;
						case UI_INPUT_KEY_BACK: 		if(I2C_MP3_detected){
												I2C_MP3_talkTime();
											}
											break;
					}
				}
				break;

		/*case 1:	ta_i=ui_input_code();
				if(ta_i==0){//code insert canceld
					ta_display_mode=0;
				}else if(ta_i==1){//code is still entered
				}else if(ta_i==1306){//code correctly entered
					ta_display_mode=2;
				}
				break;*/

		case 2:	ta_i=ui_menues_main_menu_input();
				if(ta_i==0){//menu canceled
					ta_display_mode=0;
				}else if(ta_i==2){//new show mode
					ta_display_mode=0;
				}
				break;

		case 50:	clock_start_stop_watch();
					fill_temp();
					ta_display_mode=51;
					break;
		case 51:	fill_temp();
					if(clock_get_stop_watch()*4>2000){//2seconds waiting
						clock_stop_stop_watch();
						ta_display_mode=0;
					}
					break;
		case 60:	clock_start_stop_watch();
					fill_date();
					ta_display_mode=61;
					break;
		case 61:	if(clock_get_stop_watch()*4>2000){//2seconds waiting
						clock_stop_stop_watch();
						clock_start_stop_watch();
						ta_display_mode=62;
						fill_dow_year(0);
					}
					break;
		case 62:	if(clock_get_stop_watch()*4>2000){//2seconds waiting
						clock_stop_stop_watch();
						ta_display_mode=0;
					}
					break;
		default: 
				break;
	}


}


unsigned char wbs_display_mode=0;
unsigned int wbs_i=0;
void ui_display_modes_WBS(void){
	char c[4];
	unsigned char def=0;
	unsigned int c_time=0;
	unsigned char min,hour,second,day,month,year,dow;
	ui_menues_set_stop_beep_mode(UI_MENUES_STOP_BEEP_SHORT);
	switch(wbs_display_mode){
		case 0:	clock_get_time(&min,&hour,&second,&day,&month,&year,&dow);
				c_time=min+hour*60;
				if((dow==1)||(dow==2)||(dow==3)||(dow==4)){
					if(	((min==59)&&(hour==7))||
						((min==44)&&(hour==9))||
						((min==29)&&(hour==11))||
						((min==29)&&(hour==13))
					   )
					{
						display_set_mode(DISPLAY_7SEG_ZOOM);
						def|=2;
					}
					if(((second%10==0)||((second+1)%10==0))&&(1)){
						if((c_time>=8*60+00)&&(c_time<9*60+30)){
							c[0]='S';
							c[1]='-';
							c[2]=(9*60+30-c_time)/10+48;
							c[3]=(9*60+30-c_time)%10+48;
							display_set_text(&c[0]);
						}else if((c_time>=9*60+30)&&(c_time<9*60+45)){
							c[0]='P';
							c[1]='-';
							c[2]=(9*60+45-c_time)/10+48;
							c[3]=(9*60+45-c_time)%10+48;
							display_set_text(&c[0]);
						}else if((c_time>=9*60+45)&&(c_time<11*60+15)){
							c[0]='S';
							c[1]='-';
							c[2]=(11*60+15-c_time)/10+48;
							c[3]=(11*60+15-c_time)%10+48;
							display_set_text(&c[0]);
						}else if((c_time>=11*60+15)&&(c_time<11*60+30)){
							c[0]='P';
							c[1]='-';
							c[2]=(11*60+30-c_time)/10+48;
							c[3]=(11*60+30-c_time)%10+48;
							display_set_text(&c[0]);
						}else if((c_time>=11*60+30)&&(c_time<13*60+00)){
							c[0]='S';
							c[1]='-';
							c[2]=(13*60+00-c_time)/10+48;
							c[3]=(13*60+00-c_time)%10+48;
							display_set_text(&c[0]);
						}else if((c_time>=13*60+00)&&(c_time<13*60+30)){
							c[0]='P';
							c[1]='-';
							c[2]=(13*60+30-c_time)/10+48;
							c[3]=(13*60+30-c_time)%10+48;
							display_set_text(&c[0]);
						}else if((c_time>=13*60+30)&&(c_time<15*60+00)){
							c[0]='S';
							c[1]='-';
							c[2]=(15*60+00-c_time)/10+48;
							c[3]=(15*60+00-c_time)%10+48;
							display_set_text(&c[0]);
						}else{
							def|=1;
						}
					}else{
						def|=1;
					}
				}else if(dow==5){
					if(	((min==59)&&(hour==7))||
						((min==44)&&(hour==9))||
						((min==29)&&(hour==11))
					   )
					{
						display_set_mode(DISPLAY_7SEG_ZOOM);
						def|=2;
					}
					if(((second%10==0)||((second+1)%10==0))&&(1)){
						if((c_time>=8*60+00)&&(c_time<9*60+30)){
							c[0]='S';
							c[1]='-';
							c[2]=(9*60+30-c_time)/10+48;
							c[3]=(9*60+30-c_time)%10+48;
							display_set_text(&c[0]);
						}else if((c_time>=9*60+30)&&(c_time<9*60+45)){
							c[0]='P';
							c[1]='-';
							c[2]=(9*60+45-c_time)/10+48;
							c[3]=(9*60+45-c_time)%10+48;
							display_set_text(&c[0]);
						}else if((c_time>=9*60+45)&&(c_time<11*60+15)){
							c[0]='S';
							c[1]='-';
							c[2]=(11*60+15-c_time)/10+48;
							c[3]=(11*60+15-c_time)%10+48;
							display_set_text(&c[0]);
						}else if((c_time>=11*60+15)&&(c_time<11*60+30)){
							c[0]='P';
							c[1]='-';
							c[2]=(11*60+30-c_time)/10+48;
							c[3]=(11*60+30-c_time)%10+48;
							display_set_text(&c[0]);
						}else if((c_time>=11*60+30)&&(c_time<13*60+00)){
							c[0]='S';
							c[1]='-';
							c[2]=(13*60+00-c_time)/10+48;
							c[3]=(13*60+00-c_time)%10+48;
							display_set_text(&c[0]);
						}else{
							def|=1;
						}
					}else{
						def|=1;
					}
				}else{//weekends
					def|=1;
				}
				if(!(def&2)){
					if((hour>=20)||(hour<7)){//energy save mode between 20:00-07:00
						display_set_mode(DISPLAY_7SEG_DIM);
					}else{
						display_set_mode(DISPLAY_7SEG_BRIGHT);
					}
				}
				if(def&1){
					fill_time();
				}				
				if(ui_menues_check_schedule()==0){
					switch(ui_input_get_key()){
						case UI_INPUT_KEY_OK:	wbs_display_mode=2;
										break;
						case UI_INPUT_KEY_UP: 	wbs_display_mode=50;
											break;
						case UI_INPUT_KEY_DOWN: wbs_display_mode=60;
											break;
						case UI_INPUT_KEY_BACK: 		if(I2C_MP3_detected){
												I2C_MP3_talkTime();
											}
											break;
					}
				}
				break;

		/*case 1:	wbs_i=ui_input_code();
				if(wbs_i==0){//code insert canceld
					wbs_display_mode=0;
				}else if(wbs_i==1){//code is still entered
				}else if(wbs_i==1306){//code correctly entered
					wbs_display_mode=2;
				}
				break;*/

		case 2:	wbs_i=ui_menues_main_menu_input();
				if(wbs_i==0){//menu canceled
					wbs_display_mode=0;
				}else if(wbs_i==2){//new show mode
					wbs_display_mode=0;
				}
				break;

		case 50:	clock_start_stop_watch();
					fill_temp();
					wbs_display_mode=51;
					break;
		case 51:	fill_temp();
					if(clock_get_stop_watch()*4>2000){//2seconds waiting
						clock_stop_stop_watch();
						wbs_display_mode=0;
					}
					break;
		case 60:	clock_start_stop_watch();
					fill_date();
					wbs_display_mode=61;
					break;
		case 61:	if(clock_get_stop_watch()*4>2000){//2seconds waiting
						clock_stop_stop_watch();
						clock_start_stop_watch();
						wbs_display_mode=62;
						fill_dow_year(1);
					}
					break;
		case 62:	if(clock_get_stop_watch()*4>2000){//2seconds waiting
						clock_stop_stop_watch();
						wbs_display_mode=0;
					}
					break;
		default: 
				break;
	}
}

unsigned char bin_display_mode=0;
unsigned int bin_i=0;
void ui_display_modes_bin(void){
	char c[8];
	unsigned char min,hour,second,day,month,year,dow;
	ui_menues_set_stop_beep_mode(UI_MENUES_STOP_BEEP_SHORT);
	switch(bin_display_mode){
		case 0:
				clock_get_time(&min,&hour,&second,&day,&month,&year,&dow);
				c[0]=(hour/10);
				c[1]=' ';
				c[2]=(hour%10);
				c[3]=' ';
				c[4]=(min/10);
				c[5]=' ';
				c[6]=(min%10);
				c[7]=' ';
				if(get_dcf77_signal_dot()){
					c[3]='.';
				}
				if((hour>=22)||(hour<=7)){
					display_set_mode(DISPLAY_7SEG_DIM);
				}else{
					display_set_mode(DISPLAY_7SEG_BRIGHT);
				}
				display_set_time(&c[0]);
			
				if(ui_menues_check_schedule()==0){
					switch(ui_input_get_key()){
						case UI_INPUT_KEY_OK:	bin_display_mode=2;
										break;
						case UI_INPUT_KEY_BACK: 		if(I2C_MP3_detected){
												I2C_MP3_talkTime();
											}
											break;
					}
				}
				break;

		/*case 1:	bin_i=ui_input_code();
				if(bin_i==0){//code insert canceld
					bin_display_mode=0;
				}else if(bin_i==1){//code is still entered
				}else if(bin_i==1306){//code correctly entered
					bin_display_mode=2;
				}
				break;*/

		case 2:	bin_i=ui_menues_main_menu_input();
				if(bin_i==0){//menu canceled
					bin_display_mode=0;
				}else if(bin_i==2){//new show mode
					bin_display_mode=0;
				}
				break;

		default: 
				break;
	}
}

unsigned char te_display_mode=0;
unsigned int te_i=0;
void ui_display_modes_TE(void){
	char c[4];
	unsigned char def=0;
	unsigned int c_time=0;
	unsigned char min,hour,second,day,month,year,dow;
	ui_menues_set_stop_beep_mode(UI_MENUES_STOP_BEEP_SHORT);
	switch(te_display_mode){
		case 0:	clock_get_time(&min,&hour,&second,&day,&month,&year,&dow);
				c_time=min+hour*60;
				if((dow==1)||(dow==2)||(dow==3)||(dow==4)){

					if(((second%15==0)||((second+1)%15==0))&&(1)){
						if((c_time>=7*60+00)&&(c_time<9*60+00)){
							c[0]='-';
							c[1]=(9*60+00-c_time)/100+48;
							c[2]=((9*60+00-c_time)%100)/10+48;
							c[3]=(9*60+00-c_time)%10+48;
							display_set_text(&c[0]);
						}else if((c_time>=9*60+00)&&(c_time<9*60+20)){
							c[0]='P';
							c[1]='-';
							c[2]=(9*60+20-c_time)/10+48;
							c[3]=(9*60+20-c_time)%10+48;
							display_set_text(&c[0]);
						}else if((c_time>=9*60+20)&&(c_time<12*60+00)){
							c[0]='-';
							c[1]=(12*60+00-c_time)/100+48;
							c[2]=((12*60+00-c_time)%100)/10+48;
							c[3]=(12*60+00-c_time)%10+48;
							display_set_text(&c[0]);
						}else if((c_time>=12*60+00)&&(c_time<12*60+40)){
							c[0]='P';
							c[1]='-';
							c[2]=(12*60+40-c_time)/10+48;
							c[3]=(12*60+40-c_time)%10+48;
							display_set_text(&c[0]);
						}else if((c_time>=12*60+40)&&(c_time<15*60+00)){
							c[0]='-';
							c[1]=(15*60+00-c_time)/100+48;
							c[2]=((15*60+00-c_time)%100)/10+48;
							c[3]=(15*60+00-c_time)%10+48;
							display_set_text(&c[0]);
						}else{
							def|=1;
						}
					}else{
						def|=1;
					}
				}else if(dow==5){
					if(((second%15==0)||((second+1)%15==0))&&(1)){
						if((c_time>=7*60+00)&&(c_time<9*60+00)){
							c[0]='-';
							c[1]=(9*60+00-c_time)/100+48;
							c[2]=((9*60+00-c_time)%100)/10+48;
							c[3]=(9*60+00-c_time)%10+48;
							display_set_text(&c[0]);
						}else if((c_time>=9*60+00)&&(c_time<9*60+20)){
							c[0]='P';
							c[1]='-';
							c[2]=(9*60+20-c_time)/10+48;
							c[3]=(9*60+20-c_time)%10+48;
							display_set_text(&c[0]);
						}else if((c_time>=9*60+20)&&(c_time<12*60+00)){
							c[0]='-';
							c[1]=(12*60+00-c_time)/100+48;
							c[2]=((12*60+00-c_time)%100)/10+48;
							c[3]=(12*60+00-c_time)%10+48;
							display_set_text(&c[0]);
						}else if((c_time>=12*60+00)&&(c_time<12*60+40)){
							c[0]='P';
							c[1]='-';
							c[2]=(12*60+40-c_time)/10+48;
							c[3]=(12*60+40-c_time)%10+48;
							display_set_text(&c[0]);
						}else if((c_time>=12*60+40)&&(c_time<13*60+45)){
							c[0]='-';
							c[1]=(13*60+45-c_time)/100+48;
							c[2]=((13*60+45-c_time)%100)/10+48;
							c[3]=(13*60+45-c_time)%10+48;
							display_set_text(&c[0]);
						}else{
							def|=1;
						}
					}else{
						def|=1;
					}
				}else{//weekends
					def|=1;
				}
				if((hour>=19)||(hour<6)){//energy save mode between 20:00-07:00
					display_set_mode(DISPLAY_7SEG_DIM);
				}else{
					display_set_mode(DISPLAY_7SEG_BRIGHT);
				}
				if(def&1){
					if((((second%15+2)==0)||((second+3)%15==0))&&(1)){
						fill_date();
					}else{
						fill_time();
					}
				}				
				if(ui_menues_check_schedule()==0){
					switch(ui_input_get_key()){
						case UI_INPUT_KEY_OK:	te_display_mode=2;
										break;
						case UI_INPUT_KEY_UP: 	te_display_mode=50;
											break;
						case UI_INPUT_KEY_DOWN: te_display_mode=60;
											break;
						case UI_INPUT_KEY_BACK: 		if(I2C_MP3_detected){
												I2C_MP3_talkTime();
											}
											break;
					}
				}
				break;

		/*case 1:	te_i=ui_input_code();
				if(te_i==0){//code insert canceld
					te_display_mode=0;
				}else if(te_i==1){//code is still entered
				}else if(te_i==1306){//code correctly entered
					te_display_mode=2;
				}
				break;*/

		case 2:	te_i=ui_menues_main_menu_input();
				if(te_i==0){//menu canceled
					te_display_mode=0;
				}else if(te_i==2){//new show mode
					te_display_mode=0;
				}
				break;

		case 50:	clock_start_stop_watch();
					fill_temp();
					te_display_mode=51;
					break;
		case 51:	fill_temp();
					if(clock_get_stop_watch()*4>2000){//2seconds waiting
						clock_stop_stop_watch();
						te_display_mode=0;
					}
					break;
		case 60:	clock_start_stop_watch();
					fill_date();
					te_display_mode=61;
					break;
		case 61:	if(clock_get_stop_watch()*4>2000){//2seconds waiting
						clock_stop_stop_watch();
						clock_start_stop_watch();
						te_display_mode=62;
						fill_dow_year(1);
					}
					break;
		case 62:	if(clock_get_stop_watch()*4>2000){//2seconds waiting
						clock_stop_stop_watch();
						te_display_mode=0;
					}
					break;
		default: 
				break;
	}
}

unsigned char C1_display_mode=0;
unsigned int C1_i=0;
void ui_display_modes_C1(void){
	ui_menues_set_stop_beep_mode(UI_MENUES_STOP_BEEP_SHORT);
	unsigned char min,hour,second,day,month,year,dow;
	switch(C1_display_mode){
		case 0:	clock_get_time(&min,&hour,&second,&day,&month,&year,&dow);
				if(dow<5){
					//default clock: dimmed but every hour for one minute fully on
					if(min==0){
						display_set_mode(DISPLAY_7SEG_BRIGHT);
					}else{
						display_set_mode(DISPLAY_7SEG_DIM);
					}
					fill_time();
				}else{
					display_set_mode(DISPLAY_7SEG_DIM);
					fill_time();
				}
				if(ui_menues_check_schedule()==0){
					switch(ui_input_get_key()){
						case UI_INPUT_KEY_OK:	C1_display_mode=2;
										break;
						case UI_INPUT_KEY_UP: 	C1_display_mode=50;
											break;
						case UI_INPUT_KEY_DOWN: C1_display_mode=60;
											break;
						case UI_INPUT_KEY_BACK: 		if(I2C_MP3_detected){
												I2C_MP3_talkTime();
											}
											break;
					}
				}
				break;

		/*case 1:	C1_i=ui_input_code();
				if(C1_i==0){//code insert canceld
					C1_display_mode=0;
				}else if(C1_i==1){//code is still entered
				}else if(C1_i==1306){//code correctly entered
					C1_display_mode=2;
				}
				break;*/

		case 2:	C1_i=ui_menues_main_menu_input();
				if(C1_i==0){//menu canceled
					C1_display_mode=0;
				}else if(C1_i==2){//new show mode
					C1_display_mode=0;
				}
				break;

		case 50:	clock_start_stop_watch();
					fill_temp();
					C1_display_mode=51;
					break;
		case 51:	fill_temp();
					if(clock_get_stop_watch()*4>2000){//2seconds waiting
						clock_stop_stop_watch();
						C1_display_mode=0;
					}
					break;
		case 60:	clock_start_stop_watch();
					fill_date();
					C1_display_mode=61;
					break;
		case 61:	if(clock_get_stop_watch()*4>2000){//2seconds waiting
						clock_stop_stop_watch();
						clock_start_stop_watch();
						C1_display_mode=62;
						fill_dow_year(1);
					}
					break;
		case 62:	if(clock_get_stop_watch()*4>2000){//2seconds waiting
						clock_stop_stop_watch();
						C1_display_mode=0;
					}
					break;
		default: 
				break;
	}

}

unsigned char C2_display_mode=0;
unsigned int C2_i=0;
void ui_display_modes_C2(void){
	ui_menues_set_stop_beep_mode(UI_MENUES_STOP_BEEP_LONG);
	unsigned char min,hour,second,day,month,year,dow;
	switch(C2_display_mode){
		case 0:	clock_get_time(&min,&hour,&second,&day,&month,&year,&dow);
				if((hour>=22)||(hour<=7)){
					display_set_mode(DISPLAY_7SEG_DIM);
				}else{
					display_set_mode(DISPLAY_7SEG_BRIGHT);
				}
				if((second%20>=0)&&(second%20<=9)){
					fill_time();
				}else if((second%20>=10)&&(second%20<=14)){
					fill_date();
				}else if((second%20>=15)&&(second%20<=20)){
					fill_temp();
				}
				if(ui_menues_check_schedule()==0){
					switch(ui_input_get_key()){
						case UI_INPUT_KEY_OK:	C2_display_mode=2;
										break;
						case UI_INPUT_KEY_BACK: 		if(I2C_MP3_detected){
												I2C_MP3_talkTime();
											}
											break;
					}
				}
				break;
		/*case 1:	C2_i=ui_input_code();
				if(C2_i==0){//code insert canceled
					C2_display_mode=0;
				}else if(C2_i==1){//code is still entered
				}else if(C2_i==1306){//code correctly entered
					C2_display_mode=2;
				}
				break;*/

		case 2:	C2_i=ui_menues_main_menu_input();
				if(C2_i==0){//menu canceled
					C2_display_mode=0;
				}else if(C2_i==2){//new show mode
					C2_display_mode=0;
				}
				break;
		default: 
				break;
	}

}


unsigned char C3_display_mode=0;
unsigned int C3_i=0;
void ui_display_modes_C3(void){
	ui_menues_set_stop_beep_mode(UI_MENUES_STOP_BEEP_SHORT);
	unsigned char min,hour,second,day,month,year,dow;
	switch(C3_display_mode){
		case 0:	clock_get_time(&min,&hour,&second,&day,&month,&year,&dow);
				if((hour>=22)||(hour<=7)){
					display_set_mode(DISPLAY_7SEG_DIM);
				}else{
					display_set_mode(DISPLAY_7SEG_BRIGHT);
				}
				fill_time();
				if(ui_menues_check_schedule()==0){
					switch(ui_input_get_key()){
						case UI_INPUT_KEY_OK:	C3_display_mode=2;
										break;
						case UI_INPUT_KEY_UP: 	C3_display_mode=50;
											break;
						case UI_INPUT_KEY_DOWN: C3_display_mode=60;
											break;
						case UI_INPUT_KEY_BACK: 		if(I2C_MP3_detected){
												I2C_MP3_talkTime();
											}
											break;
					}
				}
				break;
		/*case 1:	C3_i=ui_input_code();
				if(C3_i==0){//code insert canceled
					C3_display_mode=0;
				}else if(C3_i==1){//code is still entered
				}else if(C3_i==1306){//code correctly entered
					C3_display_mode=2;
				}
				break;*/

		case 2:	C3_i=ui_menues_main_menu_input();
				if(C3_i==0){//menu canceled
					C3_display_mode=0;
				}else if(C3_i==2){//new show mode
					C3_display_mode=0;
				}
				break;
		case 50:	clock_start_stop_watch();
					fill_temp();
					C3_display_mode=51;
					break;
		case 51:	fill_temp();
					if(clock_get_stop_watch()*4>2000){//2seconds waiting
						clock_stop_stop_watch();
						C3_display_mode=0;
					}
					break;
		case 60:	clock_start_stop_watch();
					fill_date();
					C3_display_mode=61;
					break;
		case 61:	if(clock_get_stop_watch()*4>2000){//2seconds waiting
						clock_stop_stop_watch();
						clock_start_stop_watch();
						C3_display_mode=62;
						fill_dow_year(0);
					}
					break;
		case 62:	if(clock_get_stop_watch()*4>2000){//2seconds waiting
						clock_stop_stop_watch();
						C3_display_mode=0;
					}
					break;
		default: 
				break;
	}

}



#define CORR_TEMP -55
//unsigned long ad_res=0;
unsigned char simple_display_mode=0;
unsigned int simple_i=0;
void ui_display_modes_simple(void){
	unsigned char min,hour,second,day,month,year,dow;
	ui_menues_set_stop_beep_mode(UI_MENUES_STOP_BEEP_SHORT);
		switch(simple_display_mode){
			case 0:	clock_get_time(&min,&hour,&second,&day,&month,&year,&dow);
					if((hour>=22)&&(hour<7)){
						display_set_mode(DISPLAY_7SEG_DIM);
					}else{
						display_set_mode(DISPLAY_7SEG_BRIGHT);
					}
					if(second%10==0){
						fill_temp();
					}else{
						fill_time();
					}
					if(ui_menues_check_schedule()==0){
						switch(ui_input_get_key()){
							case UI_INPUT_KEY_OK:	simple_display_mode=2;
											break;
							case UI_INPUT_KEY_BACK: 		if(I2C_MP3_detected){
													I2C_MP3_talkTime();
												}
												break;
						}
					}
				break;

		/*case 1:	simple_i=ui_input_code();
				if(simple_i==0){//code insert canceld
					simple_display_mode=0;;
				}else if(simple_i==1){//code is still entered
				}else if(simple_i==1306){//code correctly entered
					simple_display_mode=2;
				}
				break;*/
		case 2:	simple_i=ui_menues_main_menu_input();
				if(simple_i==0){//menu canceled
					simple_display_mode=0;
				}else if(simple_i==2){//new show mode
					simple_display_mode=0;
				}
				break;
	}
}

unsigned char version_display_mode=0;
//0 version display finished
//1 still busy
unsigned char ui_display_modes_version(void){
	char c[4];
	unsigned char polarity=settings_get(SETTINGS_SEGMENT_MODE);
	switch(version_display_mode){
		case 0:	display_set_text(VERSION);
				clock_stop_stop_watch();
				clock_start_stop_watch();
				version_display_mode++;
				break;
		case 1:	if(clock_get_stop_watch()*4>2500){//2.5seconds waiting
					clock_stop_stop_watch();
					clock_start_stop_watch();
					display_set_mode(DISPLAY_7SEG_DIM);
					version_display_mode=2;
				}
				break;
		case 2:	if(clock_get_stop_watch()*4>2500){//2.5seconds waiting
					clock_stop_stop_watch();
					version_display_mode=0;
					display_set_mode(DISPLAY_7SEG_BRIGHT);
					return 0;
				}
				c[0]=' ';
				c[1]=' ';
				c[2]=' ';
				c[3]=' ';
				if(ui_menues_get_code()>=0){
					c[2]='P';
				}
				if(fixed_mode==0){
					c[3]=' ';
				}else{
					c[3]='F';
				}
				if(dcf77_get_signal_type()==DCF77_SIGNAL_TYPE_NORMAL){
					c[0]='N';
				}else{
					c[0]='I';
				}
				display_set_text(&c[0]);
				break;
		default: break;
	}
	switch(ui_input_get_key()){
			case UI_INPUT_KEY_OK:	if(polarity==DISPLAY_7SEG_POLARITY_NORMAL){
								polarity=DISPLAY_7SEG_POLARITY_INV;
							}else{
								polarity=DISPLAY_7SEG_POLARITY_NORMAL;
							}
							display_set_polarity(polarity);
							settings_save(SETTINGS_SEGMENT_MODE,polarity);
							clock_stop_stop_watch();clock_start_stop_watch();
							break;
			case UI_INPUT_KEY_BACK:	if(fixed_mode==0){
								fixed_mode=1;
							}else{
								fixed_mode=0;
							}
							settings_save(SETTINGS_DISPLAY_FIXED_MODE,fixed_mode);
							clock_stop_stop_watch();clock_start_stop_watch();
							break;
			case UI_INPUT_KEY_DOWN:
							if(dcf77_get_signal_type()==DCF77_SIGNAL_TYPE_NORMAL){
								dcf77_set_signal_type(DCF77_SIGNAL_TYPE_INVERTED);
							}else{
								dcf77_set_signal_type(DCF77_SIGNAL_TYPE_NORMAL);
							}
							settings_save(SETTINGS_DCF77_SIGNAL_TYPE,dcf77_get_signal_type());
							clock_stop_stop_watch();clock_start_stop_watch();
							break;
			case UI_INPUT_KEY_UP:
							ui_menues_set_code(-1);
							settings_save(SETTINGS_UI_MENUES_CODE_32,ui_menues_get_code()>>8);
							settings_save(SETTINGS_UI_MENUES_CODE_10,ui_menues_get_code()&0xFF);
							break;
	}
	return 1;
}

#endif