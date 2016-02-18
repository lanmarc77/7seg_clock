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
#include <avr/eeprom.h>
#include <avr/sleep.h>
#include "TWI_Master.h"
#include "usart.h"



volatile unsigned char correct_timer=11;
volatile unsigned char correct_counter=0;
volatile signed char correct_factor=-1;
volatile unsigned int no_dcf_signal=(unsigned int)3600*12;
unsigned char max_days[2][12]={{31,29,31,30,31,30,31,31,30,31,30,31},{31,28,31,30,31,30,31,31,30,31,30,31}};
signed int last_diff=0;
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

/*
	0=no signal
	1=signal found not synced
	2=signal found collecting data
	3=signal found and synced + collecting next minute
*/
volatile unsigned char dcf_state=0;


volatile unsigned char timer1_dcf_counter=0;

volatile signed int stop_watch_counter=-1;
volatile signed int paused_save_value=-1;

unsigned char I_second,I_hour,I_minute,I_year=14,I_dow=1,I_month,I_day;


volatile unsigned char dcf77_inverted_flag=0x00;
volatile unsigned char filtered_dcf77=0;
// I will use fixed point arithmetics with 5 decimals
volatile unsigned int decimal_offset = 10000;

volatile unsigned long smoothed = 0;

volatile signed int timer2_counter=0;

unsigned int dcf_quality=0;
unsigned char last_input=0;

void low_pass_filter(void) {
    // http://en.wikipedia.org/wiki/Low-pass_filter#Continuous-time_low-pass_filters

    const uint32_t input = ((((DCF77_PIN^dcf77_inverted_flag))>>7)&0x01) * decimal_offset;
	
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

void dcf77_isr_part(unsigned char dsd){
	unsigned char c=0;
	timer1_dcf_counter++;
	if((timer1_dcf_counter==15)&&(dsd==1)){//every 1ms
		timer1_dcf_counter=0;
		low_pass_filter();

		if((filtered_dcf77) == 0){//logic 0 input
			if(dcf_signal_last){//signal change high to low
				dcf_signal_0length=0;
				dcf_signal_last=0;
			}else{
				if(dcf_signal_0length>3000){//no change on line for too long
					bit_clock_state=0;dcf_state=0;dcf_repeat_counter=0;
					//usart_send_char('R');
				}else{
					dcf_state=1;
					dcf_signal_0length++;
				}
			}
		}else{
			if(!dcf_signal_last){//signal change low to high
				if((dcf_signal_0length>1700)&&(dcf_signal_0length<2000)&&(dcf_quality>1000)){
					if(bit_clock_state==3){
						min=min_temp;
						hour=hour_temp;
						day=day_temp;
						dow=dow_temp;
						month=mon_temp;
						year=year_temp;
						second=0;
						new_time_flag|=1;
					}
					bit_clock_cnt=0;bit_clock_state=1;
					min_temp=-1;hour_temp=-1;year_temp=-1;dow_temp=-1;mon_temp=-1;day_temp=-1;
				}else{
					if(bit_clock_state==3){
						bit_clock_cnt=0;bit_clock_state=0;dcf_state=0;
						min_temp=-1;hour_temp=-1;year_temp=-1;dow_temp=-1;mon_temp=-1;day_temp=-1;
					}
				}
				dcf_signal_1length=0;
				dcf_signal_last=1;
			}else{
				if(dcf_signal_1length>300){//no change on line for too long
					bit_clock_state=0;dcf_state=0;dcf_repeat_counter=0;
				}else{
					dcf_state=1;
					dcf_signal_1length++;
				}
			}
		}
		
		bit_clock_cnt++;
		if(bit_clock_cnt==1000){
			bit_clock_cnt=0;
		}


		switch(bit_clock_state){
			case 1: bit_cnt=0;bit_clock_state++;dcf_state=1;c=0;
			case 2:	dcf_state=2;
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
											}else if((((min_temp&0x0F)+((min_temp>>4)&0x0F)*10) >=0)&&(((min_temp&0x0F)+((min_temp>>4)&0x0F)*10) <=59) &&
											    (((hour_temp&0x0F)+((hour_temp>>4)&0x0F)*10)>=0)&&(((hour_temp&0x0F)+((hour_temp>>4)&0x0F)*10)<=23)&&
												(((day_temp&0x0F)+((day_temp>>4)&0x0F)*10)>=1)&&(((day_temp&0x0F)+((day_temp>>4)&0x0F)*10)<=31)&&
												(((mon_temp&0x0F)+((mon_temp>>4)&0x0F)*10)>=1)&&(((mon_temp&0x0F)+((mon_temp>>4)&0x0F)*10)<=12)&&
												(((year_temp&0x0F)+((year_temp>>4)&0x0F)*10)>=0)&&(((year_temp&0x0F)+((year_temp>>4)&0x0F)*10)<=99)&&
												(dow_temp>=1)&&(dow_temp<=7)){
												if((dow_temp==old_dow_temp)&&(day_temp==old_day_temp)&&(mon_temp==old_mon_temp)&&(year_temp==old_year_temp)){
													if( ((((min_temp&0x0F)+((min_temp>>4)&0x0F)*10)==old_min_temp+1)&&(((hour_temp&0x0F)+((hour_temp>>4)&0x0F)*10)==old_hour_temp))||
														( (min_temp==0)&&(old_min_temp==59)&&(((hour_temp&0x0F)+((hour_temp>>4)&0x0F)*10)==(old_hour_temp+1)%24)  )
													){
														dcf_repeat_counter++;
														old_min_temp=((min_temp&0x0F)+((min_temp>>4)&0x0F)*10);old_hour_temp=((hour_temp&0x0F)+((hour_temp>>4)&0x0F)*10);
														if(dcf_repeat_counter>=1){
															dcf_repeat_counter=1;
															bit_clock_state=3;
														}
													}else{
														dcf_repeat_counter=0;
													}
												}
												old_min_temp=((min_temp&0x0F)+((min_temp>>4)&0x0F)*10);old_hour_temp=((hour_temp&0x0F)+((hour_temp>>4)&0x0F)*10);old_dow_temp=dow_temp;old_day_temp=day_temp;old_mon_temp=mon_temp;old_year_temp=year_temp;
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
			case 3: dcf_state=3;
					break;
		}
	}else{
		if(timer1_dcf_counter==15){
			timer1_dcf_counter--;
		}
	}
}


signed int get_stop_watch(void){
	unsigned int tmp=0;
	cli();
	tmp=stop_watch_counter;
	sei();
	return tmp;
}


void start_stop_watch(void){
	cli();
	stop_watch_counter=0;
	sei();
}

void stop_stop_watch(void){
	cli();
	stop_watch_counter=-1;
	sei();
}


void pause_stop_watch(void){
	cli();
	paused_save_value=stop_watch_counter;
	stop_watch_counter=-1;
	sei();
}

void unpause_stop_watch(void){
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

ISR(TIMER0_COMP_vect){//every 4ms
	unsigned char temp=0;
	if(new_time_flag&1){
		new_time_flag=0;
		no_dcf_signal=0;
		last_diff=timer2_counter;
		INT_second=(second&0x0F)+10*((second>>4)&0x0F);
		INT_minute=(min&0x0F)+10*((min>>4)&0x0F);
		INT_hour=(hour&0x0F)+10*((hour>>4)&0x0F);
		INT_dow=(dow&0x0F)+10*((dow>>4)&0x0F);
		INT_day=(day&0x0F)+10*((day>>4)&0x0F);
		INT_month=(month&0x0F)+10*((month>>4)&0x0F);
		INT_year=(year&0x0F)+10*((year>>4)&0x0F);
		display_update|=1;
		if(I2C_RTC_detected){
			I2C_RTC_setTime();
		}
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
		if(no_dcf_signal<(unsigned int)3600*12){
			no_dcf_signal++;
			if(no_dcf_signal==(unsigned int)3600*12) display_update|=1;
		}
		correct_counter++;
		if(correct_counter==correct_timer){
			correct_counter=0;
			timer2_counter+=correct_factor;
		}
		INT_second++;
		if(INT_second>=60){
			INT_second=0;
			INT_minute++;
			if(INT_minute>=60){
				INT_minute=0;
				INT_hour++;
				if(INT_hour>=24){
					INT_hour=0;
					INT_day++;
					INT_dow++;
					if(INT_dow>=8) INT_dow=1;
					if((INT_year%4)==0){//ATTENTION not working for 2100/2200/2300, I am dead by then...
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





void get_time(void){
			I_second=INT_second;
			I_minute=INT_minute;
			I_hour=INT_hour;
			I_day=INT_day;
			I_month=INT_month;
			I_year=INT_year;
			I_dow=INT_dow;
}

void dcf77_init(void){
	//unsigned char i=0;
	//unsigned char messageBuf[25];
		
	//DCF signal input
	DDRC&=~0x80;
	//timer 0 internal clock
	OCR0=250-1;
	TCCR0=8|4;		//prescaler 256, CTC mode
	TIMSK|=2;		//enable compare match interrupt

/*	messageBuf[0]=(0x68<<TWI_ADR_BITS) | (FALSE<<TWI_READ_BIT);
	messageBuf[1]=0x0F;
	messageBuf[2]=0x00;
	TWI_Start_Transceiver_With_Data( messageBuf, 3 );
	do{sleep_mode();i++;if(i==0){break;}}while( TWI_Transceiver_Busy() );
	if ((!( TWI_statusReg.lastTransOK ))||(i==0)){
		//error occured
		//TWI_Get_State_Info( );   //check the error value/last TWI state and act accordingly, error codes are defined in the header
		TWI_Master_Stop();
	}else{
		cli();
		RTC_detected=1;
		RTC_cmd=RTC_get_time;
		display_update=1;
		sei();
	}
	i=0;
	*/
	if(I2C_RTC_detected){
		I2C_RTC_getTime();
	}
}

#endif