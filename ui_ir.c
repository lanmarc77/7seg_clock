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

#ifndef UI_IR_C
#define UI_IR_C
#include "config.h"
#include "ui_ir.h"
#include "clock.h"
#include "7seg_func.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include "irmp.h"



unsigned char ir_code_mode=0;
unsigned char ir_code_i=0;
unsigned int last_ir_code=0;
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


unsigned char number_input_mode=0;

/*returns
0: number insert canceled
1: number is still entered, but no change from previous call
2: number is still entered, number changed from previous call
3: timeout
4: ok pressed
*/
unsigned char number_input(unsigned char *number,unsigned char min,unsigned char max,unsigned int timeout){

	switch(number_input_mode){
		case 0:		stop_stop_watch();start_stop_watch();
					number_input_mode=1;
					break;
		case 1:		if(get_stop_watch()>timeout){
						number_input_mode=0;
						return 3;
					}else{
						switch(get_ir_code()){
							case IR_POWER: number_input_mode=0;
											return 0;
											break;
							case IR_CH_PLUS: 	if(get_stop_watch()>(400/4)){//longer than 400ms? since last keypress?
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
												stop_stop_watch();start_stop_watch();
												return 2;
												break;
							case IR_CH_MINUS: 	if(get_stop_watch()>(400/4)){//longer than 400ms? since last keypress?
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
												stop_stop_watch();start_stop_watch();
												return 2;
												break;
							case IR_MUTE: 		stop_stop_watch();start_stop_watch();
												number_input_mode=0;
												return 4;
												break;
						}
					}
	}
	return 1;
}

//returns the entered 4 digit code as int value
//0 if the code entering was interrupted
//1 if code is still entered
unsigned int code_input(void){
	switch(get_ir_input(4,1,15000,L_C,L_o,L_d,L_E)){//number of keys
		case 0:	return 0;
				break;
		case 2:	return 0;
				break;
		case 5:	return last_ir_code;
				break;
	}
	I_SEG_MODE=SEG_BRIGHT;
	return 1;
}

//returns
//0: Power key was pressed
//1: no key was pressed
//2: timeout detected
//5: all keys number were pressed, result in last_ir_code
unsigned int get_ir_input(unsigned char keys, unsigned char pw_mode, unsigned int timeout,unsigned char i0, unsigned char i1, unsigned char i2, unsigned char i3){
	signed char v=-1;

	switch (ir_code_mode){
		case 0:	last_ir_code=0;
				ir_code_i=0;
				v=-1;
				ir_code_value_temp=-1;
				ir_code_timeout_counter=0;
				stop_stop_watch();start_stop_watch();
				ir_code_mode=1;
				I_digits[0]=i0;
				I_digits[1]=i1;
				I_digits[2]=i2;
				I_digits[3]=i3;
				I_SEG_MODE=SEG_BRIGHT;
		case 1:	if(get_stop_watch()*4>1500){//1.5seconds waiting
						ir_code_mode=2;
						I_digits[0]=L_NOTHING;I_digits[1]=L_NOTHING;I_digits[2]=L_NOTHING;I_digits[3]=L_NOTHING;
						for(ir_code_i=0;ir_code_i<keys;ir_code_i++){
							I_digits[3-ir_code_i]=L__;
						}
						ir_code_i=0;
						stop_stop_watch();start_stop_watch();
				}
				break;
		case 2:if(get_stop_watch()*4>timeout){// timeout seconds waiting
						stop_stop_watch();
						ir_code_mode=0;
						return 2;
				}else{
					switch(get_ir_code()){
						case IR_POWER: ir_code_mode=0;
										return 0;
										break;
						case IR_CH_PLUS:	if(ir_code_value_temp<0){
												ir_code_value_temp=0;
											}else{
												ir_code_value_temp++;
												if(ir_code_value_temp>9){
													ir_code_value_temp=0;
												}
											}
											I_digits[ir_code_i+(4-keys)]=L_0+ir_code_value_temp;
											stop_stop_watch();start_stop_watch();
											break;
						case IR_CH_MINUS:	if(ir_code_value_temp<0){
												ir_code_value_temp=0;
											}else{
												ir_code_value_temp--;
												if(ir_code_value_temp<0){
													ir_code_value_temp=9;
												}
											}
											I_digits[ir_code_i+(4-keys)]=L_0+ir_code_value_temp;
											stop_stop_watch();start_stop_watch();
											break;
						case IR_MUTE:		if(ir_code_value_temp>=0){
												if(pw_mode){
													I_digits[ir_code_i+(4-keys)]=L_minus;
												}else{
													I_digits[ir_code_i+(4-keys)]=ir_code_value_temp;
												}
												ir_code_i++;
												last_ir_code=last_ir_code*10;
												last_ir_code+=ir_code_value_temp;
												ir_code_value_temp=-1;
												if(ir_code_i==keys){
													ir_code_mode=3;
												}
											}
											stop_stop_watch();start_stop_watch();
											break;
					}
					if(((get_stop_watch()*4)%500)>250){
						I_digits[ir_code_i+(4-keys)]=L_NOTHING;
					}else{
						if(ir_code_value_temp==-1){//nothing entered yet for the current digit
							I_digits[ir_code_i+(4-keys)]=L__;
						}else{
							I_digits[ir_code_i+(4-keys)]=ir_code_value_temp;
						}
					}
				}
				break;
		case 3: if(get_stop_watch()*4>500){// 0.5 seconds waiting
					stop_stop_watch();
					ir_code_mode=0;
					return 5;
				}
	
	}
	return 1;

}

void ir_init(void){
	//RC5 signal
	DDRC&=~0x40;
	PORTC|=0x40; //enable internal pullup
}

#endif