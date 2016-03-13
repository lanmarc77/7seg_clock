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

#include "config.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include "usart.h"
#ifndef USART_C
#define USART_C


unsigned char sendBufferPtr=0;
unsigned char sentBufferPtr=0;
unsigned char sendBuffer[50]={0};

char usart_send_char(unsigned char c){
	if(sendBufferPtr<sizeof(sendBuffer)){
		sendBuffer[sendBufferPtr]=c;
		cli();
		sendBufferPtr++;
		//if(UCSRA&0x20){
		if(sendBufferPtr==1){
			usart_send_next_ISR();
		}
		sei();
		return 1;
	}else{
		return 0;
	}
}

void usart_send_next_ISR(void){
	if(sendBufferPtr!=sentBufferPtr){
		UDR=sendBuffer[sentBufferPtr];
		sentBufferPtr++;
	}else{
		sentBufferPtr=0;
		sendBufferPtr=0;
	}
}


void usart_init(void){
	//UBRRL=51;//38400@16MHz
	UBRRL=16;//115200@16MHz
	UBRRH=0;
	UCSRA=0x02;
	UCSRB=0x90|0x48;
	UCSRC=0x86;
}

#endif