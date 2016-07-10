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
#include <avr/delay.h>
#include "settings.h"
#include "TWI_Master.h"
//#include "irmp.h"


#include "ui_display_modes.h"
#include "ui_menus.h"
#include "ui_input.h"
#include "dcf77.h"
#include "clock.h"
#include "display.h"
#include "beeper.h"
#include "adc.h"
#include "usart.h"
#include "i2c_modules.h"



unsigned char dcf77_isr_part_counter=0;
unsigned char irmp_isr_part_counter=0;
unsigned char clock_isr_part_counter=0;
unsigned char segments_isr_part_counter=0;




//extra long running timer to allow timer0 to work a bit longer without interfering
//with the precision of the internal clock
ISR(TIMER2_COMP_vect){//every 50µs;main worker ISR
	clock_isr_part_counter++;
	if(clock_isr_part_counter==4){//every 4ms
		clock_isr_part();
		clock_isr_part_counter=0;
	}

}

ISR(TIMER0_COMP_vect){//every 50µs;main worker ISR
	unsigned char dsd=0;
	irmp_isr_part_counter++;
	if(irmp_isr_part_counter==2){//10khz
		irmp_isr_part_counter=0;
		wdt_reset();
		ui_input_ISR();
		dsd=display_ISR();
		beeper_ISR();
		dcf77_isr_part_counter++;
		if(dcf77_isr_part_counter==10){
			if(dsd){
				dcf77_isr_part_counter=0;
				dcf77_isr_part();//every 1ms
			}else{
				dcf77_isr_part_counter--;
			}
		}
	}

}

ISR(ADC_vect){
	adc_ISR();

}

//ISR for received USART characters
ISR(USART_RXC_vect){
	char c=UDR;
	if(c=='U'){//update needed?
       	wdt_enable(WDTO_15MS); //enable watchdog
       	while(1);//force a watchdog reset
	}
}

ISR(USART_TXC_vect){
	usart_send_next_ISR();
}


int main (void)
{
		unsigned char display_mode=0;
		unsigned char old_second=60;
		unsigned char i=0;
		char c[8];
		unsigned char TWI_buf[10];
		unsigned char min,hour,second,day,month,year,dow;
		
		wdt_enable(WDTO_15MS);

		GICR&=0x1F; //disable all external interrupts
		
		//load saved values from EEPROM
		display_set_dark_level(settings_get(SETTINGS_DARK_BRIGHTNESS));
		display_set_bright_level(settings_get(SETTINGS_LIGHT_BRIGHTNESS));
		ui_menues_set_cont_mode(settings_get(SETTINGS_CONT_MODE));
		ui_menues_set_alarm_mode(settings_get(SETTINGS_ALARM_MODE));
		ui_menues_set_alarm_hour(settings_get(SETTINGS_ALARM_HOUR));
		ui_menues_set_alarm_minute(settings_get(SETTINGS_ALARM_MINUTE));
		ui_menues_set_alarm_mp3_track(settings_get(SETTINGS_ALARM_MP3_TRACK));
		ui_menues_load_schedules();
		clock_set_dst_mode(settings_get(SETTINGS_DST_MODE));
		ui_menues_set_code((settings_get(SETTINGS_UI_MENUES_CODE_32)<<8)|settings_get(SETTINGS_UI_MENUES_CODE_10));
		
		sei();
		set_sleep_mode(SLEEP_MODE_IDLE);

		TWI_MasterSlave_Initialise();	//initialize the TWI/I2C interface
		ui_input_init();
		adc_init();
		display_init();
		beeper_init();
		dcf77_init();
		for(i=0;i<50;i++){//wait for slower I2C devices to power up themselves
			_delay_ms(5);wdt_reset();
		}
		I2C_init_modules();
		for(i=0;i<50;i++){//wait for slower I2C devices to initialize themselves
			_delay_ms(5);wdt_reset();
		}
		clock_init();
		
		OCR0=100-1; // 100 counts until compare match = every 50µs compare match = 20khz call freqeuency
		TCCR0=0x08|0x02;//CTC mode, prescaler 8 = 2Mhz
		TIMSK|=0x02;

		OCR2=125-1; // 125 counts until compare match = every 1ms compare match = 1khz call freqeuency
		TCCR2=0x08|0x05;//CTC mode, prescaler 128 = 125Khz
		TIMSK|=0x80;


		usart_init();
		
		
		//ui_display_modes_set_fixed_mode(7);
		//I2C_MP3_detected=0;


		while(1){
			sleep_mode();
			I2C_check_state_machines();
			switch(display_mode){
				case 0:if(ui_display_modes_version()==0){
							if(I2C_RTC_detected) {
								display_mode=2;//2=default
							}else{
								display_mode=1;
							}
						}
						break;
				case 1:	c[0]=' ';c[1]=' ';c[2]=' ';c[3]=' ';c[4]=' ';c[5]=' ';c[6]=' ';c[7]=' ';
						if(dcf77_get_signal()==0){
							c[3]=' ';//default 10
						}else{
							c[3]='.';//default 10
						}
						
						c[6]=dcf77_get_state()%10+48;
						if(dcf77_get_state()==DCF77_STATE_FOUND_SYNC_FULL){
							display_mode=2;
						}
						
						display_set_mode(DISPLAY_7SEG_DIM);
						display_set_time(&c[0]);
						break;
				case 2:	if(ui_menues_check_alarm()){
						}else{
							ui_display_mode();
						}
						if(TWI_Get_Data_From_Receiver(&TWI_buf[0])){
							if(TWI_buf[0]==0x01){//key code via I2C
								if(TWI_buf[1]&0x01){
									ui_input_simulate(UI_INPUT_KEY_BACK);
								}else if(TWI_buf[1]&0x02){
									ui_input_simulate(UI_INPUT_KEY_OK);
								}else if(TWI_buf[1]&0x04){
									ui_input_simulate(UI_INPUT_KEY_UP);
								}else if(TWI_buf[1]&0x08){
									ui_input_simulate(UI_INPUT_KEY_DOWN);
								}
								TWI_buf[0]=0;TWI_buf[1]=0;TWI_buf[2]=0;
							}
						}
						clock_get_time(&min,&hour,&second,&day,&month,&year,&dow);
						if(second!=old_second){
							if(second==0){
								if((I2C_MP3_detected)&&(ui_menues_get_cont_mode()==UI_MENUES_CONT_MODE_ON)){
									I2C_MP3_playCont();
								}							
							}
							old_second=second;
						}
						break;
			}
		}
}
