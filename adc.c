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

#ifndef ADC_C
#define ADC_C
#include "config.h"
#include <avr/io.h>
#include <avr/interrupt.h>

unsigned int ad_value=0;

void adc_ISR(void){
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


void adc_init(void){

	ADMUX=0x40;	//reference on vccf, channel 0 start
	ADCSRA=0x9F;	//interrupts on, highest prescaler, no started yet
	ADCSRA|=0x40;
}

#endif