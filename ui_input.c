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

#ifndef UI_INPUT_C
#define UI_INPUT_C
#include "config.h"
#include "ui_input.h"
#include "clock.h"
#include "display.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include "irmp.h"


#define BUTTON_BACK (!(PINA&0x10))
#define BUTTON_DOWN (!(PINA&0x20))
#define BUTTON_OK (!(PINA&0x40))
#define BUTTON_UP (!(PINA&0x80))
#define BUTTON_ANY ((PINA&0xF0)!=0xF0)
#define BUTTON_NONE ((PINA&0xF0)==0xF0)
/*
	0xff: no key pressed
	other see defines below
*/
#define IR_POWER 0x12
/*#define IR_VOL_PLUS 0x13
#define IR_VOL_MINUS 0x14
#define IR_0 0x15
#define IR_1 0x16
#define IR_2 0x17
#define IR_3 0x18
#define IR_4 0x19
#define IR_5 0x1A
#define IR_6 0x1B
#define IR_7 0x1C
#define IR_8 0x1D
#define IR_9 0x1E*/
#define IR_CH_PLUS 0x1F
#define IR_CH_MINUS 0x20
#define IR_MUTE 0x21

unsigned char ir_code_mode=0;
unsigned char ir_code_i=0;
unsigned int last_code=0;
unsigned int ir_code_timeout_counter=0;
signed char ir_code_value_temp=-1;


unsigned char get_ir_code(void){
    IRMP_DATA   irmp_data;
	irmp_data.command=0;
	irmp_data.flags=0;
	irmp_data.protocol=0;
	if (irmp_get_data (&irmp_data))
	{
		if((irmp_data.flags & IRMP_FLAG_REPETITION) ==  0){
			if(	((irmp_data.protocol==IRMP_NEC_PROTOCOL)&&(irmp_data.command == 0x12))||
			((irmp_data.protocol==IRMP_RC5_PROTOCOL)&&(irmp_data.command == 12))
			){
				return IR_POWER;
			}
			/*if(	((irmp_data.protocol==IRMP_NEC_PROTOCOL)&&(irmp_data.command == 0x1A))||
			((irmp_data.protocol==IRMP_RC5_PROTOCOL)&&(irmp_data.command == 16))
			){
				return IR_VOL_PLUS;
			}
			if(	((irmp_data.protocol==IRMP_NEC_PROTOCOL)&&(irmp_data.command == 0x02))||
			((irmp_data.protocol==IRMP_RC5_PROTOCOL)&&(irmp_data.command == 17))
			){
				return IR_VOL_MINUS;
			}
			if(	((irmp_data.protocol==IRMP_NEC_PROTOCOL)&&(irmp_data.command == 0x0D))||
			((irmp_data.protocol==IRMP_RC5_PROTOCOL)&&(irmp_data.command == 0))
			){
				return IR_0;
			}
			if(	((irmp_data.protocol==IRMP_NEC_PROTOCOL)&&(irmp_data.command == 0x04))||
			((irmp_data.protocol==IRMP_RC5_PROTOCOL)&&(irmp_data.command == 1))
			){
				return IR_1;
			}
			if(	((irmp_data.protocol==IRMP_NEC_PROTOCOL)&&(irmp_data.command == 0x05))||
			((irmp_data.protocol==IRMP_RC5_PROTOCOL)&&(irmp_data.command == 2))
			){
				return IR_2;
			}
			if(	((irmp_data.protocol==IRMP_NEC_PROTOCOL)&&(irmp_data.command == 0x06))||
			((irmp_data.protocol==IRMP_RC5_PROTOCOL)&&(irmp_data.command == 3))
			){
				return IR_3;
			}
			if(	((irmp_data.protocol==IRMP_NEC_PROTOCOL)&&(irmp_data.command == 0x07))||
			((irmp_data.protocol==IRMP_RC5_PROTOCOL)&&(irmp_data.command == 4))
			){
				return IR_4;
			}
			if(	((irmp_data.protocol==IRMP_NEC_PROTOCOL)&&(irmp_data.command == 0x08))||
			((irmp_data.protocol==IRMP_RC5_PROTOCOL)&&(irmp_data.command == 5))
			){
				return IR_5;
			}
			if(	((irmp_data.protocol==IRMP_NEC_PROTOCOL)&&(irmp_data.command == 0x09))||
			((irmp_data.protocol==IRMP_RC5_PROTOCOL)&&(irmp_data.command == 6))
			){
				return IR_6;
			}
			if(	((irmp_data.protocol==IRMP_NEC_PROTOCOL)&&(irmp_data.command == 0x0A))||
			((irmp_data.protocol==IRMP_RC5_PROTOCOL)&&(irmp_data.command == 7))
			){
				return IR_7;
			}
			if(	((irmp_data.protocol==IRMP_NEC_PROTOCOL)&&(irmp_data.command == 0x1B))||
			((irmp_data.protocol==IRMP_RC5_PROTOCOL)&&(irmp_data.command == 8))
			){
				return IR_8;
			}
			if(	((irmp_data.protocol==IRMP_NEC_PROTOCOL)&&(irmp_data.command == 0x1F))||
			((irmp_data.protocol==IRMP_RC5_PROTOCOL)&&(irmp_data.command == 9))
			){
				return IR_9;
			}*/
			if(	((irmp_data.protocol==IRMP_NEC_PROTOCOL)&&(irmp_data.command == 0x1E))||
			((irmp_data.protocol==IRMP_RC5_PROTOCOL)&&(irmp_data.command == 32))
			){
				return IR_CH_PLUS;
			}
			if(	((irmp_data.protocol==IRMP_NEC_PROTOCOL)&&(irmp_data.command == 0x03))||
			((irmp_data.protocol==IRMP_RC5_PROTOCOL)&&(irmp_data.command == 33))
			){
				return IR_CH_MINUS;
			}
			if(	((irmp_data.protocol==IRMP_NEC_PROTOCOL)&&(irmp_data.command == 0x01))||
			((irmp_data.protocol==IRMP_RC5_PROTOCOL)&&(irmp_data.command == 13))
			){
				return IR_MUTE;
			}
		}
	}
	return 0xFF;
}

#define B_NONE 0
#define B_OK 1
#define B_BACK 2
#define B_DOWN 3
#define B_UP 4
unsigned char last_button_press=B_NONE;

unsigned char get_button_code(void){
	cli();
	unsigned char c=last_button_press;
	last_button_press=B_NONE;
	sei();
	return c;
}


unsigned char ui_input_simulated=UI_INPUT_KEY_NONE;
void ui_input_simulate(unsigned char k){
	ui_input_simulated=k;
}

unsigned char ui_input_get_key(void){
	unsigned char c=ui_input_simulated;
	switch(get_ir_code()){
		case IR_CH_PLUS:return UI_INPUT_KEY_UP;break;
		case IR_CH_MINUS:return UI_INPUT_KEY_DOWN;break;
		case IR_MUTE:return UI_INPUT_KEY_OK;break;
		case IR_POWER:return UI_INPUT_KEY_BACK;break;
	}
	switch(get_button_code()){
		case B_OK: return UI_INPUT_KEY_OK;break;
		case B_BACK:return UI_INPUT_KEY_BACK;break;
		case B_DOWN: return UI_INPUT_KEY_DOWN;break;
		case B_UP:return UI_INPUT_KEY_UP;break;
	}
	if(ui_input_simulated!=UI_INPUT_KEY_NONE){
		ui_input_simulated=UI_INPUT_KEY_NONE;
		return c;
	}
	return UI_INPUT_KEY_NONE;
}



unsigned char number_input_mode=0;

/*returns
0: number insert canceled
1: number is still entered, but no change from previous call
2: number is still entered, number changed from previous call
3: timeout
4: ok pressed
*/
unsigned char ui_input_number(unsigned char *number,unsigned char min,unsigned char max,unsigned int timeout){

	switch(number_input_mode){
		case 0:		clock_stop_stop_watch();clock_start_stop_watch();
					number_input_mode=1;
					break;
		case 1:		if(clock_get_stop_watch()>timeout){
						number_input_mode=0;
						return 3;
					}else{
						switch(ui_input_get_key()){
							case UI_INPUT_KEY_BACK: number_input_mode=0;
											return 0;
											break;
							case UI_INPUT_KEY_UP: 	if(clock_get_stop_watch()>(400/4)){//longer than 400ms? since last keypress?
													if(*number>=max){
														*number=min;
													}else{
														*number+=1;
													}
												}else{
													if(*number>=max){
														*number=min;
													}else{
														if(*number%5==0){
															*number+=5;
														}else{
															*number+=(5-*number%5);
														}
														if(*number>max){
															*number=min;
														}
													}
												}
												clock_stop_stop_watch();clock_start_stop_watch();
												return 2;
												break;
							case UI_INPUT_KEY_DOWN: 	if(clock_get_stop_watch()>(400/4)){//longer than 400ms? since last keypress?
													if(*number<=min){
														*number=max;
													}else{
														*number-=1;
													}
												}else{
													if(*number<=min){
														*number=max;
													}else{
														if(*number>5){
															if(*number%5==0){
																*number-=5;
															}else{
																*number-=*number%5;
															}
														}
													}
												}
												clock_stop_stop_watch();clock_start_stop_watch();
												return 2;
												break;
							case UI_INPUT_KEY_OK: 		clock_stop_stop_watch();clock_start_stop_watch();
												number_input_mode=0;
												return 4;
												break;
						}
					}
	}
	return 1;
}





char ir_c[4];

//returns
//0: Power key was pressed
//1: no key was pressed
//2: timeout detected
//5: all keys number were pressed, result in last_code
unsigned char ui_input_get_digits(unsigned char keys, unsigned char pw_mode, unsigned int timeout,char *t,unsigned int *res){
	signed char v=-1;
	switch (ir_code_mode){
		case 0:	last_code=0;
				ir_code_i=0;
				v=-1;
				ir_code_value_temp=-1;
				ir_code_timeout_counter=0;
				clock_stop_stop_watch();clock_start_stop_watch();
				ir_code_mode=1;
				display_set_mode(DISPLAY_7SEG_BRIGHT);
				display_set_text(t);
		case 1:	if(clock_get_stop_watch()*4>1500){//1.5seconds waiting
						ir_code_mode=2;
						ir_c[0]=' ';ir_c[1]=' ';ir_c[2]=' ';ir_c[3]=' ';
						for(ir_code_i=0;ir_code_i<keys;ir_code_i++){
							ir_c[3-ir_code_i]='_';
						}
						ir_code_i=0;
						clock_stop_stop_watch();clock_start_stop_watch();
						display_set_text(&ir_c[0]);
				}
				break;
		case 2:if(clock_get_stop_watch()*4>timeout){// timeout seconds waiting
						clock_stop_stop_watch();
						ir_code_mode=0;
						return 2;
				}else{
					switch(ui_input_get_key()){
						case UI_INPUT_KEY_BACK: ir_code_mode=0;
										return 0;
										break;
						case UI_INPUT_KEY_UP:	if(ir_code_value_temp<0){
												ir_code_value_temp=0;
											}else{
												ir_code_value_temp++;
												if(ir_code_value_temp>9){
													ir_code_value_temp=0;
												}
											}
											ir_c[ir_code_i+(4-keys)]=ir_code_value_temp+48;
											clock_stop_stop_watch();clock_start_stop_watch();
											break;
						case UI_INPUT_KEY_DOWN:	if(ir_code_value_temp<0){
												ir_code_value_temp=0;
											}else{
												ir_code_value_temp--;
												if(ir_code_value_temp<0){
													ir_code_value_temp=9;
												}
											}
											ir_c[ir_code_i+(4-keys)]=ir_code_value_temp+48;
											clock_stop_stop_watch();clock_start_stop_watch();
											break;
						case UI_INPUT_KEY_OK:		if(ir_code_value_temp>=0){
												if(pw_mode){
													ir_c[ir_code_i+(4-keys)]='-';
												}else{
													ir_c[ir_code_i+(4-keys)]=ir_code_value_temp+48;
												}
												ir_code_i++;
												last_code=last_code*10;
												last_code+=ir_code_value_temp;
												ir_code_value_temp=-1;
												if(ir_code_i==keys){
													ir_code_mode=3;
													return 1;
												}
											}
											clock_stop_stop_watch();clock_start_stop_watch();
											break;
					}
					if(((clock_get_stop_watch()*4)%500)>250){
						ir_c[ir_code_i+(4-keys)]=' ';
					}else{
						if(ir_code_value_temp==-1){//nothing entered yet for the current digit
							ir_c[ir_code_i+(4-keys)]='_';
						}else{
							ir_c[ir_code_i+(4-keys)]=ir_code_value_temp+48;
						}
					}
					display_set_text(&ir_c[0]);
				}
				break;
		case 3: if(clock_get_stop_watch()*4>500){// 0.5 seconds waiting
					clock_stop_stop_watch();
					ir_code_mode=0;
					*res=last_code;
					return 5;
				}
	
	}
	return 1;

}


//returns the entered 4 digit code as int value
//0 if the code entering was interrupted
//1 if code is still entered
/*unsigned int ui_input_code(void){
	unsigned int code=0;
	switch(ui_input_get_digits(4,1,15000,"Code",&code)){//number of keys
		case 0:	return 0;
				break;
		case 2:	return 0;
				break;
		case 5:	return code;
				break;
	}
	//I_SEG_MODE=SEG_BRIGHT;
	display_set_mode(DISPLAY_7SEG_BRIGHT);
	return 1;
}*/

unsigned char ui_input_code_state=0;
unsigned char ui_input_code_digit=0;
unsigned char ui_input_code_cnt=0;
unsigned int code=0;
signed char ui_input_code_ok=-1;
signed int ui_input_code(void){


	switch(ui_input_code_state){
		case 0:	clock_stop_stop_watch();clock_start_stop_watch();
				ui_input_code_digit=0;ui_input_code_cnt=0;
				ui_input_code_state=1;code=0;ui_input_code_ok=-1;
				ir_c[0]='C';
				ir_c[1]='o';
				ir_c[2]='d';
				ir_c[3]='e';
				display_set_text(&ir_c[0]);
				break;
		case 1:	if((clock_get_stop_watch()*4)>2000){//2 seconds
					clock_stop_stop_watch();clock_start_stop_watch();
					ui_input_code_state=2;
					ir_c[0]='-';
					ir_c[1]='-';
					ir_c[2]='-';
					ir_c[3]='-';
					display_set_text(&ir_c[0]);
				}
				break;
		case 2:	if((clock_get_stop_watch()*4)>1250){//every second
					clock_stop_stop_watch();clock_start_stop_watch();
					if((ui_input_code_ok==ui_input_code_digit)&&(ui_input_code_cnt%10==9)){
						ui_input_code_cnt=0;
						ui_input_code_digit++;
						if(ui_input_code_digit==4){
							ui_input_code_state=0;
							return code;
						}
						code=code*10;
					}else{
						ui_input_code_cnt++;
						if(ui_input_code_cnt>39){
							ui_input_code_state=0;
							return -1;
						}
					}
				}
				switch(ui_input_get_key()){
					case UI_INPUT_KEY_BACK:	ui_input_code_state=0;return -1;
											break;
					case UI_INPUT_KEY_UP:
											break;
					case UI_INPUT_KEY_DOWN:
											break;
					case UI_INPUT_KEY_OK:	code=(code/10)*10+ui_input_code_cnt%10;
											ui_input_code_ok=ui_input_code_digit;
											break;
				}
				ir_c[0]='-';
				ir_c[1]='-';
				ir_c[2]='-';
				ir_c[3]='-';
				ir_c[ui_input_code_digit]=ui_input_code_cnt%10+48;
				display_set_text(&ir_c[0]);
				break;
	}
	return -2;
}

unsigned int button_debounce=0;
unsigned char button_sm=0;

extern void ui_input_ISR(void){
	irmp_ISR();
	
	switch(button_sm){
		case 0: if(BUTTON_ANY){
					button_debounce=500;//50ms
					button_sm++;
				 }
				 break;
	
		case 1: button_debounce--;
				if(button_debounce==0){
					if(BUTTON_ANY){
						if(BUTTON_OK){
							cli();last_button_press=B_OK;sei();
						}else if(BUTTON_BACK){
							cli();last_button_press=B_BACK;sei();
						}else if(BUTTON_UP){
							cli();last_button_press=B_UP;sei();
						}else if(BUTTON_DOWN){
							cli();last_button_press=B_DOWN;sei();
						}
						button_sm++;
					}else{
						button_sm=0;
					}
				}
				break;
	
		case 2: if(BUTTON_NONE){
					button_debounce=500;//50ms
					button_sm++;
				}
				break;

		case 3: button_debounce--;
				if(button_debounce==0){
					button_debounce=0;
					button_sm=0;
				}
				break;
	}
	
}

void ui_input_init(void){
	//RC5 signal
	DDRC&=~0x40;
	PORTC|=0x40; //enable internal pullup
	DDRA&=~0xF0; //PA4..P7 button input
	PORTA|=0xF0; //enable internal pullups for buttons
	button_sm=0;
}

#endif