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

#ifndef TEMP_C
#define TEMP_C
#include "config.h"
#include "ui_display_modes.h"
#include "clock.h"
#include "temp.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>
#include "TWI_Master.h"

volatile signed int SE95_temp=99;
volatile signed int SE95_temp_frac=99;
volatile unsigned char SE95_detected=0;

volatile unsigned char SE95_cmd=0;
volatile unsigned char SE95_state=0;

unsigned char temp_messageBuf[25];
volatile signed char I2C_temp=0;
volatile unsigned char I2C_temp_frac=0;


void SE95_check_i2c_state_machine(void){
	if(SE95_detected==0){
		SE95_cmd=0;
		SE95_state=0;
		return;
	}
	if(RTC_state!=0){//wait for RTC calls to finish
		return;
	}
	switch(SE95_state){
		case 0:	if(SE95_cmd==SE95_get_temp){
					temp_messageBuf[0]=(0x48<<TWI_ADR_BITS) | (FALSE<<TWI_READ_BIT);
					temp_messageBuf[1]=0x00;//setup reading from register 00 means temperature
					TWI_Start_Transceiver_With_Data( &temp_messageBuf[0], 2 );
					SE95_state=20;
				}
				break;
		case 20:if(!(TWI_Transceiver_Busy() )){
					if ( TWI_statusReg.lastTransOK ){
						temp_messageBuf[0]=(0x48<<TWI_ADR_BITS) | (TRUE<<TWI_READ_BIT);
						TWI_Start_Transceiver_With_Data( &temp_messageBuf[0], 3);
						SE95_state++;
					}else{
						SE95_cmd=0;
						SE95_state=0;
					}
				}
				break;
		case 21:if(!(TWI_Transceiver_Busy() )){
					if ( TWI_statusReg.lastTransOK ){
						TWI_Get_Data_From_Transceiver( &temp_messageBuf[0], 3 );
						cli();
						SE95_temp=((temp_messageBuf[1]<<8)|temp_messageBuf[2])>>3;
						SE95_temp_frac=(SE95_temp)&0x1F;
						SE95_temp>>=5;//reduce to 1 centigrade resolution
						sei();
					}
					SE95_cmd=0;
					SE95_state=0;
				}
				break;
	}
}


//signed int LMT_temp=0;
unsigned int ad_value=0;

ISR(ADC_vect){
	switch(ADMUX&0x07){
		case 0: ADMUX=(ADMUX&0xF7)|0x01;ad_value=ADCL|(ADCL<<8);break;
		case 1: ADMUX=(ADMUX&0xF7)|0x02;break;
		case 2: ADMUX=(ADMUX&0xF7)|0x03;break;
		case 3: ADMUX=(ADMUX&0xF7)|0x04;break;
		case 4: ADMUX=(ADMUX&0xF7)|0x05;break;
		case 5: ADMUX=(ADMUX&0xF7)|0x06;break;
		case 6: ADMUX=(ADMUX&0xF7)|0x07;break;
		case 7: ADMUX=(ADMUX&0xF7)|0x00;break;
		default: break;
	}
	ADCSRA|=0x40;
}


void temp_init(void){
	unsigned char i=0;
	unsigned char messageBuf[25];

	ADMUX=0x40;	//reference on vccf, channel 0 start
	ADCSRA=0x9F;	//interrupts on, highest prescaler, no started yet
	ADCSRA|=0x40;

	messageBuf[0]=(0x48<<TWI_ADR_BITS) | (FALSE<<TWI_READ_BIT);
	messageBuf[1]=0x01;//configuration register
	messageBuf[2]=0x00;//POR value=0x00
	TWI_Start_Transceiver_With_Data( messageBuf, 3 );
	do{sleep_mode();i++;if(i==0){break;}}while( TWI_Transceiver_Busy() );
	if ((!( TWI_statusReg.lastTransOK ))||(i==0)){
		//error occured
		//TWI_Get_State_Info( );   //check the error value/last TWI state and act accordingly, error codes are defined in the header
		TWI_Master_Stop();
	}else{
		cli();
		SE95_detected=1;
		SE95_cmd=SE95_get_temp;
		display_update=1;
		sei();
	}
}

#endif