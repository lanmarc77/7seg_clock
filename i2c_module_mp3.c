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

#ifndef I2C_MODULE_MODULE_C
#define I2C_MODULE_MODULE_C
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "clock.h"

unsigned char MP3_messageBuf[25];

volatile unsigned char MP3_cmd=0;
volatile unsigned char MP3_state=0;
#define MP3_play_alarm 1
#define MP3_play_event 2
#define MP3_play_sched 3
#define MP3_play_amb 4
#define MP3_talk_time 5
#define MP3_play_cont 6
#define MP3_set_volume 7
#define MP3_stop 8
#define MP3_reset 9

#define MP3_done 200
#define MP3_error 230
volatile unsigned char MP3_cmd;
volatile unsigned char MP3_track=0;
volatile unsigned char MP3_volume=0;

    //00: NOP
    //01: get state (initial states, is playing)
    //02: play a specific alarm sound (01...99), if MSB is set repeat the song
    //03: play a specific schedule sound (01...99), if MSB is set repeat the song
    //04: play a specific event sound (01...99), if MSB is set repeat the song
    //05: play a specific ambient sound (01...99), if MSB is set repeat the song
    //06: setup volume (0...255)
    //07: stop playing whatever you playing (any value)
    //08: setup the time (hour setup) for talking
    //09: setup the time (minute setup) for talking, starts talking after setup

    //0A: setup the time (hour+dow setup) for continous play (binary value dow dow dow h h h h h)
    //0B: setup the time (day setup) for continous play
    //0C: setup the time (month setup) for continous play
    //0D: setup the time (minute setup) for continous play, starts playing after setup


    //10: setup play timer seconds (00:00 means infinite)
    //11: setup play timer minutes (00:00 means infinite)

    //AA: WDT reset module (for any value written)


unsigned char MP3_detect(void){
	unsigned int i=0;
	//unsigned char messageBuf[25];

	MP3_messageBuf[0]=(0x22<<TWI_ADR_BITS) | (FALSE<<TWI_READ_BIT);
	MP3_messageBuf[1]=0xAA; // RESET module command
	MP3_messageBuf[2]=0x01;
	TWI_Start_Transceiver_With_Data( MP3_messageBuf, 3 );
	do{sleep_mode();i++;if(i==0){break;}}while( TWI_Transceiver_Busy() );
	if ((!( TWI_statusReg.lastTransOK ))||(i==0)){
		//error occured
		//TWI_Get_State_Info( );   //check the error value/last TWI state and act accordingly, error codes are defined in the header
		TWI_Master_Stop();
		return 0;
	}else{
		return 1;
	}
}

unsigned char MP3_talkTime(void){
	if(MP3_cmd==0){
		MP3_cmd=MP3_talk_time;
	}
	if(MP3_cmd==MP3_talk_time+MP3_done){
		MP3_cmd=MP3_talk_time;
		return 1;
	}
	if(MP3_cmd>MP3_error){
		MP3_cmd=0;
		return 1;
	}
	if(MP3_cmd>MP3_done){
		MP3_cmd=MP3_talk_time;
		return 1;
	}
	return 0;
}

unsigned char MP3_stopPlaying(void){
	if(MP3_cmd==0){
		MP3_cmd=MP3_stop;
	}
	if(MP3_cmd==MP3_stop+MP3_done){
		MP3_cmd=MP3_stop;
		return 1;
	}
	if(MP3_cmd>MP3_error){
		MP3_cmd=0;
		return 1;
	}
	if(MP3_cmd>MP3_done){
		MP3_cmd=MP3_stop;
		return 1;
	}
	return 0;
}

unsigned char MP3_playAlarm(unsigned char s){
	if(MP3_cmd==0){//alarm has always priority
		MP3_track=s;
		MP3_cmd=MP3_play_alarm;
		
	}
	if(MP3_cmd==MP3_play_alarm+MP3_done){
		MP3_track=s;
		MP3_cmd=MP3_play_alarm;
		return 1;
	}
	if(MP3_cmd>MP3_error){
		MP3_cmd=0;
		return 1;
	}
	if(MP3_cmd>MP3_done){
		MP3_track=s;
		MP3_cmd=MP3_play_alarm;
		return 1;
	}
	return 0;
}


unsigned char MP3_playSched(unsigned char s){
	if(MP3_cmd==0){//alarm has always priority
		MP3_track=s;
		MP3_cmd=MP3_play_sched;
		
	}
	if(MP3_cmd==MP3_play_sched+MP3_done){
		MP3_track=s;
		MP3_cmd=MP3_play_sched;
		return 1;
	}
	if(MP3_cmd>MP3_error){
		MP3_cmd=0;
		return 1;
	}
	if(MP3_cmd>MP3_done){
		MP3_track=s;
		MP3_cmd=MP3_play_sched;
		return 1;
	}
	return 0;
}

unsigned char MP3_playEvent(unsigned char s){
	if(MP3_cmd==0){//alarm has always priority
		MP3_track=s;
		MP3_cmd=MP3_play_event;
		
	}
	if(MP3_cmd==MP3_play_event+MP3_done){
		MP3_track=s;
		MP3_cmd=MP3_play_event;
		return 1;
	}
	if(MP3_cmd>MP3_error){
		MP3_cmd=0;
		return 1;
	}
	if(MP3_cmd>MP3_done){
		MP3_track=s;
		MP3_cmd=MP3_play_event;
		return 1;
	}
	return 0;
}

unsigned char MP3_playAmb(unsigned char s){
	if(MP3_cmd==0){//alarm has always priority
		MP3_track=s;
		MP3_cmd=MP3_play_amb;
		
	}
	if(MP3_cmd==MP3_play_amb+MP3_done){
		MP3_track=s;
		MP3_cmd=MP3_play_amb;
		return 1;
	}
	if(MP3_cmd>MP3_error){
		MP3_cmd=0;
		return 1;
	}
	if(MP3_cmd>MP3_done){
		MP3_track=s;
		MP3_cmd=MP3_play_amb;
		return 1;
	}
	return 0;
}


unsigned char MP3_playCont(void){
	if(MP3_cmd==0){
		MP3_cmd=MP3_play_cont;
	}
	if(MP3_cmd==MP3_play_cont+MP3_done){
		MP3_cmd=MP3_play_cont;
		return 1;
	}
	if(MP3_cmd>MP3_error){
		MP3_cmd=0;
		return 1;
	}
	if(MP3_cmd>MP3_done){
		MP3_cmd=MP3_play_cont;
		return 1;
	}
	return 0;
}


unsigned char MP3_setVol(unsigned char vol){
	if(MP3_cmd==0){
		MP3_cmd=MP3_set_volume;
		MP3_volume=vol;
	}
	if(MP3_cmd==MP3_set_volume+MP3_done){
		MP3_cmd=MP3_set_volume;
		MP3_volume=vol;
		return 1;
	}
	if(MP3_cmd>MP3_error){
		MP3_cmd=0;
		return 1;
	}
	if(MP3_cmd>MP3_done){
		MP3_cmd=MP3_set_volume;
		MP3_volume=vol;
		return 1;
	}
	return 0;
}


char MP3_check_i2c_state_machine(void){
	unsigned char min,hour,second,day,month,year,dow;
	clock_get_time(&min,&hour,&second,&day,&month,&year,&dow);
	switch(MP3_state){
		case 0:	if(MP3_cmd==MP3_stop){
					MP3_messageBuf[0]=(0x22<<TWI_ADR_BITS) | (FALSE<<TWI_READ_BIT);
					MP3_messageBuf[1]=0x07;
					MP3_messageBuf[2]=1;
					TWI_Start_Transceiver_With_Data( &MP3_messageBuf[0], 3 );
					MP3_state=10;
				}
				if(MP3_cmd==MP3_play_alarm){
					MP3_messageBuf[0]=(0x22<<TWI_ADR_BITS) | (FALSE<<TWI_READ_BIT);
					MP3_messageBuf[1]=0x06;
					MP3_messageBuf[2]=0;
					TWI_Start_Transceiver_With_Data( &MP3_messageBuf[0], 3 );
					MP3_state=20;
				}
				if(MP3_cmd==MP3_play_cont){
					MP3_messageBuf[0]=(0x22<<TWI_ADR_BITS) | (FALSE<<TWI_READ_BIT);
					MP3_messageBuf[1]=0x0A;//write the current hour+dow
					MP3_messageBuf[2]=hour|((dow-1)<<5);
					TWI_Start_Transceiver_With_Data( &MP3_messageBuf[0], 3 );
					MP3_state=30;
				}
				if(MP3_cmd==MP3_talk_time){
					MP3_messageBuf[0]=(0x22<<TWI_ADR_BITS) | (FALSE<<TWI_READ_BIT);
					MP3_messageBuf[1]=0x08;//write the current hour
					MP3_messageBuf[2]=hour;
					TWI_Start_Transceiver_With_Data( &MP3_messageBuf[0], 3 );
					MP3_state=40;
				}
				if(MP3_cmd==MP3_play_event){
					MP3_messageBuf[0]=(0x22<<TWI_ADR_BITS) | (FALSE<<TWI_READ_BIT);
					MP3_messageBuf[1]=0x04;
					MP3_messageBuf[2]=MP3_track;
					TWI_Start_Transceiver_With_Data( &MP3_messageBuf[0], 3 );
					MP3_state=10;
				}
				if(MP3_cmd==MP3_play_sched){
					MP3_messageBuf[0]=(0x22<<TWI_ADR_BITS) | (FALSE<<TWI_READ_BIT);
					MP3_messageBuf[1]=0x03;
					MP3_messageBuf[2]=MP3_track;
					TWI_Start_Transceiver_With_Data( &MP3_messageBuf[0], 3 );
					MP3_state=10;
				}
				if(MP3_cmd==MP3_play_amb){
					MP3_messageBuf[0]=(0x22<<TWI_ADR_BITS) | (FALSE<<TWI_READ_BIT);
					MP3_messageBuf[1]=0x05;
					MP3_messageBuf[2]=MP3_track;
					TWI_Start_Transceiver_With_Data( &MP3_messageBuf[0], 3 );
					MP3_state=10;
				}
				if(MP3_cmd==MP3_set_volume){
					MP3_messageBuf[0]=(0x22<<TWI_ADR_BITS) | (FALSE<<TWI_READ_BIT);
					MP3_messageBuf[1]=0x06;
					MP3_messageBuf[2]=MP3_volume;
					TWI_Start_Transceiver_With_Data( &MP3_messageBuf[0], 3 );
					MP3_state=10;
				}
				break;
		case 10:if(!(TWI_Transceiver_Busy() )){
					if ( TWI_statusReg.lastTransOK ){
					}
					MP3_cmd=0;
					MP3_state=0;
				}
				break;	
		case 20:if(!(TWI_Transceiver_Busy() )){
					if ( TWI_statusReg.lastTransOK ){
						MP3_messageBuf[0]=(0x22<<TWI_ADR_BITS) | (FALSE<<TWI_READ_BIT);
						MP3_messageBuf[1]=0x02;
						MP3_messageBuf[2]=MP3_track;
						TWI_Start_Transceiver_With_Data( &MP3_messageBuf[0], 3 );
						MP3_state=10;
					}else{
						MP3_cmd=0;
						MP3_state=0;
					}
				}
				break;	
		case 30:if(!(TWI_Transceiver_Busy() )){
					if ( TWI_statusReg.lastTransOK ){
						MP3_messageBuf[0]=(0x22<<TWI_ADR_BITS) | (FALSE<<TWI_READ_BIT);
						MP3_messageBuf[1]=0x0B;//write the current day
						MP3_messageBuf[2]=day;
						TWI_Start_Transceiver_With_Data( &MP3_messageBuf[0], 3 );
						MP3_state=31;
					}else{
						//MP3_cmd+=MP3_done;
						MP3_cmd=0;
						MP3_state=0;
					}
				}
				break;
		case 31:if(!(TWI_Transceiver_Busy() )){
					if ( TWI_statusReg.lastTransOK ){
						MP3_messageBuf[0]=(0x22<<TWI_ADR_BITS) | (FALSE<<TWI_READ_BIT);
						MP3_messageBuf[1]=0x0C;//write the current month
						MP3_messageBuf[2]=month;
						TWI_Start_Transceiver_With_Data( &MP3_messageBuf[0], 3 );
						MP3_state=32;
					}else{
						//MP3_cmd+=MP3_done;
						MP3_cmd=0;
						MP3_state=0;
					}
				}
				break;
		case 32:if(!(TWI_Transceiver_Busy() )){
					if ( TWI_statusReg.lastTransOK ){
						MP3_messageBuf[0]=(0x22<<TWI_ADR_BITS) | (FALSE<<TWI_READ_BIT);
						MP3_messageBuf[1]=0x0D;//write the current minute
						MP3_messageBuf[2]=min;
						TWI_Start_Transceiver_With_Data( &MP3_messageBuf[0], 3 );
						MP3_state=33;
					}else{
						//MP3_cmd+=MP3_done;
						MP3_cmd=0;
						MP3_state=0;
					}
				}
				break;
		case 33:if(!(TWI_Transceiver_Busy() )){
					if ( TWI_statusReg.lastTransOK ){
					}
					MP3_cmd=0;
					MP3_state=0;
				}
				break;	
		case 40:if(!(TWI_Transceiver_Busy() )){
					if ( TWI_statusReg.lastTransOK ){
						MP3_messageBuf[0]=(0x22<<TWI_ADR_BITS) | (FALSE<<TWI_READ_BIT);
						MP3_messageBuf[1]=0x09;//write the current minute
						MP3_messageBuf[2]=min;
						TWI_Start_Transceiver_With_Data( &MP3_messageBuf[0], 3 );
						//MP3_cmd+=MP3_done;
						MP3_cmd=0;
						MP3_state=41;
					}else{
						//MP3_cmd+=MP3_done;
						MP3_cmd=0;
						MP3_state=0;
					}
				}
				break;
		case 41:if(!(TWI_Transceiver_Busy() )){
					if ( TWI_statusReg.lastTransOK ){
					}
					MP3_cmd=0;
					MP3_state=0;
				}
				break;	
	}
	return MP3_state;
}
#endif
