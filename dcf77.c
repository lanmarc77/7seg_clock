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

#ifndef DCF77_C
#define DCF77_C
#include "config.h"
#include "dcf77.h"
#include "clock.h"
#include <avr/io.h>
#include "usart.h"

/*
	0=no signal
	1=signal found not synced
	2=signal found collecting data
	3=signal found and synced + collecting next minute
*/
volatile unsigned char dcf_state=DCF77_STATE_NONE;


unsigned char dcf77_get_state(void){
	return dcf_state;
}

unsigned char dcf77_signal_type=DCF77_SIGNAL_TYPE_INVERTED;


volatile unsigned char filtered_dcf77=0;
extern unsigned char dcf77_get_signal(void){
	return filtered_dcf77;
}

// I will use fixed point arithmetics with 5 decimals
volatile unsigned int decimal_offset = 10000;

volatile unsigned long smoothed = 0;


unsigned int dcf_quality=0;
unsigned char last_input=0;


#define DCF77_PIN_NORMAL (PINC&0x80)
#define DCF77_PIN_INVERTED (PINC&0x80)^0x80

void low_pass_filter(void) {
    // http://en.wikipedia.org/wiki/Low-pass_filter#Continuous-time_low-pass_filters

    uint32_t input;
	if(dcf77_signal_type==DCF77_SIGNAL_TYPE_NORMAL){
		input = ((((DCF77_PIN_NORMAL))>>7)&0x01) * decimal_offset;
	}else{
		input = ((((DCF77_PIN_INVERTED))>>7)&0x01) * decimal_offset;
	}
	
	if((input==0)&&(last_input==1)){
		dcf_quality=0;
		last_input=0;
	}else if((input)&&(last_input==0)){
		last_input=1;
	}else if((input)&&(last_input==1)){
	}else{
		dcf_quality++;
	}

    // compute N such that the smoothed signal will always reach 50% of
    // the input after at most 50 samples (=50ms).
    // N = 1 / (1- 2^-(1/50)) = 72.635907286
    const uint16_t N = 72;
    smoothed = ((N-1) * smoothed + input) / N;

    // introduce some hysteresis
    static uint8_t square_wave_output = 0;

    if ((square_wave_output == 0) == (smoothed >= decimal_offset/2)) {
        // smoothed value more >= 50% away from output
        // ==> switch output
        square_wave_output = 1-square_wave_output;
        // ==> max the smoothed value in order to introduce some
        //     hysteresis, this also ensures that there is no
        //     "infinite memory"
        smoothed = square_wave_output? decimal_offset: 0;
    }
	filtered_dcf77=square_wave_output;
}


unsigned char bcd2dec(unsigned char c){
	return (c&0x0F)+((c>>4)&0x0F)*10;
}

unsigned char bit_clock_state=0;
unsigned char bit_cnt=0;
unsigned int bit_clock_cnt=0;
unsigned int dcf_signal_0length=0;
unsigned int dcf_signal_1length=0;
unsigned int dcf_signal_last=0;
unsigned char dcf_repeat_counter=0;

signed char min_temp=-1;signed char hour_temp=-1;
signed char mon_temp=-1;signed char dow_temp=-1;
signed char day_temp=-1;signed char year_temp=-1;

signed char old_min_temp=-1;signed char old_hour_temp=-1;
signed char old_mon_temp=-1;signed char old_dow_temp=-1;
signed char old_day_temp=-1;signed char old_year_temp=-1;

unsigned char p=0;

void dcf77_isr_part(void){
	unsigned char c=0;
//	timer1_dcf_counter++;
//	if((timer1_dcf_counter==15)&&(dsd==1)){//every 1ms
//		timer1_dcf_counter=0;
		low_pass_filter();

		if((filtered_dcf77) == 0){//logic 0 input
			if(dcf_signal_last){//signal change high to low
				dcf_signal_0length=0;
				dcf_signal_last=0;
			}else{
				if(dcf_signal_0length>3000){//no change on line for too long
					bit_clock_state=0;dcf_state=DCF77_STATE_NONE;dcf_repeat_counter=0;
					//usart_send_char('R');
				}else{
					dcf_state=DCF77_STATE_FOUND_NO_SYNC;
					dcf_signal_0length++;
				}
			}
		}else{
			if(!dcf_signal_last){//signal change low to high
				if((dcf_signal_0length>1700)&&(dcf_signal_0length<2000)&&(dcf_quality>1000)){
					if(bit_clock_state==3){
						clock_set_time(bcd2dec(min_temp),bcd2dec(hour_temp),0,bcd2dec(day_temp),bcd2dec(mon_temp),bcd2dec(year_temp),bcd2dec(dow_temp),CLOCK_UPDATE_SOURCE_DCF77);
						/*min=min_temp;
						hour=hour_temp;
						day=day_temp;
						dow=dow_temp;
						month=mon_temp;
						year=year_temp;
						second=0;
						new_time_flag|=1;*/
					}
					bit_clock_cnt=0;bit_clock_state=1;
					min_temp=-1;hour_temp=-1;year_temp=-1;dow_temp=-1;mon_temp=-1;day_temp=-1;
				}else{
					if(bit_clock_state==3){
						bit_clock_cnt=0;bit_clock_state=0;dcf_state=DCF77_STATE_NONE;
						min_temp=-1;hour_temp=-1;year_temp=-1;dow_temp=-1;mon_temp=-1;day_temp=-1;
					}
				}
				dcf_signal_1length=0;
				dcf_signal_last=1;
			}else{
				if(dcf_signal_1length>300){//no change on line for too long
					bit_clock_state=0;dcf_state=DCF77_STATE_NONE;dcf_repeat_counter=0;
				}else{
					dcf_state=DCF77_STATE_FOUND_NO_SYNC;
					dcf_signal_1length++;
				}
			}
		}
		
		bit_clock_cnt++;
		if(bit_clock_cnt==1000){
			bit_clock_cnt=0;
		}


		switch(bit_clock_state){
			case 1: bit_cnt=0;bit_clock_state++;dcf_state=DCF77_STATE_FOUND_NO_SYNC;c=0;
			case 2:	dcf_state=DCF77_STATE_FOUND_SYNC_1;
					if(bit_clock_cnt==500){
						if((bit_cnt>=21)&&(bit_cnt<=58)){
							if((dcf_signal_1length>=60)&&(dcf_signal_1length<=140)){
								if(c<2){
									c=0;
								}
							}else if((dcf_signal_1length>=160)&&(dcf_signal_1length<=240)){
								if(c<2){
									c=1;p^=1;
								}
							}else{
								c=2;
							}
							if(c<=1){
								switch(bit_cnt){
									case 21:min_temp=c;p=c;break;
									case 22:
									case 23:
									case 24:
									case 25:
									case 26:
									case 27:min_temp|=c<<(bit_cnt-21);break;
									case 28:if(p!=0){min_temp=-1;}break;
									case 29:hour_temp=c;p=c;break;
									case 30:
									case 31:
									case 32:
									case 33:
									case 34:hour_temp|=c<<(bit_cnt-29);break;
									case 35:if(p!=0){hour_temp=-1;}break;
									case 36:day_temp=c;p=c;break;
									case 37:
									case 38:
									case 39:
									case 40:
									case 41:day_temp|=c<<(bit_cnt-36);break;
									case 42:dow_temp=c;break;
									case 43:
									case 44:dow_temp|=c<<(bit_cnt-42);break;
									case 45:mon_temp=c;break;
									case 46:
									case 47:
									case 48:
									case 49:mon_temp|=c<<(bit_cnt-45);break;
									case 50:year_temp=c;break;
									case 51:
									case 52:
									case 53:
									case 54:
									case 55:
									case 56:
									case 57:year_temp|=c<<(bit_cnt-50);break;
									case 58:if(p!=0){day_temp=-1;dow_temp=-1;mon_temp=-1;year_temp=-1;}
											if(c==2){
												min_temp=-1;hour_temp=-1;year_temp=-1;dow_temp=-1;mon_temp=-1;day_temp=-1;dcf_repeat_counter=0;
												old_min_temp=-1;old_hour_temp=-1;old_year_temp=-1;old_dow_temp=-1;old_mon_temp=-1;old_day_temp=-1;
											}else /*if((((min_temp&0x0F)+((min_temp>>4)&0x0F)*10) >=0)&&(((min_temp&0x0F)+((min_temp>>4)&0x0F)*10) <=59) &&
											    (((hour_temp&0x0F)+((hour_temp>>4)&0x0F)*10)>=0)&&(((hour_temp&0x0F)+((hour_temp>>4)&0x0F)*10)<=23)&&
												(((day_temp&0x0F)+((day_temp>>4)&0x0F)*10)>=1)&&(((day_temp&0x0F)+((day_temp>>4)&0x0F)*10)<=31)&&
												(((mon_temp&0x0F)+((mon_temp>>4)&0x0F)*10)>=1)&&(((mon_temp&0x0F)+((mon_temp>>4)&0x0F)*10)<=12)&&
												(((year_temp&0x0F)+((year_temp>>4)&0x0F)*10)>=0)&&(((year_temp&0x0F)+((year_temp>>4)&0x0F)*10)<=99)&&
												(dow_temp>=1)&&(dow_temp<=7)){*/
												if(	(bcd2dec(min_temp) >=0)   &&    (bcd2dec(min_temp) <=59) &&
													(bcd2dec(hour_temp)>=0)   &&    (bcd2dec(hour_temp)<=23) &&
												    (bcd2dec(day_temp)>=1)    &&    (bcd2dec(day_temp)<=31)  &&
												    (bcd2dec(mon_temp)>=1)    &&    (bcd2dec(mon_temp)<=12)  &&
												    (bcd2dec(year_temp)>=0)   &&    (bcd2dec(year_temp)<=99) &&
												    (dow_temp>=1)             &&    (dow_temp<=7)
												  )													
												{
												
												/*if((dow_temp==old_dow_temp)&&(day_temp==old_day_temp)&&(mon_temp==old_mon_temp)&&(year_temp==old_year_temp)){
													if( ((((min_temp&0x0F)+((min_temp>>4)&0x0F)*10)==old_min_temp+1)&&(((hour_temp&0x0F)+((hour_temp>>4)&0x0F)*10)==old_hour_temp))||
														( (min_temp==0)&&(old_min_temp==59)&&(((hour_temp&0x0F)+((hour_temp>>4)&0x0F)*10)==(old_hour_temp+1)%24)  )
													){*/
												if((dow_temp==old_dow_temp)&&(day_temp==old_day_temp)&&(mon_temp==old_mon_temp)&&(year_temp==old_year_temp)){
													if( (((bcd2dec(min_temp)==old_min_temp+1))&&((bcd2dec(hour_temp)==old_hour_temp)))||
														( (min_temp==0)&&(old_min_temp==59)&&((bcd2dec(hour_temp)==(old_hour_temp+1)%24))  )
													){
														dcf_repeat_counter++;
														//old_min_temp=((min_temp&0x0F)+((min_temp>>4)&0x0F)*10);old_hour_temp=((hour_temp&0x0F)+((hour_temp>>4)&0x0F)*10);
														old_min_temp=bcd2dec(min_temp);old_hour_temp=bcd2dec(hour_temp);
														if(dcf_repeat_counter>=1){
															dcf_repeat_counter=1;
															bit_clock_state=3;
														}
													}else{
														dcf_repeat_counter=0;
													}
												}
												//old_min_temp=((min_temp&0x0F)+((min_temp>>4)&0x0F)*10);old_hour_temp=((hour_temp&0x0F)+((hour_temp>>4)&0x0F)*10);old_dow_temp=dow_temp;old_day_temp=day_temp;old_mon_temp=mon_temp;old_year_temp=year_temp;
												old_min_temp=bcd2dec(min_temp);old_hour_temp=bcd2dec(hour_temp);old_dow_temp=dow_temp;old_day_temp=day_temp;old_mon_temp=mon_temp;old_year_temp=year_temp;
											}else{
												min_temp=-1;hour_temp=-1;year_temp=-1;dow_temp=-1;mon_temp=-1;day_temp=-1;dcf_repeat_counter=0;
												old_min_temp=-1;old_hour_temp=-1;old_year_temp=-1;old_dow_temp=-1;old_mon_temp=-1;old_day_temp=-1;
											}
											break;
								}
							}
						}
						bit_cnt++;
						if(bit_cnt==60){
							bit_cnt=0;c=0;min_temp=-1;hour_temp=-1;year_temp=-1;dow_temp=-1;mon_temp=-1;day_temp=-1;
						}
					}
					break;
			case 3: dcf_state=DCF77_STATE_FOUND_SYNC_FULL;
					break;
		}
//	}else{
//		if(timer1_dcf_counter==15){
//			timer1_dcf_counter--;
//		}
//	}
}
void dcf77_set_signal_type(unsigned char c){
	if(c==DCF77_SIGNAL_TYPE_NORMAL){
		dcf77_signal_type=c;
	}else{
		dcf77_signal_type=DCF77_SIGNAL_TYPE_INVERTED;
	}
}

unsigned char dcf77_get_signal_type(void){
	return dcf77_signal_type;
}


void dcf77_init(void){

	//DCF signal input
	DDRC&=~0x80;
}

#endif
