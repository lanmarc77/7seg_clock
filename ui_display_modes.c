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
#include "ui_setup_menus.h"
#include "ui_ir.h"
#include "clock.h"
#include "7seg_func.h"
#include "i2c_modules.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>


volatile unsigned char show_mode=0;
unsigned char old_second=0;
volatile unsigned char display_update=0;
unsigned char fixed_mode=0;
 
unsigned char show_dot_flag=1;
void show_dcf77_signal_dot(void){
	if(show_dot_flag){
		if((!I2C_RTC_detected && (no_dcf_signal>=(unsigned int)3600))||(I2C_RTC_detected && (no_dcf_signal>=(unsigned int)3600*12))){
			if(dcf_state==0){
				I_digits[1]&=~0x80;
			}else{
				if((filtered_dcf77) == 0){//logic 0 input
					I_digits[1]&=~0x80;
				}else{
					I_digits[1]|=0x80;
				}
			}
		}else{
			if(show_mode==6){
				I_digits[1]&=~0x80;
			}else{
				I_digits[1]|=0x80;
			}
		}
	}
}


void fill_date(void){
	I_COLON_MODE=COLON_OFF;
	I_digits[0]=I_day/10;
	I_digits[1]=(I_day%10)|0x80;
	I_digits[2]=I_month/10;
	I_digits[3]=(I_month%10)|0x80;
	show_dot_flag=0;
}

void fill_dow_year(unsigned char ger){
	I_COLON_MODE=COLON_OFF;
	if(ger){
		switch(I_dow){
				case 1:	I_digits[0]=L_M;
						I_digits[1]=L_o;
						break;
				case 2:	I_digits[0]=L_d;
						I_digits[1]=L_I;
						break;
				case 3:	I_digits[0]=L_M;
						I_digits[1]=L_I;
						break;
				case 4:	I_digits[0]=L_d;
						I_digits[1]=L_o;
						break;
				case 5:	I_digits[0]=L_F;
						I_digits[1]=L_r;
						break;
				case 6:	I_digits[0]=L_S;
						I_digits[1]=L_A;
						break;
				case 7:	I_digits[0]=L_S;
						I_digits[1]=L_o;
						break;
			}
	}else{
		switch(I_dow){
			case 1:	I_digits[0]=L_M;
					I_digits[1]=L_o;
					break;
			case 2:	I_digits[0]=L_t;
					I_digits[1]=L_U;
					break;
			case 3:	I_digits[0]=L_W;
					I_digits[1]=L_E;
					break;
			case 4:	I_digits[0]=L_t;
					I_digits[1]=L_H;
					break;
			case 5:	I_digits[0]=L_F;
					I_digits[1]=L_r;
					break;
			case 6:	I_digits[0]=L_S;
					I_digits[1]=L_A;
					break;
			case 7:	I_digits[0]=L_S;
					I_digits[1]=L_U;
					break;
		}
	}
	I_digits[2]=I_year/10;
	I_digits[3]=I_year%10;
	show_dot_flag=0;
}

void fill_time(void){
	I_COLON_MODE=COLON_ON;
	I_digits[0]=I_hour/10;
	I_digits[1]=(I_hour%10);
	I_digits[2]=I_minute/10;
	I_digits[3]=I_minute%10;
	show_dot_flag=1;
}

void fill_temp(void){
	unsigned int temp=0;
	if(I2C_TEMP_detected){
		if(I2C_getTemp(&temp)){
			if(temp>=0){
				I_digits[0]=temp/100;
				I_digits[1]=((temp%100)/10)|0x80;
				I_digits[2]=temp%10;
				I_digits[3]=L_C;
				if(I_digits[0]==0){
					I_digits[0]=L_NOTHING;
				}
			}else{
				I_digits[0]=L_minus;
				I_digits[1]=-temp/100;
				I_digits[2]=((-temp%100)/10)|0x80;
				I_digits[3]=-temp%10;
				if(I_digits[1]==0){
					I_digits[1]=I_digits[2];
					I_digits[2]=I_digits[3];
					I_digits[3]=L_C;
				}
			}
	
		}
	}else{
		I_digits[0]=L_minus;
		I_digits[1]=L_minus;
		I_digits[2]=L_minus;
		I_digits[3]=L_minus;
	}
}



unsigned char ta_display_mode=0;
unsigned int ta_i=0;
void TA_default(void){
	unsigned int c_time=0;
	stop_beep_mode=STOP_BEEP_SHORT;
	switch(ta_display_mode){
		case 0:	if((old_second!=I_second)||(display_update)){
					I_COLON_MODE=COLON_OFF;
					c_time=I_minute+I_hour*60;
					if((I_dow==1)||(I_dow==2)||(I_dow==3)||(I_dow==4)){
						fill_time();//default
						I_SEG_MODE=SEG_BRIGHT; //default
						if(	((I_minute==59)&&(I_hour==7))||
							((I_minute==44)&&(I_hour==9))||
							((I_minute==29)&&(I_hour==11))||
							((I_minute==44)&&(I_hour==13))
						   )
						{
							I_SEG_MODE=SEG_ZOOM;
						}else{
							I_SEG_MODE=SEG_BRIGHT;
						}
						if((c_time>=8*60+00)&&(c_time<9*60+30)){
							I_digits[0]=13;
							I_digits[1]=12;
							I_digits[2]=(9*60+30-c_time)/10;
							I_digits[3]=(9*60+30-c_time)%10;
							show_dot_flag=0;
						}else if((c_time>=9*60+30)&&(c_time<9*60+45)){
							I_digits[0]=14;
							I_digits[1]=12;
							I_digits[2]=(9*60+45-c_time)/10;
							I_digits[3]=(9*60+45-c_time)%10;
							show_dot_flag=0;
						}else if((c_time>=9*60+45)&&(c_time<11*60+15)){
							I_digits[0]=13;
							I_digits[1]=12;
							I_digits[2]=(11*60+15-c_time)/10;
							I_digits[3]=(11*60+15-c_time)%10;
							show_dot_flag=0;
						}else if((c_time>=11*60+15)&&(c_time<11*60+30)){
							I_digits[0]=14;
							I_digits[1]=12;
							I_digits[2]=(11*60+30-c_time)/10;
							I_digits[3]=(11*60+30-c_time)%10;
							show_dot_flag=0;
						}else if((c_time>=11*60+30)&&(c_time<13*60+00)){
							I_digits[0]=13;
							I_digits[1]=12;
							I_digits[2]=(13*60+00-c_time)/10;
							I_digits[3]=(13*60+00-c_time)%10;
							show_dot_flag=0;
						}else if((c_time>=13*60+00)&&(c_time<13*60+45)){
							I_digits[0]=14;
							I_digits[1]=12;
							I_digits[2]=(13*60+45-c_time)/10;
							I_digits[3]=(13*60+45-c_time)%10;
							show_dot_flag=0;
						}else if((c_time>=13*60+45)&&(c_time<15*60+15)){
							I_digits[0]=13;
							I_digits[1]=12;
							I_digits[2]=(15*60+15-c_time)/10;
							I_digits[3]=(15*60+15-c_time)%10;
							show_dot_flag=0;
						}
					}else if(I_dow==5){
						fill_time();
						I_SEG_MODE=SEG_BRIGHT;
						if(	((I_minute==29)&&(I_hour==7))||
							((I_minute==14)&&(I_hour==9))||
							((I_minute==59)&&(I_hour==10))
						   )
						{
							I_SEG_MODE=SEG_ZOOM;
						}else{
							I_SEG_MODE=SEG_BRIGHT;
						}
						if((c_time>=7*60+30)&&(c_time<9*60+00)){
							I_digits[0]=13;
							I_digits[1]=12;
							I_digits[2]=(9*60+00-c_time)/10;
							I_digits[3]=(9*60+00-c_time)%10;
							show_dot_flag=0;
						}else if((c_time>=9*60+00)&&(c_time<9*60+15)){
							I_digits[0]=14;
							I_digits[1]=12;
							I_digits[2]=(9*60+15-c_time)/10;
							I_digits[3]=(9*60+15-c_time)%10;
							show_dot_flag=0;
						}else if((c_time>=9*60+15)&&(c_time<10*60+45)){
							I_digits[0]=13;
							I_digits[1]=12;
							I_digits[2]=(10*60+45-c_time)/10;
							I_digits[3]=(10*60+45-c_time)%10;
							show_dot_flag=0;
						}else if((c_time>=10*60+45)&&(c_time<11*60+00)){
							I_digits[0]=14;
							I_digits[1]=12;
							I_digits[2]=(11*60+00-c_time)/10;
							I_digits[3]=(11*60+00-c_time)%10;
							show_dot_flag=0;
						}else if((c_time>=11*60+00)&&(c_time<12*60+30)){
							I_digits[0]=13;
							I_digits[1]=12;
							I_digits[2]=(12*60+30-c_time)/10;
							I_digits[3]=(12*60+30-c_time)%10;
							show_dot_flag=0;
						}
					}else{//weekends
						fill_time();
						I_SEG_MODE=SEG_BRIGHT;
					}
					display_update=0;
					if(((I_second%10==0)||((I_second+1)%10==0))&&(1)){//use precalculated values

					}else{//default display time all days in all modes
						fill_time();
					}
					old_second=I_second;
				}
				show_dcf77_signal_dot();
				if((I_hour>=20)||(I_hour<7)){//energy save mode between 20:00-07:00
					I_SEG_MODE=SEG_DIM;
				}
				if(check_schedule()==0){
					switch(get_ir_code()){
						case IR_MUTE:	ta_display_mode=1;I_COLON_MODE=COLON_OFF;show_dot_flag=0;
										break;
						case IR_CH_PLUS: 	ta_display_mode=50;I_COLON_MODE=COLON_OFF;show_dot_flag=0;
											break;
						case IR_CH_MINUS: ta_display_mode=60;I_COLON_MODE=COLON_OFF;show_dot_flag=0;
											break;
						case IR_POWER: 		if(I2C_MP3_detected){
												I2C_MP3_talkTime();
											}
											break;
					}
				}
				break;

		case 1:	ta_i=code_input();
				if(ta_i==0){//code insert canceld
					ta_display_mode=0;display_update=1;
				}else if(ta_i==1){//code is still entered
				}else if(ta_i==1306){//code correctly entered
					ta_display_mode=2;
				}
				break;

		case 2:	ta_i=main_menu_input();
				if(ta_i==0){//menu canceled
					ta_display_mode=0;display_update=1;
				}else if(ta_i==2){//new show mode
					ta_display_mode=0;display_update=1;
				}
				break;

		case 50:	start_stop_watch();
					fill_temp();
					ta_display_mode=51;
					break;
		case 51:	fill_temp();
					if(get_stop_watch()*4>2000){//2seconds waiting
						stop_stop_watch();
						display_update=1;
						ta_display_mode=0;display_update=1;
					}
					break;
		case 60:	start_stop_watch();
					fill_date();
					ta_display_mode=61;
					break;
		case 61:	if(get_stop_watch()*4>2000){//2seconds waiting
						stop_stop_watch();
						start_stop_watch();
						display_update=1;
						ta_display_mode=62;
						fill_dow_year(0);
					}
					break;
		case 62:	if(get_stop_watch()*4>2000){//2seconds waiting
						stop_stop_watch();
						display_update=1;
						ta_display_mode=0;display_update=1;
					}
					break;
		default: 
				break;
	}


}


unsigned char wbs_display_mode=0;
unsigned int wbs_i=0;
void WBS_default(void){
	unsigned int c_time=0;
	stop_beep_mode=STOP_BEEP_SHORT;
	switch(wbs_display_mode){
		case 0:	if((old_second!=I_second)||(display_update)){
					c_time=I_minute+I_hour*60;
					I_COLON_MODE=COLON_OFF;
					if((I_dow==1)||(I_dow==2)||(I_dow==3)||(I_dow==4)){
						fill_time();
						I_SEG_MODE=SEG_BRIGHT;
						if(	((I_minute==59)&&(I_hour==7))||
							((I_minute==44)&&(I_hour==9))||
							((I_minute==29)&&(I_hour==11))||
							((I_minute==29)&&(I_hour==13))
						   )
						{
							I_SEG_MODE=SEG_ZOOM;
						}else{
							I_SEG_MODE=SEG_BRIGHT;
						}
						if((c_time>=8*60+00)&&(c_time<9*60+30)){
							I_digits[0]=L_S;
							I_digits[1]=12;
							I_digits[2]=(9*60+30-c_time)/10;
							I_digits[3]=(9*60+30-c_time)%10;
							show_dot_flag=0;
						}else if((c_time>=9*60+30)&&(c_time<9*60+45)){
							I_digits[0]=L_P;
							I_digits[1]=12;
							I_digits[2]=(9*60+45-c_time)/10;
							I_digits[3]=(9*60+45-c_time)%10;
							show_dot_flag=0;
						}else if((c_time>=9*60+45)&&(c_time<11*60+15)){
							I_digits[0]=L_S;
							I_digits[1]=12;
							I_digits[2]=(11*60+15-c_time)/10;
							I_digits[3]=(11*60+15-c_time)%10;
							show_dot_flag=0;
						}else if((c_time>=11*60+15)&&(c_time<11*60+30)){
							I_digits[0]=L_P;
							I_digits[1]=12;
							I_digits[2]=(11*60+30-c_time)/10;
							I_digits[3]=(11*60+30-c_time)%10;
							show_dot_flag=0;
						}else if((c_time>=11*60+30)&&(c_time<13*60+00)){
							I_digits[0]=L_S;
							I_digits[1]=12;
							I_digits[2]=(13*60+00-c_time)/10;
							I_digits[3]=(13*60+00-c_time)%10;
							show_dot_flag=0;
						}else if((c_time>=13*60+00)&&(c_time<13*60+30)){
							I_digits[0]=L_P;
							I_digits[1]=12;
							I_digits[2]=(13*60+30-c_time)/10;
							I_digits[3]=(13*60+30-c_time)%10;
							show_dot_flag=0;
						}else if((c_time>=13*60+30)&&(c_time<15*60+00)){
							I_digits[0]=L_S;
							I_digits[1]=12;
							I_digits[2]=(15*60+00-c_time)/10;
							I_digits[3]=(15*60+00-c_time)%10;
							show_dot_flag=0;
						}
					}else if(I_dow==5){
						fill_time();
						I_SEG_MODE=SEG_BRIGHT;
						if(	((I_minute==59)&&(I_hour==7))||
							((I_minute==44)&&(I_hour==9))||
							((I_minute==29)&&(I_hour==11))
						   )
						{
							I_SEG_MODE=SEG_ZOOM;
						}else{
							I_SEG_MODE=SEG_BRIGHT;
						}
						if((c_time>=8*60+00)&&(c_time<9*60+30)){
							I_digits[0]=L_S;
							I_digits[1]=12;
							I_digits[2]=(9*60+30-c_time)/10;
							I_digits[3]=(9*60+30-c_time)%10;
							show_dot_flag=0;
						}else if((c_time>=9*60+30)&&(c_time<9*60+45)){
							I_digits[0]=L_P;
							I_digits[1]=12;
							I_digits[2]=(9*60+45-c_time)/10;
							I_digits[3]=(9*60+45-c_time)%10;
							show_dot_flag=0;
						}else if((c_time>=9*60+45)&&(c_time<11*60+15)){
							I_digits[0]=L_S;
							I_digits[1]=12;
							I_digits[2]=(11*60+15-c_time)/10;
							I_digits[3]=(11*60+15-c_time)%10;
							show_dot_flag=0;
						}else if((c_time>=11*60+15)&&(c_time<11*60+30)){
							I_digits[0]=L_P;
							I_digits[1]=12;
							I_digits[2]=(11*60+30-c_time)/10;
							I_digits[3]=(11*60+30-c_time)%10;
							show_dot_flag=0;
						}else if((c_time>=11*60+30)&&(c_time<13*60+00)){
							I_digits[0]=L_S;
							I_digits[1]=12;
							I_digits[2]=(13*60+00-c_time)/10;
							I_digits[3]=(13*60+00-c_time)%10;
							show_dot_flag=0;
						}
					}else{//weekends
						fill_time();
						I_SEG_MODE=SEG_BRIGHT;
					}
					display_update=0;
					if(((I_second%10==0)||((I_second+1)%10==0))&&(1)){//use precalculated values

					}else{//default display time all days in all modes
						fill_time();
					}
					old_second=I_second;
				}
				show_dcf77_signal_dot();
				if((I_hour>=20)||(I_hour<7)){//energy save mode between 20:00-07:00
					I_SEG_MODE=SEG_DIM;
				}
				if(check_schedule()==0){
					switch(get_ir_code()){
						case IR_MUTE:	wbs_display_mode=1;I_COLON_MODE=COLON_OFF;show_dot_flag=0;
										break;
						case IR_CH_PLUS: 	wbs_display_mode=50;I_COLON_MODE=COLON_OFF;show_dot_flag=0;
											break;
						case IR_CH_MINUS: wbs_display_mode=60;I_COLON_MODE=COLON_OFF;show_dot_flag=0;
											break;
						case IR_POWER: 		if(I2C_MP3_detected){
												I2C_MP3_talkTime();
											}
											break;
					}
				}
				break;

		case 1:	wbs_i=code_input();
				if(wbs_i==0){//code insert canceld
					wbs_display_mode=0;display_update=1;
				}else if(wbs_i==1){//code is still entered
				}else if(wbs_i==1306){//code correctly entered
					wbs_display_mode=2;
				}
				break;

		case 2:	wbs_i=main_menu_input();
				if(wbs_i==0){//menu canceled
					wbs_display_mode=0;display_update=1;
				}else if(wbs_i==2){//new show mode
					wbs_display_mode=0;display_update=1;
				}
				break;

		case 50:	start_stop_watch();
					fill_temp();
					wbs_display_mode=51;
					break;
		case 51:	fill_temp();
					if(get_stop_watch()*4>2000){//2seconds waiting
						stop_stop_watch();
						display_update=1;
						wbs_display_mode=0;display_update=1;
					}
					break;
		case 60:	start_stop_watch();
					fill_date();
					wbs_display_mode=61;
					break;
		case 61:	if(get_stop_watch()*4>2000){//2seconds waiting
						stop_stop_watch();
						start_stop_watch();
						display_update=1;
						wbs_display_mode=62;
						fill_dow_year(1);
					}
					break;
		case 62:	if(get_stop_watch()*4>2000){//2seconds waiting
						stop_stop_watch();
						display_update=1;
						wbs_display_mode=0;display_update=1;
					}
					break;
		default: 
				break;
	}
}

unsigned char bin_display_mode=0;
unsigned int bin_i=0;
void bin_default(void){
	stop_beep_mode=STOP_BEEP_SHORT;
	switch(bin_display_mode){
		case 0:	if((old_second!=I_second)||(display_update)){
					I_SEG_MODE=SEG_BRIGHT;
					I_COLON_MODE=COLON_OFF;
					display_update=0;
					I_digits[0]=(I_hour/10)+L_b0;
					I_digits[1]=(I_hour%10)+L_b0;
					I_digits[2]=(I_minute/10)+L_b0;
					I_digits[3]=(I_minute%10)+L_b0;
					old_second=I_second;
				}
				show_dot_flag=1;
				show_dcf77_signal_dot();
				if((I_hour>=20)||(I_hour<7)){//energy save mode between 20:00-07:00
					I_SEG_MODE=SEG_DIM;
				}
				if(check_schedule()==0){
					switch(get_ir_code()){
						case IR_MUTE:	bin_display_mode=1;show_dot_flag=0;
										break;
						case IR_POWER: 		if(I2C_MP3_detected){
												I2C_MP3_talkTime();
											}
											break;
					}
				}
				break;

		case 1:	bin_i=code_input();
				if(bin_i==0){//code insert canceld
					bin_display_mode=0;display_update=1;
				}else if(bin_i==1){//code is still entered
				}else if(bin_i==1306){//code correctly entered
					bin_display_mode=2;
				}
				break;

		case 2:	bin_i=main_menu_input();
				if(bin_i==0){//menu canceled
					bin_display_mode=0;display_update=1;
				}else if(bin_i==2){//new show mode
					bin_display_mode=0;display_update=1;
				}
				break;

		default: 
				break;
	}
}

unsigned char te_display_mode=0;
unsigned int te_i=0;
void TE_default(void){
	stop_beep_mode=STOP_BEEP_SHORT;
	unsigned int c_time=0;
	switch(te_display_mode){
		case 0:	if((old_second!=I_second)||(display_update)){
					I_SEG_MODE=SEG_BRIGHT;
					I_COLON_MODE=COLON_OFF;
					c_time=I_minute+I_hour*60;
					if((I_dow==1)||(I_dow==2)||(I_dow==3)||(I_dow==4)){
						if((c_time>=7*60+00)&&(c_time<9*60+00)){
							I_digits[0]=12;
							I_digits[1]=(9*60+00-c_time)/100;
							I_digits[2]=((9*60+00-c_time)%100)/10;
							I_digits[3]=((9*60+00-c_time)%100)%10;
							show_dot_flag=0;
						}else if((c_time>=9*60+00)&&(c_time<9*60+20)){
							I_digits[0]=L_P;
							I_digits[1]=12;
							I_digits[2]=(9*60+20-c_time)/10;
							I_digits[3]=(9*60+20-c_time)%10;
							show_dot_flag=0;
						}else if((c_time>=9*60+20)&&(c_time<12*60+00)){
							I_digits[0]=12;
							I_digits[1]=(12*60+00-c_time)/100;
							I_digits[2]=((12*60+00-c_time)%100)/10;
							I_digits[3]=((12*60+00-c_time)%100)%10;
							show_dot_flag=0;
						}else if((c_time>=12*60+00)&&(c_time<12*60+40)){
							I_digits[0]=L_P;
							I_digits[1]=12;
							I_digits[2]=(12*60+40-c_time)/10;
							I_digits[3]=(12*60+40-c_time)%10;
							show_dot_flag=0;
						}else if((c_time>=12*60+40)&&(c_time<15*60+00)){
							I_digits[0]=12;
							I_digits[1]=(15*60+00-c_time)/100;
							I_digits[2]=((15*60+00-c_time)%100)/10;
							I_digits[3]=((15*60+00-c_time)%100)%10;
							show_dot_flag=0;
						}else{//outside of TE blocks
							fill_time();
							I_SEG_MODE=SEG_BRIGHT;
						}
					}else if(I_dow==5){
						if((c_time>=7*60+00)&&(c_time<9*60+00)){
							I_digits[0]=12;
							I_digits[1]=(9*60+00-c_time)/100;
							I_digits[2]=((9*60+00-c_time)%100)/10;
							I_digits[3]=((9*60+00-c_time)%100)%10;
							show_dot_flag=0;
						}else if((c_time>=9*60+00)&&(c_time<9*60+20)){
							I_digits[0]=L_P;
							I_digits[1]=12;
							I_digits[2]=(9*60+45-c_time)/10;
							I_digits[3]=(9*60+45-c_time)%10;
							show_dot_flag=0;
						}else if((c_time>=9*60+20)&&(c_time<12*60+00)){
							I_digits[0]=12;
							I_digits[1]=(12*60+00-c_time)/100;
							I_digits[2]=((12*60+00-c_time)%100)/10;
							I_digits[3]=((12*60+00-c_time)%100)%10;
							show_dot_flag=0;
						}else if((c_time>=12*60+00)&&(c_time<12*60+40)){
							I_digits[0]=L_P;
							I_digits[1]=12;
							I_digits[2]=(12*60+40-c_time)/10;
							I_digits[3]=(12*60+40-c_time)%10;
							show_dot_flag=0;
						}else if((c_time>=12*60+40)&&(c_time<13*60+45)){
							I_digits[0]=12;
							I_digits[1]=(13*60+45-c_time)/100;
							I_digits[2]=((13*60+45-c_time)%100)/10;
							I_digits[3]=((13*60+45-c_time)%100)%10;
							show_dot_flag=0;
						}else{//outside of TE blocks
							fill_time();
							I_SEG_MODE=SEG_BRIGHT;
						}
					}else{//weekends
						fill_time();
						I_SEG_MODE=SEG_BRIGHT;
					}
					display_update=0;
					if(((I_second%15==0)||((I_second+1)%15==0)||((I_second+2)%15==0)||((I_second+3)%15==0))&&(1)){//use precalculated values
						if(((I_second%15==0)||((I_second+1)%15==0))){
							fill_date();
						}
					}else{//default display time all days in all modes
						fill_time();
					}
					old_second=I_second;
				}
				show_dcf77_signal_dot();
				if((I_hour>=19)||(I_hour<6)){//energy save mode between 19:00-05:59
					I_SEG_MODE=SEG_DIM;
				}
				if(check_schedule()==0){
					switch(get_ir_code()){
						case IR_MUTE:	te_display_mode=1;I_COLON_MODE=COLON_OFF;show_dot_flag=0;
										break;
						case IR_CH_PLUS: 	te_display_mode=50;I_COLON_MODE=COLON_OFF;show_dot_flag=0;
											break;
						case IR_CH_MINUS: te_display_mode=60;I_COLON_MODE=COLON_OFF;show_dot_flag=0;
											break;
						case IR_POWER: 		if(I2C_MP3_detected){
												I2C_MP3_talkTime();
											}
											break;
					}
				}
				break;

		case 1:	te_i=code_input();
				if(te_i==0){//code insert canceld
					te_display_mode=0;display_update=1;
				}else if(te_i==1){//code is still entered
				}else if(te_i==1306){//code correctly entered
					te_display_mode=2;
				}
				break;

		case 2:	te_i=main_menu_input();
				if(te_i==0){//menu canceled
					te_display_mode=0;display_update=1;
				}else if(te_i==2){//new show mode
					te_display_mode=0;display_update=1;
				}
				break;

		case 50:	start_stop_watch();
					fill_temp();
					te_display_mode=51;
					break;
		case 51:	fill_temp();
					if(get_stop_watch()*4>2000){//2seconds waiting
						stop_stop_watch();
						display_update=1;
						te_display_mode=0;display_update=1;
					}
					break;
		case 60:	start_stop_watch();
					fill_date();
					te_display_mode=61;
					break;
		case 61:	if(get_stop_watch()*4>2000){//2seconds waiting
						stop_stop_watch();
						start_stop_watch();
						display_update=1;
						te_display_mode=62;
						fill_dow_year(1);
					}
					break;
		case 62:	if(get_stop_watch()*4>2000){//2seconds waiting
						stop_stop_watch();
						display_update=1;
						te_display_mode=0;display_update=1;
					}
					break;
		default: 
				break;
	}
}

unsigned char c1_display_mode=0;
unsigned int c1_i=0;
void c1_default(void){
	stop_beep_mode=STOP_BEEP_SHORT;
	unsigned int c_time=0;

	switch(c1_display_mode){
		case 0:	if((old_second!=I_second)||(display_update)){
					c_time=I_minute+I_hour*60;
					fill_time();
					I_SEG_MODE=SEG_DIM;
					if(I_dow<=5){
						//default clock: dimmed but every hour for one minute fully on
						if(I_minute==0){
							I_SEG_MODE=SEG_BRIGHT;
						}else{
							I_SEG_MODE=SEG_DIM;
						}
					}else{//weekends
					}
					display_update=0;
					old_second=I_second;
				}
				if(check_schedule()==0){
					switch(get_ir_code()){
						case IR_MUTE:	c1_display_mode=1;I_COLON_MODE=COLON_OFF;show_dot_flag=0;
										break;
						case IR_CH_PLUS: 	c1_display_mode=50;I_COLON_MODE=COLON_OFF;show_dot_flag=0;
											break;
						case IR_CH_MINUS: c1_display_mode=60;I_COLON_MODE=COLON_OFF;show_dot_flag=0;
											break;
						case IR_POWER: 		if(I2C_MP3_detected){
												I2C_MP3_talkTime();
											}
											break;
					}
				}
				break;

		case 1:	c1_i=code_input();
				if(c1_i==0){//code insert canceld
					c1_display_mode=0;display_update=1;
				}else if(c1_i==1){//code is still entered
				}else if(c1_i==1306){//code correctly entered
					c1_display_mode=2;
				}
				break;

		case 2:	c1_i=main_menu_input();
				if(c1_i==0){//menu canceled
					c1_display_mode=0;display_update=1;
				}else if(c1_i==2){//new show mode
					c1_display_mode=0;display_update=1;
				}
				break;

		case 50:	start_stop_watch();
					fill_temp();
					c1_display_mode=51;
					break;
		case 51:	fill_temp();
					if(get_stop_watch()*4>2000){//2seconds waiting
						stop_stop_watch();
						display_update=1;
						c1_display_mode=0;display_update=1;
					}
					break;
		case 60:	start_stop_watch();
					fill_date();
					c1_display_mode=61;
					break;
		case 61:	if(get_stop_watch()*4>2000){//2seconds waiting
						stop_stop_watch();
						start_stop_watch();
						display_update=1;
						c1_display_mode=62;
						fill_dow_year(1);
					}
					break;
		case 62:	if(get_stop_watch()*4>2000){//2seconds waiting
						stop_stop_watch();
						display_update=1;
						c1_display_mode=0;display_update=1;
					}
					break;
		default: 
				break;
	}

}

unsigned char C2_display_mode=0;
unsigned int C2_i=0;
void C2_default(void){
	stop_beep_mode=STOP_BEEP_LONG;
	switch(C2_display_mode){
		case 0:	if((old_second!=I_second)||(display_update)){
					cli();
					I_COLON_MODE=COLON_OFF;
					if((I_hour>=22)||(I_hour<=7)){
						I_SEG_MODE=SEG_DIM;
					}else{
						I_SEG_MODE=SEG_BRIGHT;
					}
					if((I_second%20>=0)&&(I_second%20<=9)){
						fill_time();
					}else if((I_second%20>=10)&&(I_second%20<=14)){
						fill_date();
					}else if((I_second%20>=15)&&(I_second%20<=20)){
						fill_temp();
						show_dot_flag=0;
					}
					sei();
					display_update=0;
					old_second=I_second;
				}
				show_dcf77_signal_dot();
				if(check_schedule()==0){
					switch(get_ir_code()){
						case IR_MUTE:	C2_display_mode=1;show_dot_flag=0;
										I_COLON_MODE=COLON_OFF;
										break;
						case IR_POWER: 		if(I2C_MP3_detected){
												I2C_MP3_talkTime();
											}
											break;
					}
				}
				break;
		case 1:	C2_i=code_input();
				if(C2_i==0){//code insert canceled
					C2_display_mode=0;display_update=1;
				}else if(C2_i==1){//code is still entered
				}else if(C2_i==1306){//code correctly entered
					C2_display_mode=2;
				}
				break;

		case 2:	C2_i=main_menu_input();
				if(C2_i==0){//menu canceled
					C2_display_mode=0;display_update=1;
				}else if(C2_i==2){//new show mode
					C2_display_mode=0;display_update=1;
				}
				break;
		default: 
				break;
	}

}


unsigned char C3_display_mode=0;
unsigned int C3_i=0;
void C3_default(void){
	stop_beep_mode=STOP_BEEP_SHORT;
	switch(C3_display_mode){
		case 0:	if((old_second!=I_second)||(display_update)){
					cli();
					if((I_hour>=22)||(I_hour<=7)){
						I_SEG_MODE=SEG_DIM;
					}else{
						I_SEG_MODE=SEG_BRIGHT;
					}
					fill_time();
					I_COLON_MODE=COLON_OFF;
					sei();
					display_update=0;
					show_dot_flag=1;
					old_second=I_second;
				}
				show_dcf77_signal_dot();
				if(check_schedule()==0){
					switch(get_ir_code()){
						case IR_MUTE:	C3_display_mode=2;show_dot_flag=0;
										I_COLON_MODE=COLON_OFF;
										break;
						case IR_CH_PLUS: 	C3_display_mode=50;I_COLON_MODE=COLON_OFF;show_dot_flag=0;
											break;
						case IR_CH_MINUS: C3_display_mode=60;I_COLON_MODE=COLON_OFF;show_dot_flag=0;
											break;
						case IR_POWER: 		if(I2C_MP3_detected){
												I2C_MP3_talkTime();
											}
											break;
					}
				}
				break;
		case 1:	C3_i=code_input();
				if(C3_i==0){//code insert canceled
					C3_display_mode=0;display_update=1;
				}else if(C3_i==1){//code is still entered
				}else if(C3_i==1306){//code correctly entered
					C3_display_mode=2;
				}
				break;

		case 2:	C3_i=main_menu_input();
				if(C3_i==0){//menu canceled
					C3_display_mode=0;display_update=1;
				}else if(C3_i==2){//new show mode
					C3_display_mode=0;display_update=1;
				}
				break;
		case 50:	start_stop_watch();
					fill_temp();
					C3_display_mode=51;
					break;
		case 51:	fill_temp();
					if(get_stop_watch()*4>2000){//2seconds waiting
						stop_stop_watch();
						display_update=1;
						C3_display_mode=0;display_update=1;
					}
					break;
		case 60:	start_stop_watch();
					fill_date();
					C3_display_mode=61;
					break;
		case 61:	if(get_stop_watch()*4>2000){//2seconds waiting
						stop_stop_watch();
						start_stop_watch();
						display_update=1;
						C3_display_mode=62;
						fill_dow_year(0);
					}
					break;
		case 62:	if(get_stop_watch()*4>2000){//2seconds waiting
						stop_stop_watch();
						display_update=1;
						C3_display_mode=0;display_update=1;
					}
					break;
		default: 
				break;
	}

}



#define CORR_TEMP -55
unsigned long ad_res=0;
unsigned char simple_display_mode=0;
unsigned int simple_i=0;
void simple_default(void){
	stop_beep_mode=STOP_BEEP_SHORT;
		switch(simple_display_mode){
			case 0:	if((old_second!=I_second)||(display_update)){
						display_update=0;
						I_COLON_MODE=COLON_OFF;
						if(I_second%10==0){
							fill_temp();
							show_dot_flag=0;
						}else{
							fill_time();
						}
						if((I_hour>=22)&&(I_hour<7)){
							I_SEG_MODE=SEG_DIM;
						}else{
							I_SEG_MODE=SEG_BRIGHT;
						}
						old_second=I_second;
					}
					show_dcf77_signal_dot();
					if(check_schedule()==0){
						switch(get_ir_code()){
							case IR_MUTE:	simple_display_mode=1;I_COLON_MODE=COLON_OFF;show_dot_flag=0;
											break;
							case IR_POWER: 		if(I2C_MP3_detected){
													I2C_MP3_talkTime();
												}
												break;
						}
					}
				break;

		case 1:	simple_i=code_input();
				if(simple_i==0){//code insert canceld
					simple_display_mode=0;display_update=1;
				}else if(simple_i==1){//code is still entered
				}else if(simple_i==1306){//code correctly entered
					simple_display_mode=2;
				}
				break;
		case 2:	simple_i=main_menu_input();
				if(simple_i==0){//menu canceled
					simple_display_mode=0;display_update=1;
				}else if(simple_i==2){//new show mode
					simple_display_mode=0;display_update=1;
				}
				break;
	}
}

unsigned char version_display_mode=0;
//0 version display finished
//1 still busy
unsigned char version_default(void){
	switch(version_display_mode){
		case 0:	I_digits[0]=L_v;
				I_digits[1]=VERSION_1;
				I_digits[2]=VERSION_2;
				I_digits[3]=VERSION_3;
				stop_stop_watch();
				start_stop_watch();
				version_display_mode++;
				break;
		case 1:	if(get_stop_watch()*4>2500){//2.5seconds waiting
					stop_stop_watch();
					start_stop_watch();
					display_update=1;
					I_SEG_MODE=SEG_DIM;
					version_display_mode=2;
				}
				break;
		case 2:	if(get_stop_watch()*4>2500){//2.5seconds waiting
					stop_stop_watch();
					display_update=1;
					version_display_mode=0;
					I_SEG_MODE=SEG_BRIGHT;
					return 0;
				}
				break;
		default: break;
	}
	switch(get_ir_code()){
			case IR_MUTE:	if(segment_mode==0){
								segment_mode=1;
							}else{
								segment_mode=0;
							}
							eeprom_write_byte ((uint8_t*)16, segment_mode);
							break;
			case IR_POWER:	if(fixed_mode==0){
								fixed_mode=1;
							}else{
								fixed_mode=0;
							}
							eeprom_write_byte ((uint8_t*)17, fixed_mode);
							break;
			case IR_CH_MINUS:
							if(dcf77_inverted_flag==0){
								dcf77_inverted_flag=0x80;
							}else{
								dcf77_inverted_flag=0;
							}
							eeprom_write_byte ((uint8_t*)18, dcf77_inverted_flag);
							break;
	}
	if(fixed_mode==0){
		I_digits[3]&=~0x80;
	}else{
		I_digits[3]|=0x80;
	}
	if(dcf77_inverted_flag==0){
		I_digits[0]&=~0x80;
	}else{
		I_digits[0]|=0x80;
	}
	return 1;
}

#endif