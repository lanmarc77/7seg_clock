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

#ifndef CLOCK_C
#define CLOCK_C
#include "config.h"
#include "ui_display_modes.h"
#include "clock.h"
#include "i2c_modules.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "TWI_Master.h"
#include "usart.h"



unsigned char INT_winter_flag=0;
unsigned char INT_dst_mode=CLOCK_DST_MODE_OFF;

void clock_set_dst_mode(unsigned char mode){
	if(mode>CLOCK_DST_MODE_US){
		mode=CLOCK_DST_MODE_OFF;
	}
	INT_dst_mode=mode;
}

unsigned char clock_get_dst_mode(void){
	return INT_dst_mode;
}



unsigned int refresh_times[5]={0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF};


unsigned int clock_get_last_refresh(unsigned char source){
	return refresh_times[source];
}
//volatile unsigned int no_dcf_signal=(unsigned int)3600*12;
unsigned char max_days[2][12]={{31,29,31,30,31,30,31,31,30,31,30,31},{31,28,31,30,31,30,31,31,30,31,30,31}};
//signed int last_diff=0;
volatile unsigned char hour=0,min=0,second=0,day=1,month=1,year=0,dow=1;
volatile unsigned char new_time_flag=0;

volatile unsigned char INT_hour=0;//0
volatile unsigned char INT_minute=0;//0
volatile unsigned char INT_second=0;//0
volatile unsigned char INT_day=1;//1
volatile unsigned char INT_month=1;//1
volatile unsigned char INT_year=0;//0
volatile unsigned char INT_dow=6;//6
volatile unsigned char debug_char='0';



volatile signed int stop_watch_counter=-1;
volatile signed int paused_save_value=-1;

//unsigned char I_second,I_hour,I_minute,I_year=14,I_dow=1,I_month,I_day;




volatile signed int timer2_counter=0;




signed int clock_get_stop_watch(void){
	unsigned int tmp=0;
	cli();
	tmp=stop_watch_counter;
	sei();
	return tmp;
}


void clock_start_stop_watch(void){
	cli();
	stop_watch_counter=0;
	sei();
}

void clock_stop_stop_watch(void){
	cli();
	stop_watch_counter=-1;
	sei();
}


void clock_pause_stop_watch(void){
	cli();
	paused_save_value=stop_watch_counter;
	stop_watch_counter=-1;
	sei();
}

void clock_unpause_stop_watch(void){
	cli();
	stop_watch_counter=paused_save_value;
	sei();
}

void send_usart_time(void){
	usart_send_char((INT_day/10)+48);usart_send_char((INT_day%10)+48);
	usart_send_char((INT_month/10)+48);usart_send_char((INT_month%10)+48);
	usart_send_char((INT_year/10)+48);usart_send_char((INT_year%10)+48);
	usart_send_char((INT_dow%10)+48);
	usart_send_char((INT_hour/10)+48);usart_send_char((INT_hour%10)+48);
	usart_send_char((INT_minute/10)+48);usart_send_char((INT_minute%10)+48);
	usart_send_char((INT_second/10)+48);usart_send_char((INT_second%10)+48);
	usart_send_char('-');usart_send_char(debug_char);

	usart_send_char(13);usart_send_char(10);
}

void clock_set_time(unsigned char t_min,unsigned char t_hour,unsigned char t_second,unsigned char t_day,unsigned char t_month,unsigned char t_year,unsigned char t_dow,unsigned char source){
	min=t_min;
	hour=t_hour;
	day=t_day;
	dow=t_dow;
	month=t_month;
	year=t_year;
	second=t_second;
	new_time_flag=source;
}

void clock_isr_part(void){
	unsigned char temp=0;
	if(new_time_flag){
		refresh_times[new_time_flag]=0;
		if(INT_dst_mode==CLOCK_DST_MODE_EU){
			if((INT_month==10)&&(INT_dow==7)&&(INT_day>=25)&&(INT_minute>30)&&(hour==2)&&(INT_hour==2)&&(new_time_flag!=CLOCK_UPDATE_SOURCE_RTC)){//someone else seems to set winter time
				INT_winter_flag=1;
			}
		}else if(INT_dst_mode==CLOCK_DST_MODE_US){
			if((INT_month==11)&&(INT_dow==7)&&(INT_day<=7)&&(INT_minute>30)&&(hour==1)&&(INT_hour==1)&&(new_time_flag!=CLOCK_UPDATE_SOURCE_RTC)){//someone else seems to set winter time
				INT_winter_flag=1;
			}
		}
		INT_second=second;
		INT_minute=min;
		INT_hour=hour;
		INT_dow=dow;
		INT_day=day;
		INT_month=month;
		INT_year=year;
		if((I2C_RTC_detected)&&(new_time_flag!=CLOCK_UPDATE_SOURCE_RTC)){
			I2C_RTC_setTime();
		}
		new_time_flag=0;
		timer2_counter=0;
		send_usart_time();
	}else{
		timer2_counter++;
	}
	if(stop_watch_counter>=0){
		stop_watch_counter++;
	}
	if(timer2_counter==250){
		timer2_counter=0;
		INT_second++;
		if(INT_second>=60){
			INT_second=0;
			INT_minute++;
			if(refresh_times[CLOCK_UPDATE_SOURCE_DCF77]<0xFFFF){
				refresh_times[CLOCK_UPDATE_SOURCE_DCF77]++;
			}
			if(refresh_times[CLOCK_UPDATE_SOURCE_RTC]<0xFFFF){
				refresh_times[CLOCK_UPDATE_SOURCE_RTC]++;
			}
			if(refresh_times[CLOCK_UPDATE_SOURCE_MANUAL]<0xFFFF){
				refresh_times[CLOCK_UPDATE_SOURCE_MANUAL]++;
			}
			if(INT_minute>=60){
				INT_minute=0;
				INT_hour++;
				if(INT_dst_mode==CLOCK_DST_MODE_EU){
					if((INT_month==3)&&(INT_dow==7)){//march, sunday
						if((INT_day>=25)&&(INT_minute==0)&&(INT_hour==2)){//last sunday, 2:00
							INT_hour=3;
							if(I2C_RTC_detected){
								I2C_RTC_setTime();
							}
						}
					}else if((INT_month==10)&&(INT_dow==7)){//october, sunday
						if((INT_day>=25)&&(INT_minute==0)&&(INT_hour==3)){//last sunday, 3:00
							if(INT_winter_flag==0){
								INT_hour=2;
								INT_winter_flag=1;
								if(I2C_RTC_detected){
									I2C_RTC_setTime();
								}
							}
						}
					}else{
						INT_winter_flag=0;
					}
				}else if(INT_dst_mode==CLOCK_DST_MODE_US){
					if((INT_month==3)&&(INT_dow==7)){//march, sunday
						if((INT_day>=8)&&(INT_day<=14)&&(INT_minute==0)&&(INT_hour==2)){//second sunday, 2:00
							INT_hour=3;
							if(I2C_RTC_detected){
								I2C_RTC_setTime();
							}
						}
					}else if((INT_month==11)&&(INT_dow==7)){//november, sunday
						if((INT_day<=7)&&(INT_minute==0)&&(INT_hour==2)){//first sunday, 2:00
							if(INT_winter_flag==0){
								INT_hour=1;
								INT_winter_flag=1;
								if(I2C_RTC_detected){
									I2C_RTC_setTime();
								}
							}
						}
					}else{
						INT_winter_flag=0;
					}
				}
				if(INT_hour>=24){
					INT_hour=0;
					INT_day++;
					INT_dow++;
					if(INT_dow>=8) INT_dow=1;
					if((INT_year%4)==0){//ATTENTION not working for 2100/2200/2300, I sure hope no one uses old sw like this anymore by then...
						temp=max_days[0][INT_month-1];
					}else{
						temp=max_days[1][INT_month-1];
					}
					if(INT_day>temp){
						INT_day=1;
						INT_month++;
						if(INT_month>=13){
							INT_month=1;
							INT_year=(INT_year+1)%100;
						}
					}
				}
			}

		}else if(INT_second==30){
			if(I2C_RTC_detected){
				I2C_RTC_getTime();
			}
		}
		send_usart_time();
	}
}
void clock_get_time(unsigned char *t_min,unsigned char *t_hour,unsigned char *t_second,unsigned char *t_day,unsigned char *t_month,unsigned char *t_year,unsigned char *t_dow){
	cli();
	*t_min=INT_minute;
	*t_hour=INT_hour;
	*t_second=INT_second;
	*t_day=INT_day;
	*t_month=INT_month;
	*t_year=INT_year;
	*t_dow=INT_dow;
	sei();
}

void clock_init(void){
	if(I2C_RTC_detected){
		I2C_RTC_getTime();
	}
}

#endif