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
#include <avr/signal.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/eeprom.h>
#include "TWI_Master.h"
#include "irmp.h"


#include "ui_display_modes.h"
#include "ui_setup_menus.h"
#include "ui_ir.h"
#include "clock.h"
#include "7seg_func.h"
#include "temp.h"
#include "usart.h"




ISR(TIMER1_COMPA_vect){//every 1/15000s = 66,66667µs;main worker ISR
	unsigned char dsd=segments_ISR();
	wdt_reset();
	irmp_ISR();
	dcf77_isr_part(dsd);
}


//ISR for received USART characters
ISR(USART_RXC_vect){
	char c=UDR;
	if(c=='U'){//update needed?
       	wdt_enable(WDTO_15MS); //enable watchdog
       	while(1);//force a watchdog reset
    }else{
		usart_receive(c);
	}
}

ISR(USART_TXC_vect){
	usart_send_next();
}


int main (void)
{
		unsigned char display_mode=0;

		
		wdt_enable(WDTO_15MS);

		GICR&=0x1F; //disable all external interrupts
		
		//load saved values from EEPROM
		show_mode = eeprom_read_byte((uint8_t*)10);
		if(show_mode>7){show_mode=2;}

		dimm_value = eeprom_read_byte((uint8_t*)11);
		if(dimm_value>250){dimm_value=40;}
		
		bright_value = eeprom_read_byte((uint8_t*)12);
		if(bright_value>250){bright_value=0;}

		alarm_mode=eeprom_read_byte((uint8_t*)13);
		if(alarm_mode>ALARM_ON){alarm_mode=ALARM_OFF;}
		alarm_hour=eeprom_read_byte((uint8_t*)14);
		if(alarm_hour>23){alarm_hour=0;}
		alarm_minute=eeprom_read_byte((uint8_t*)15);
		if(alarm_minute>59){alarm_minute=0;}

		segment_mode = eeprom_read_byte((uint8_t*)16);
		if(segment_mode>1){segment_mode=1;}

		fixed_mode = eeprom_read_byte((uint8_t*)17);
		if(fixed_mode>1){fixed_mode=0;}
		
		dcf77_inverted_flag = eeprom_read_byte((uint8_t*)18);
		if((dcf77_inverted_flag!=0x00)&&(dcf77_inverted_flag!=0x80)){
			dcf77_inverted_flag=0x00;
		}
		
		
		load_schedules();
		
		sei();
		set_sleep_mode(SLEEP_MODE_IDLE);

		TWI_Master_Initialise();	//initialize the TWI/I2C interface
		segments_init();
		dcf77_init();
		ir_init();
		temp_init();
		
		//timer 1 is dcf signal search, display numbers and IR reception
		OCR1A=1067;
		TIMSK|=0x10;
		TCCR1A=0x00;
		TCCR1B=0x08|0x01;

		//UBRRL=51;//38400@16MHz
		UBRRL=16;//115200@16MHz
		UBRRH=0;
		UCSRA=0x02;
		UCSRB=0x90|0x48;
		UCSRC=0x86;

		while(1){
			cli();
			refresh_display_content();
			sei();
			sleep_mode();
			cli();
			get_time();
			sei();
			RTC_check_i2c_state_machine();
			SE95_check_i2c_state_machine();
			switch(display_mode){
				case 0:if(version_default()==0){
							if(RTC_detected) {
								display_mode=2;//2=default
							}else{
								display_mode=1;
							}
						}
						break;
				case 1:
						if(((DCF77_PIN^dcf77_inverted_flag))==0){
							I_digits[0]=10;//default 10
						}else{
							I_digits[0]=10|0x80;//default 10
						}
						I_digits[1]=10;//default 10
						I_digits[2]=10;//default 10
						I_digits[3]=dcf_state%10;
						if(filtered_dcf77!=0){
							I_digits[1]|=0x80;
						}else{
							I_digits[1]&=~0x80;
						}
						if(dcf_state==3){
							display_mode=2;
						}
						I_SEG_MODE=SEG_DIM;
						//I_SEG_MODE=SEG_BRIGHT;
						break;
				case 2:	if(check_alarm()){
						}else{
							switch(show_mode){
								case 0:	c1_default();break;
								case 1:TA_default();break;
								case 2:simple_default();break;
								case 3:WBS_default();break;
								case 4:TE_default();break;
								case 5:bin_default();break;
								case 6:C2_default();break;
								case 7:C3_default();break;
							}
							break;
						}
			}
		}
}
