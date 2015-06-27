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

#ifndef SEG_FUNC_C
#define SEG_FUNC_C
#include "config.h"
#include "clock.h"
#include "7seg_func.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include "TWI_Master.h"



#define SEG_A 0x04
#define SEG_B 0x08
#define SEG_C 0x20
#define SEG_D 0x40
#define SEG_E 0x80
#define SEG_F 0x02
#define SEG_G 0x01
#define SEG_DP 0x10

#define SEG_PORT PORTB



unsigned char number_patterns[45]={	SEG_A|SEG_B|SEG_C|SEG_D|SEG_E|SEG_F, //0
										SEG_B|SEG_C, //1
										SEG_A|SEG_B|SEG_D|SEG_E|SEG_G, //2
										SEG_A|SEG_B|SEG_C|SEG_D|SEG_G, //3
										SEG_G|SEG_B|SEG_C|SEG_F, //4
										SEG_A|SEG_G|SEG_C|SEG_D|SEG_F, //5
										SEG_A|SEG_C|SEG_D|SEG_E|SEG_F|SEG_G, //6
										SEG_A|SEG_B|SEG_C, //7
										SEG_A|SEG_B|SEG_C|SEG_D|SEG_E|SEG_F|SEG_G, //8
										SEG_A|SEG_B|SEG_C|SEG_D|SEG_F|SEG_G, //9
										0, //nothing
										SEG_A|SEG_D|SEG_E|SEG_F, //C
										SEG_G, //-
										SEG_F|SEG_E|SEG_D, //L
										SEG_C|SEG_D|SEG_E|SEG_F|SEG_G, //b
										SEG_G|SEG_B|SEG_E|SEG_C|SEG_D, //d
										SEG_G|SEG_C|SEG_D|SEG_E, //o
										SEG_A|SEG_F|SEG_G|SEG_E|SEG_D, //E
										SEG_D, //_
										SEG_A|SEG_F|SEG_G|SEG_E, //F
										SEG_A|SEG_B|SEG_C|SEG_E|SEG_F|SEG_G, //A
										SEG_F|SEG_E, //I
										SEG_F|SEG_E|SEG_G|SEG_D, //t
										SEG_A|SEG_B|SEG_E|SEG_F|SEG_G, //P
										SEG_E|SEG_C|SEG_A, //M
										SEG_B|SEG_C|SEG_D|SEG_F|SEG_G, //Y
										SEG_C|SEG_E|SEG_G, //n
										SEG_E|SEG_G, //r
										SEG_F|SEG_B|SEG_D, //W
										SEG_B|SEG_C|SEG_E|SEG_F|SEG_G, //H
										SEG_B|SEG_C|SEG_D|SEG_E|SEG_F, //U
										SEG_C|SEG_E|SEG_D, //v
										0, //b0
										SEG_D, //b1
										SEG_C, //b2
										SEG_D|SEG_C, //b3
										SEG_B, //b4
										SEG_B|SEG_D, //b5
										SEG_B|SEG_C, //b6
										SEG_B|SEG_D|SEG_C, //b7
										SEG_A, //b8
										SEG_A|SEG_D, //b9
										SEG_G, //
										SEG_G, //
										SEG_G //
									};



volatile unsigned char digit_counter=0;
volatile unsigned char digits[4]={L_v,VERSION_1,VERSION_2,VERSION_3};
//0:normal
//1:inverted
unsigned char segment_mode=1;

volatile unsigned char br_counter=0;

#define PORT_COLON_ON PORTD|=0x80
#define PORT_COLON_OFF PORTD&=~0x80



volatile unsigned char dimm_value=40;
volatile unsigned char bright_value=0;
volatile unsigned char SEG_MODE=SEG_BRIGHT;
unsigned int flash_counter=0;

unsigned char zoom_counter=0;
unsigned char zoom_state=0;
unsigned char z_cnt=0;

unsigned char I_SEG_MODE=SEG_BRIGHT;
unsigned char I_COLON_MODE=COLON_OFF;
unsigned char I_BEEPER_MODE=BEEPER_OFF;
unsigned char I_digits[4]={0x88,0x88,0x88,0x88};


volatile unsigned char COLON_MODE=COLON_OFF;


volatile unsigned char BEEPER_MODE=BEEPER_OFF;
#define PORT_BEEPER_ON PORTC|=0x04
#define PORT_BEEPER_OFF PORTC&=~0x04
unsigned int beeper_counter=0;




void set_digit_pattern(unsigned char digit,unsigned char pattern){
	if(segment_mode==0){
		SEG_PORT=(pattern);
	}else{
		SEG_PORT=~(pattern);
	}
}

void enable_digit(unsigned char digit){
	if(segment_mode==0){
		PORTD&=~(1<<(digit+3));
	}else{
		PORTD|=(1<<(digit+3));
	}
}

void all_digits_off(void){
	if(segment_mode==0){
		PORTD|=0x78;
	}else{
		PORTD&=~0x78;
	}
}


unsigned char segments_ISR(void){
	unsigned char display_sync_dim=0;
	switch(BEEPER_MODE){
			case BEEPER_ON:		beeper_counter++;
								switch(beeper_counter){
									case 1: PORT_BEEPER_ON;break;
									case 500:	PORT_BEEPER_OFF;break;
									case 2001: PORT_BEEPER_ON;break;
									case 2500:	PORT_BEEPER_OFF;break;
									case 4001: PORT_BEEPER_ON;break;
									case 4500:	PORT_BEEPER_OFF;break;
									case 6001: PORT_BEEPER_ON;break;
									case 6500:	PORT_BEEPER_OFF;break;
									case 15000: beeper_counter=0;break;
								}
								break;
			case BEEPER_OFF:	PORT_BEEPER_OFF;beeper_counter=0;
								break;
		
			case BEEPER_ON_FULL: PORT_BEEPER_ON;beeper_counter=0;
								break;
	}
	switch(SEG_MODE){
		case SEG_BRIGHT:	all_digits_off();PORT_COLON_OFF;
							if(digit_counter<4){
								if(digits[digit_counter]&0x80){
									set_digit_pattern(digit_counter,number_patterns[(digits[digit_counter]&0x3F)]|SEG_DP);
								}else{
									set_digit_pattern(digit_counter,number_patterns[(digits[digit_counter]&0x3F)]);
								}
								enable_digit(digit_counter);
								if((digit_counter==0)&&(COLON_MODE==COLON_ON)){
									PORT_COLON_ON;
								}
								digit_counter++;
							}else{
								display_sync_dim=1;
								br_counter++;
								if(br_counter>bright_value){
									digit_counter=0;
									br_counter=0;
								}
							}
							break;
		case SEG_DIM:		all_digits_off();PORT_COLON_OFF;
							if(digit_counter<4){
								if(digits[digit_counter]&0x80){
									set_digit_pattern(digit_counter,number_patterns[(digits[digit_counter]&0x3F)]|SEG_DP);
								}else{
									set_digit_pattern(digit_counter,number_patterns[(digits[digit_counter]&0x3F)]);
								}
								if(dimm_value<250)enable_digit(digit_counter);
								if((digit_counter==0)&&(COLON_MODE==COLON_ON)){
									if(dimm_value<250)PORT_COLON_ON;
								}
								digit_counter++;
							}else{
								display_sync_dim=1;
								br_counter++;
								if(br_counter>dimm_value){
									digit_counter=0;
									br_counter=0;
								}
							}
					break;

		case SEG_FLICKER:	if(flash_counter>18){
									all_digits_off();PORT_COLON_OFF;
									if(digit_counter!=4){
										if(digits[digit_counter]&0x80){
											set_digit_pattern(digit_counter,number_patterns[(digits[digit_counter]&0x3F)]|SEG_DP);
										}else{
											set_digit_pattern(digit_counter,number_patterns[(digits[digit_counter]&0x3F)]);
										}
										enable_digit(digit_counter);
										if((digit_counter==0)&&(COLON_MODE==COLON_ON)){
											PORT_COLON_ON;
										}
										digit_counter++;
									}else{
										display_sync_dim=1;
										br_counter++;
										if(br_counter>bright_value){
											digit_counter=0;
											br_counter=0;
										}
									}
									flash_counter=0;
								}else{
									flash_counter++;
								}
					break;
		case SEG_DIM_FLICKER:	if(flash_counter>18){
									all_digits_off();PORT_COLON_OFF;
									if(digit_counter!=4){
										if(digits[digit_counter]&0x80){
											set_digit_pattern(digit_counter,number_patterns[(digits[digit_counter]&0x3F)]|SEG_DP);
										}else{
											set_digit_pattern(digit_counter,number_patterns[(digits[digit_counter]&0x3F)]);
										}
										enable_digit(digit_counter);
										if((digit_counter==0)&&(COLON_MODE==COLON_ON)){
											PORT_COLON_ON;
										}
										digit_counter++;
									}else{
										display_sync_dim=1;
										br_counter++;
										if(br_counter>dimm_value){
											digit_counter=0;
											br_counter=0;
										}
									}
									flash_counter=0;
								}else{
									flash_counter++;
								}
					break;
		case SEG_OFF:	all_digits_off();PORT_COLON_OFF;
					break;
		case SEG_FLASH:		if(flash_counter<2000){
								all_digits_off();PORT_COLON_OFF;
								if(digit_counter!=4){
									if(digits[digit_counter]&0x80){
										set_digit_pattern(digit_counter,number_patterns[(digits[digit_counter]&0x3F)]|SEG_DP);
									}else{
										set_digit_pattern(digit_counter,number_patterns[(digits[digit_counter]&0x3F)]);
									}
									enable_digit(digit_counter);
									if((digit_counter==0)&&(COLON_MODE==COLON_ON)){
										PORT_COLON_ON;
									}
									digit_counter++;
								}else{
									digit_counter=0;
								}
							}else{
								all_digits_off();PORT_COLON_OFF;
							}
							display_sync_dim=1;
							flash_counter++;
							if(flash_counter>5000){
								flash_counter=0;
							}
					break;
		case SEG_ZOOM:	all_digits_off();PORT_COLON_OFF;
						if(digit_counter<4){
							if(digits[digit_counter]&0x80){
								set_digit_pattern(digit_counter,number_patterns[(digits[digit_counter]&0x3F)]|SEG_DP);
							}else{
								set_digit_pattern(digit_counter,number_patterns[(digits[digit_counter]&0x3F)]);
							}
							enable_digit(digit_counter);
							if((digit_counter==0)&&(COLON_MODE==COLON_ON)){
								PORT_COLON_ON;
							}							
							digit_counter++;
						}else{
							display_sync_dim=1;
							br_counter++;
							if(br_counter>zoom_counter){
								digit_counter=0;
								br_counter=0;
							}
							z_cnt++;
							if(z_cnt>=50){
								z_cnt=0;
								switch(zoom_state){
									case 0:	zoom_counter+=1;
											if(zoom_counter>=80){
												zoom_state=1;
											}
											break;
									case 1:	zoom_counter-=1;
											if(zoom_counter==0){
												z_cnt=0;
												zoom_state=0;
											}
											break;
								}
							}
						}
					break;
	}
	return display_sync_dim;
}



void refresh_display_content(void){
	digits[0]=I_digits[0];
	digits[1]=I_digits[1];
	digits[2]=I_digits[2];
	digits[3]=I_digits[3];
	SEG_MODE=I_SEG_MODE;
	COLON_MODE=I_COLON_MODE;
	BEEPER_MODE=I_BEEPER_MODE;
}


void segments_init(void){
		//all leds off
		DDRD=0x78;
		PORTD=0x78;
		
		//PD7 output and default low=off for a fixed :
		DDRD|=0x80;
		PORT_COLON_OFF;
		
		//beeper port off by default
		PORTC&=~0x04;
		DDRC|=0x04;
		
		
		DDRB|=0xFF;
		PORTB=0xFF;
}


#endif