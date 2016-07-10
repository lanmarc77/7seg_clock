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

#ifndef DISPLAY_7SEG_C
#define DISPLAY_7SEG_C
#include "config.h"
#include "clock.h"
#include "display_7seg.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include "TWI_Master.h"

#define DISPLAY_7SEG_COLON_OFF 0
#define DISPLAY_7SEG_COLON_ON 1

#define SEG_A 0x04
#define SEG_B 0x08
#define SEG_C 0x20
#define SEG_D 0x40
#define SEG_E 0x80
#define SEG_F 0x02
#define SEG_G 0x01
#define SEG_DP 0x10

#define SEG_PORT PORTB

const unsigned char number_patterns_ascii[75] = {	0, //b0
												SEG_D, //b1
												SEG_C, //b2
												SEG_D|SEG_C, //b3
												SEG_B, //b4
												SEG_B|SEG_D, //b5
												SEG_B|SEG_C, //b6
												SEG_B|SEG_D|SEG_C, //b7
												SEG_A, //b8
												SEG_A|SEG_D, //b9
												0, //space
												0,//!
												SEG_F|SEG_B,//"
												0,//#
												0,//$
												0,//%
												0,//&
												SEG_B,//'
												SEG_A|SEG_F|SEG_E|SEG_D,//(
												SEG_A|SEG_B|SEG_C|SEG_D,//)
												0,//*
												0,//+
												SEG_B,//´
												SEG_G,//-
												0,//.
												0,///
												SEG_A|SEG_B|SEG_C|SEG_D|SEG_E|SEG_F, //0
												SEG_B|SEG_C, //1
												SEG_A|SEG_B|SEG_D|SEG_E|SEG_G, //2
												SEG_A|SEG_B|SEG_C|SEG_D|SEG_G, //3
												SEG_G|SEG_B|SEG_C|SEG_F, //4
												SEG_A|SEG_G|SEG_C|SEG_D|SEG_F, //5
												SEG_A|SEG_C|SEG_D|SEG_E|SEG_F|SEG_G, //6
												SEG_A|SEG_B|SEG_C, //7
												SEG_A|SEG_B|SEG_C|SEG_D|SEG_E|SEG_F|SEG_G, //8
												SEG_A|SEG_B|SEG_C|SEG_D|SEG_F|SEG_G, //9
												0,//:
												0,//;
												0,//<
												SEG_D|SEG_B,//=
												0,//>
												0,//?
												0,//@
												SEG_A|SEG_B|SEG_C|SEG_E|SEG_F|SEG_G, //A
												SEG_C|SEG_D|SEG_E|SEG_F|SEG_G, //b
												SEG_A|SEG_D|SEG_E|SEG_F, //C
												SEG_G|SEG_B|SEG_E|SEG_C|SEG_D, //d
												SEG_A|SEG_F|SEG_G|SEG_E|SEG_D, //E
												SEG_A|SEG_F|SEG_G|SEG_E, //F
												SEG_A|SEG_B|SEG_C|SEG_D|SEG_F|SEG_G, //g
												SEG_B|SEG_C|SEG_E|SEG_F|SEG_G, //H
												SEG_F|SEG_E, //I
												SEG_B|SEG_C|SEG_D|SEG_E,//J
												SEG_F|SEG_E|SEG_G,//K
												SEG_F|SEG_E|SEG_D, //L
												SEG_E|SEG_C|SEG_A, //M
												SEG_C|SEG_E|SEG_G, //n
												SEG_G|SEG_C|SEG_D|SEG_E, //o
												SEG_A|SEG_B|SEG_E|SEG_F|SEG_G, //P
												SEG_A|SEG_F|SEG_G|SEG_B|SEG_C, //q
												SEG_E|SEG_G, //r
												SEG_A|SEG_G|SEG_C|SEG_D|SEG_F, //S
												SEG_F|SEG_E|SEG_G|SEG_D, //t
												SEG_B|SEG_C|SEG_D|SEG_E|SEG_F, //U
												SEG_C|SEG_E|SEG_D, //v
												SEG_F|SEG_B|SEG_D, //W
												SEG_B|SEG_C|SEG_E|SEG_F|SEG_G, //X
												SEG_B|SEG_C|SEG_D|SEG_F|SEG_G, //Y
												SEG_A|SEG_B|SEG_D|SEG_E|SEG_G, //Z
												SEG_A|SEG_F|SEG_E|SEG_D,//[
												0,//
												SEG_A|SEG_B|SEG_C|SEG_D,//]
												SEG_F|SEG_A|SEG_B,//^
												SEG_D, //_
												0//AVR GCC bug dummy
											};


unsigned char digit_counter=0;
unsigned char digits[4]={0,0,0,0};
unsigned char T_digits[4]={0,0,0,0};
unsigned char T_digits_state=0;
unsigned int T_digits_time_counter=0;
unsigned int T_digits_counter=3;

unsigned char anim_on=0;
unsigned char anim_mode=DISPLAY_7SEG_ANIM_MODE;

void display_7seg_anim_on(void){
	anim_on=1;
}

unsigned char segment_mode=DISPLAY_7SEG_POLARITY;
unsigned char br_counter=0;

#define PORT_COLON_ON PORTD|=0x80
#define PORT_COLON_OFF PORTD&=~0x80

unsigned char dimm_value=40;
unsigned char bright_value=0;


extern void display_7seg_set_bright_level(unsigned char c){
	if(c>250){
		c=0;
	}
	cli();
	bright_value=c;
	sei();
}

extern void display_7seg_set_dark_level(unsigned char c){
	if(c>250){
		c=40;
	}
	cli();
	dimm_value=c;
	sei();
}

extern unsigned char display_7seg_get_bright_level(void){
	return bright_value;
}

extern unsigned char display_7seg_get_dark_level(void){
	return dimm_value;
}

unsigned int flash_counter=0;

unsigned char zoom_counter=0;
unsigned char zoom_state=0;
unsigned char z_cnt=0;



unsigned char I_digits[4]={0x88,0x88,0x88,0x88};



unsigned char SEG_MODE=DISPLAY_7SEG_BRIGHT;
unsigned char I_SEG_MODE=DISPLAY_7SEG_BRIGHT;
extern void display_7seg_set_mode(unsigned char c){
	I_SEG_MODE=c;
}


unsigned char COLON_MODE=DISPLAY_7SEG_COLON_OFF;
unsigned char I_COLON_MODE=DISPLAY_7SEG_COLON_OFF;
extern void display_7seg_set_colon_mode(unsigned char c){
	if(c>DISPLAY_7SEG_COLON_ON){
		c=DISPLAY_7SEG_COLON_OFF;
	}
	I_COLON_MODE=c;
}


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

unsigned char cc(unsigned char c){
	if((c>=97)&&(c<=122)){
		return c-32;
	}
	if((c>=0)&&(c<10)){
		return c+22;
	}
	return c;
}


unsigned char new_display_data_flag=0;
void display_7seg_set_time(char *c){
	I_digits[0]=cc(c[0])-22;
	I_digits[1]=cc(c[2])-22;
	I_digits[2]=cc(c[4])-22;
	I_digits[3]=cc(c[6])-22;
	if(c[1]=='.'){
		I_digits[0]|=0x80;
	}
	if(c[3]=='.'){
		I_digits[1]|=0x80;
		display_7seg_set_colon_mode(DISPLAY_7SEG_COLON_OFF);
	}else if (c[3]==':'){
		display_7seg_set_colon_mode(DISPLAY_7SEG_COLON_ON);
	}else if (c[3]=='|'){
		I_digits[1]|=0x80;
		display_7seg_set_colon_mode(DISPLAY_7SEG_COLON_ON);
	}else if (c[3]==' '){
		display_7seg_set_colon_mode(DISPLAY_7SEG_COLON_OFF);
	}
	if(c[5]=='.'){
		I_digits[2]|=0x80;
	}
	if(c[7]=='.'){
		I_digits[3]|=0x80;
	}
	cli();
	new_display_data_flag=1;
	sei();
}

void display_7seg_set_text(char *c){
	I_digits[0]=cc(c[0])-22;
	I_digits[1]=cc(c[1])-22;
	I_digits[2]=cc(c[2])-22;
	I_digits[3]=cc(c[3])-22;
	display_7seg_set_colon_mode(DISPLAY_7SEG_COLON_OFF);
	cli();
	new_display_data_flag=1;
	sei();
}

#define ANIM_WAIT 750
unsigned char display_7seg_ISR(void){
	unsigned char display_sync_dim=0;
	unsigned char T_mask_and[5]={0xFF,0xFF,0xFF,0xFF,0xFF};
	unsigned char T_mask_or[5]={0x00,0x00,0x00,0x00,0x00};
	unsigned char tmp_or=0;
/*	switch (T_digits_state){
		case 0:	break; //no transition to do
		case 1:	if((digits[T_digits_counter]&0x7F)==(T_digits[T_digits_counter]&0x7F)){
					digits[T_digits_counter]=T_digits[T_digits_counter];
					if(T_digits_counter!=0){
						T_digits_counter--;
						T_digits_state=1;
					}else{
						T_digits_state=0;
						T_digits_counter=3;
					}
				}else{
					T_digits_state=2;
				}
				break;
		case 2:	T_mask_and[T_digits_counter]=~(SEG_A);
				T_digits_time_counter++;
				if(T_digits_time_counter>150){
					T_digits_time_counter=0;
					T_digits_state++;
				}
				break;
		case 3:	T_mask_and[T_digits_counter]=~(SEG_A|SEG_F|SEG_B);
				T_digits_time_counter++;
				if(T_digits_time_counter>350){
					T_digits_time_counter=0;
					T_digits_state++;
				}
				break;
		case 4:	T_mask_and[T_digits_counter]=~(SEG_A|SEG_F|SEG_B|SEG_G);
				T_digits_time_counter++;
				if(T_digits_time_counter>350){
					T_digits_time_counter=0;
					digits[T_digits_counter]=T_digits[T_digits_counter];
					T_digits_state=1;
				}
				break;
		case 30:	T_digits_state=0;
					break;
	}
*/
/*	switch (T_digits_state){
		case 0:	break; //no transition to do
		case 1:	if((digits[T_digits_counter]&0x7F)==(T_digits[T_digits_counter]&0x7F)){
					digits[T_digits_counter]=T_digits[T_digits_counter];
					if(T_digits_counter!=0){
						T_digits_counter--;
						T_digits_state=1;
					}else{
						T_digits_state=0;
						T_digits_counter=3;
					}
				}else{
					T_digits_state=2;
				}
				break;
		case 2:	T_mask_and[T_digits_counter]=~(SEG_A);
				T_mask_or[T_digits_counter]=number_patterns[T_digits[T_digits_counter]]&(SEG_A);
				T_digits_time_counter++;
				if(T_digits_time_counter>700){
					T_digits_time_counter=0;
					T_digits_state++;
				}
				break;
		case 3:	T_mask_and[T_digits_counter]=~(SEG_A|SEG_F|SEG_B);
				T_mask_or[T_digits_counter]=number_patterns[T_digits[T_digits_counter]]&(SEG_A|SEG_F|SEG_B);
				T_digits_time_counter++;
				if(T_digits_time_counter>700){
					T_digits_time_counter=0;
					T_digits_state++;
				}
				break;
		case 4:	T_mask_and[T_digits_counter]=~(SEG_A|SEG_F|SEG_B|SEG_G);
				T_mask_or[T_digits_counter]=number_patterns[T_digits[T_digits_counter]]&(SEG_A|SEG_F|SEG_B|SEG_G);
				T_digits_time_counter++;
				if(T_digits_time_counter>100){
					T_digits_time_counter=0;
					digits[T_digits_counter]=T_digits[T_digits_counter];
					T_digits_state=1;
				}
				break;
		case 30:	T_digits_state=0;
					break;
	}*/
	
	if(new_display_data_flag){
		if(T_digits_state==0){
			T_digits[0]=I_digits[0];
			T_digits[1]=I_digits[1];
			T_digits[2]=I_digits[2];
			T_digits[3]=I_digits[3];
			T_digits_state=1;
			new_display_data_flag=0;
		}
	}
	
	switch (T_digits_state){
		case 0:	break; //no transition to do
		case 1:	if((anim_mode==0)||(anim_on==0)){
					digits[0]=I_digits[0];digits[1]=I_digits[1];digits[2]=I_digits[2];digits[3]=I_digits[3];
					T_digits_state=0;T_digits_counter=3;
					anim_on=0;
					break;
				}
				if((digits[T_digits_counter]&0x7F)==(T_digits[T_digits_counter]&0x7F)){
					digits[T_digits_counter]=T_digits[T_digits_counter];
					if(T_digits_counter!=0){
						T_digits_counter--;
						T_digits_state=1;
					}else{
						anim_on=0;
						T_digits_state=0;
						T_digits_counter=3;
					}
				}else{
					T_digits_time_counter=0;
					T_digits_state=2;
				}
				break;
		case 2:	T_mask_and[T_digits_counter]=0;
				if(number_patterns_ascii[digits[T_digits_counter]]&SEG_A){
					tmp_or|=SEG_G;
				}
				if(number_patterns_ascii[digits[T_digits_counter]]&SEG_F){
					tmp_or|=SEG_E;
				}
				if(number_patterns_ascii[digits[T_digits_counter]]&SEG_B){
					tmp_or|=SEG_C;
				}
				if(number_patterns_ascii[digits[T_digits_counter]]&SEG_G){
					tmp_or|=SEG_D;
				}
				T_mask_or[T_digits_counter]=tmp_or;
				T_digits_time_counter++;
				if(T_digits_time_counter>ANIM_WAIT){
					T_digits_time_counter=0;
					T_digits_state++;
				}
				break;
		case 3:	T_mask_and[T_digits_counter]=0;

				if(number_patterns_ascii[digits[T_digits_counter]]&SEG_A){
					tmp_or|=SEG_D;
				}
				T_mask_or[T_digits_counter]=tmp_or;
				T_digits_time_counter++;
				if(T_digits_time_counter>ANIM_WAIT){
					T_digits_time_counter=0;
					T_digits_state++;
				}
				break;
		case 4:	T_mask_and[T_digits_counter]=0;
				T_mask_or[T_digits_counter]=0;
				T_digits_time_counter++;
				if(T_digits_time_counter>ANIM_WAIT*4){
					T_digits_time_counter=0;
					T_digits_state++;
				}

				break;
		case 5:	T_mask_and[T_digits_counter]=0;
				if(number_patterns_ascii[T_digits[T_digits_counter]]&SEG_D){
					tmp_or|=SEG_A;
				}
				T_mask_or[T_digits_counter]=tmp_or;
				T_digits_time_counter++;
				if(T_digits_time_counter>ANIM_WAIT){
					T_digits_time_counter=0;
					T_digits_state++;
				}
				break;
		case 6:	T_mask_and[T_digits_counter]=0;
				if(number_patterns_ascii[T_digits[T_digits_counter]]&SEG_D){
					tmp_or|=SEG_G;
				}
				if(number_patterns_ascii[T_digits[T_digits_counter]]&SEG_E){
					tmp_or|=SEG_F;
				}
				if(number_patterns_ascii[T_digits[T_digits_counter]]&SEG_C){
					tmp_or|=SEG_B;
				}
				if(number_patterns_ascii[T_digits[T_digits_counter]]&SEG_G){
					tmp_or|=SEG_A;
				}
				T_mask_or[T_digits_counter]=tmp_or;
				T_digits_time_counter++;
				if(T_digits_time_counter>ANIM_WAIT){
					T_digits_time_counter=0;
					T_digits_state++;
				}
				break;

		case 7:	T_mask_and[T_digits_counter]=~(SEG_A|SEG_F|SEG_B|SEG_G);
				T_mask_or[T_digits_counter]=number_patterns_ascii[T_digits[T_digits_counter]]&(SEG_A|SEG_F|SEG_B|SEG_G);
				T_digits_time_counter++;
				if(T_digits_time_counter>1){
					T_digits_time_counter=0;
					digits[T_digits_counter]=T_digits[T_digits_counter];
					T_digits_state=1;
				}
				break;
	}
	SEG_MODE=I_SEG_MODE;
	COLON_MODE=I_COLON_MODE;
	switch(SEG_MODE){
		case DISPLAY_7SEG_BRIGHT:	all_digits_off();PORT_COLON_OFF;
							if(digit_counter<4){
								if(digits[digit_counter]&0x80){
									set_digit_pattern(digit_counter,((number_patterns_ascii[(digits[digit_counter]&0x7F)]|SEG_DP)&T_mask_and[digit_counter])|T_mask_or[digit_counter]);
								}else{
									set_digit_pattern(digit_counter,((number_patterns_ascii[(digits[digit_counter]&0x7F)])&T_mask_and[digit_counter])|T_mask_or[digit_counter]);
								}
								enable_digit(digit_counter);
								if((digit_counter==0)&&(COLON_MODE==DISPLAY_7SEG_COLON_ON)){
									PORT_COLON_ON;
								}
								digit_counter++;
								if((bright_value==0)&&(digit_counter==4)){
									digit_counter=0;
									br_counter=0;
									display_sync_dim=1;
								}
							}else{
								display_sync_dim=1;
								br_counter++;
								//if(br_counter>br_translate[bright_value/10]){
								if(br_counter>bright_value){
									digit_counter=0;
									br_counter=0;
								}
							}
							break;
		case DISPLAY_7SEG_DIM:		all_digits_off();PORT_COLON_OFF;
							if(digit_counter<4){
								if(digits[digit_counter]&0x80){
									set_digit_pattern(digit_counter,((number_patterns_ascii[(digits[digit_counter]&0x7F)]|SEG_DP)&T_mask_and[digit_counter])|T_mask_or[digit_counter]);
								}else{
									set_digit_pattern(digit_counter,((number_patterns_ascii[(digits[digit_counter]&0x7F)])&T_mask_and[digit_counter])|T_mask_or[digit_counter]);
								}
								if(dimm_value<250)enable_digit(digit_counter);
								if((digit_counter==0)&&(COLON_MODE==DISPLAY_7SEG_COLON_ON)){
									if(dimm_value<250)PORT_COLON_ON;
								}
								digit_counter++;
								if((dimm_value==0)&&(digit_counter==4)){
									digit_counter=0;
									br_counter=0;
									display_sync_dim=1;
								}
							}else{
								display_sync_dim=1;
								br_counter++;
								//if(br_counter>br_translate[dimm_value/10]){
								if(br_counter>dimm_value){
									digit_counter=0;
									br_counter=0;
								}
							}
					break;

		case DISPLAY_7SEG_FLICKER:	if(flash_counter>18){
									all_digits_off();PORT_COLON_OFF;
									if(digit_counter!=4){
										if(digits[digit_counter]&0x80){
									set_digit_pattern(digit_counter,((number_patterns_ascii[(digits[digit_counter]&0x7F)]|SEG_DP)&T_mask_and[digit_counter])|T_mask_or[digit_counter]);
										}else{
									set_digit_pattern(digit_counter,((number_patterns_ascii[(digits[digit_counter]&0x7F)])&T_mask_and[digit_counter])|T_mask_or[digit_counter]);
										}
										enable_digit(digit_counter);
										if((digit_counter==0)&&(COLON_MODE==DISPLAY_7SEG_COLON_ON)){
											PORT_COLON_ON;
										}
										digit_counter++;
										if((bright_value==0)&&(digit_counter==4)){
											digit_counter=0;
											br_counter=0;
											display_sync_dim=1;
										}
									}else{
										display_sync_dim=1;
										br_counter++;
										//if(br_counter>br_translate[bright_value/10]){
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
		case DISPLAY_7SEG_DIM_FLICKER:	if(flash_counter>18){
									all_digits_off();PORT_COLON_OFF;
									if(digit_counter!=4){
										if(digits[digit_counter]&0x80){
											set_digit_pattern(digit_counter,((number_patterns_ascii[(digits[digit_counter]&0x7F)]|SEG_DP)&T_mask_and[digit_counter])|T_mask_or[digit_counter]);
										}else{
											set_digit_pattern(digit_counter,((number_patterns_ascii[(digits[digit_counter]&0x7F)])&T_mask_and[digit_counter])|T_mask_or[digit_counter]);
										}
										enable_digit(digit_counter);
										if((digit_counter==0)&&(COLON_MODE==DISPLAY_7SEG_COLON_ON)){
											PORT_COLON_ON;
										}
										digit_counter++;
										if((dimm_value==0)&&(digit_counter==4)){
											digit_counter=0;
											br_counter=0;
											display_sync_dim=1;
										}
									}else{
										display_sync_dim=1;
										br_counter++;
										//if(br_counter>br_translate[dimm_value/10]){
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
		case DISPLAY_7SEG_OFF:	all_digits_off();PORT_COLON_OFF;
					break;
		case DISPLAY_7SEG_FLASH:		if(flash_counter<2000){
								all_digits_off();PORT_COLON_OFF;
								if(digit_counter!=4){
									if(digits[digit_counter]&0x80){
										set_digit_pattern(digit_counter,((number_patterns_ascii[(digits[digit_counter]&0x7F)]|SEG_DP)&T_mask_and[digit_counter])|T_mask_or[digit_counter]);
									}else{
										set_digit_pattern(digit_counter,((number_patterns_ascii[(digits[digit_counter]&0x7F)])&T_mask_and[digit_counter])|T_mask_or[digit_counter]);
									}
									enable_digit(digit_counter);
									if((digit_counter==0)&&(COLON_MODE==DISPLAY_7SEG_COLON_ON)){
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
		case DISPLAY_7SEG_ZOOM:	all_digits_off();PORT_COLON_OFF;
						if(digit_counter<4){
							if(digits[digit_counter]&0x80){
								set_digit_pattern(digit_counter,((number_patterns_ascii[(digits[digit_counter]&0x7F)]|SEG_DP)&T_mask_and[digit_counter])|T_mask_or[digit_counter]);
							}else{
								set_digit_pattern(digit_counter,((number_patterns_ascii[(digits[digit_counter]&0x7F)])&T_mask_and[digit_counter])|T_mask_or[digit_counter]);
							}
							enable_digit(digit_counter);
							if((digit_counter==0)&&(COLON_MODE==DISPLAY_7SEG_COLON_ON)){
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

void display_7seg_init(void){
		//all leds off
		DDRD=0x78;
		PORTD=0x78;
		
		//PD7 output and default low=off for a fixed :
		DDRD|=0x80;
		PORT_COLON_OFF;
		
		DDRB|=0xFF;
		PORTB=0xFF;
}


#endif