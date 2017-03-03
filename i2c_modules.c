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

#ifndef I2C_MODULES_C
#define I2C_MODULES_C
#include "config.h"
#include <avr/io.h>
#include "TWI_Master.h"

unsigned char I2CErrorCount=0;

#include "i2c_module_mp3.c"
#include "i2c_module_ds3231.c"
#include "i2c_module_se95.c"


unsigned char I2C_RTC_detected=0;
unsigned char I2C_TEMP_detected=0;
unsigned char I2C_RADIO_detected=0;
unsigned char I2C_MP3_detected=0;
unsigned char I2C_MOTION_detected=0;
volatile unsigned char I2C_busy=0;

unsigned char I2C_getBusy(void){
	return I2C_busy;
}

unsigned char I2C_getErrorCount(void){
	return I2CErrorCount;
}


void I2C_init_modules(void){
	if(SE95_detect()){
		I2C_TEMP_detected=1;
	}
	if(DS3231_detect()){
		I2C_RTC_detected=1;
		if(I2C_TEMP_detected==0){
			I2C_TEMP_detected=2;
		}
	}
	if(MP3_detect()){
		I2C_MP3_detected=1;
	}
}

void I2C_check_state_machines(void){
	if((I2C_TEMP_detected)&&((I2C_busy==0)||(I2C_busy==1))){
		if(SE95_check_i2c_state_machine()){
			I2C_busy=1;
		}else{
			I2C_busy=0;
		}
	}
	if((I2C_RTC_detected)&&((I2C_busy==0)||(I2C_busy==2))){
		if(DS3231_check_i2c_state_machine()){
			I2C_busy=2;
		}else{
			I2C_busy=0;
		}
	}
	if((I2C_MP3_detected)&&((I2C_busy==0)||(I2C_busy==3))){
		if(MP3_check_i2c_state_machine()){
			I2C_busy=3;
		}else{
			I2C_busy=0;
		}
	}	
}



/* MP3 */
char I2C_MP3_talkTime(void){
	return MP3_talkTime();
}

char I2C_MP3_playCont(void){
	return MP3_playCont();
}

char I2C_MP3_stopPlaying(void){
	return MP3_stopPlaying();
}

char I2C_MP3_playAlarm(unsigned char s){
	return MP3_playAlarm(s);
}

char I2C_MP3_playSched(unsigned char s){
	return MP3_playSched(s);
}

char I2C_MP3_playEvent(unsigned char s){
	return MP3_playEvent(s);
}

char I2C_MP3_playAmb(unsigned char s){
	return MP3_playAmb(s);
}

char I2C_MP3_setVol(unsigned char vol){
	return MP3_setVol(vol);
}

/* returns true if temperature was read, false otherwise */
char I2C_getTemp(signed int *temp){
	if(I2C_TEMP_detected==1){
		return SE95_getTemp(temp);
	}
	if(I2C_TEMP_detected==2){
		return DS3231_getTemp(temp);
	}
	return 0;
}

/* RTC */

char I2C_RTC_getTime(void){
	return DS3231_getTime();
}
char I2C_RTC_setTime(void){
	return DS3231_setTime();
}


#endif
