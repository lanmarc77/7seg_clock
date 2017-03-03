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

#ifndef I2C_MODULE_SE95_C
#define I2C_MODULES_E95_C
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#define SE95_get_temp 1
#define SE95_done 200
#define SE95_error 230
volatile unsigned char SE95_cmd=0;
volatile unsigned char SE95_state=0;
volatile signed int SE95_temp=99;
volatile signed int SE95_temp_frac=99;
unsigned char se95_messageBuf[25];


unsigned char SE95_detect(void){
	unsigned int i=0;
	//unsigned char messageBuf[25];

	se95_messageBuf[0]=(0x48<<TWI_ADR_BITS) | (FALSE<<TWI_READ_BIT);
	se95_messageBuf[1]=0x01;//configuration register
	se95_messageBuf[2]=0x00;//POR value=0x00
	TWI_Start_Transceiver_With_Data( se95_messageBuf, 3 );
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

unsigned char SE95_getTemp(signed int *temp){
	if(SE95_cmd==0){
		SE95_cmd=SE95_get_temp;
	}
	if(SE95_cmd==SE95_get_temp+SE95_done){
		SE95_cmd=0;
		*temp=SE95_temp*10;
		*temp+=((SE95_temp_frac*313)/100)/10;
		return 1;
	}
	if(SE95_cmd>SE95_error){
		SE95_cmd=0;
		return 1;
	}
	return 0;
}
#define SE95_I2C_ERROR_CNT_MAX 3
unsigned char SE95_i2c_error_cnt=0;
#define SE95_I2C_WAIT_CNT_MAX 15000
unsigned int SE95_i2c_wait_cnt=0;

char SE95_check_i2c_state_machine(void){

	switch(SE95_state){
		case 1:
		case 0:	SE95_i2c_wait_cnt=0;
				if(SE95_cmd==SE95_get_temp){
					se95_messageBuf[0]=(0x48<<TWI_ADR_BITS) | (FALSE<<TWI_READ_BIT);
					se95_messageBuf[1]=0x00;//setup reading from register 00 means temperature
					TWI_Start_Transceiver_With_Data( &se95_messageBuf[0], 2 );
					SE95_state=20;
				}
				break;
		case 20:if(!(TWI_Transceiver_Busy() )){
					if ( TWI_statusReg.lastTransOK ){
						se95_messageBuf[0]=(0x48<<TWI_ADR_BITS) | (TRUE<<TWI_READ_BIT);
						TWI_Start_Transceiver_With_Data( &se95_messageBuf[0], 3);
						SE95_state++;
						SE95_i2c_error_cnt=0;
					}else{
						SE95_i2c_error_cnt++;
						TWI_Master_Stop();TWI_MasterSlave_Initialise();I2CErrorCount++; //reset I2C module
						if(SE95_i2c_error_cnt>SE95_I2C_ERROR_CNT_MAX){//ok give up
							SE95_cmd+=SE95_error;
							SE95_state=0;
							SE95_i2c_error_cnt=0;
						}else{//retry same command from scratch
							SE95_state=1;
						}
					}
				}else{
					SE95_i2c_wait_cnt++;
					if(SE95_i2c_wait_cnt>SE95_I2C_WAIT_CNT_MAX){
						SE95_i2c_error_cnt++;
						TWI_Master_Stop();TWI_MasterSlave_Initialise();I2CErrorCount++; //reset I2C module
						if(SE95_i2c_error_cnt>SE95_I2C_ERROR_CNT_MAX){//ok give up
							SE95_state=0;
							SE95_cmd+=SE95_error;
							SE95_i2c_error_cnt=0;
						}else{//retry same command from scratch
							SE95_state=1;
						}
					}
				}
				break;
		case 21:if(!(TWI_Transceiver_Busy() )){
					if ( TWI_statusReg.lastTransOK ){
						TWI_Get_Data_From_Transceiver( &se95_messageBuf[0], 3 );
						cli();
						SE95_temp=((se95_messageBuf[1]<<8)|se95_messageBuf[2])>>3;
						SE95_temp_frac=(SE95_temp)&0x1F;
						SE95_temp>>=5;//reduce to 1 centigrade resolution
						sei();
						SE95_i2c_error_cnt=0;
						SE95_cmd+=SE95_done;
						SE95_state=0;
					}else{
						SE95_i2c_error_cnt++;
						TWI_Master_Stop();TWI_MasterSlave_Initialise();I2CErrorCount++; //reset I2C module
						if(SE95_i2c_error_cnt>SE95_I2C_ERROR_CNT_MAX){//ok give up
							SE95_cmd+=SE95_error;
							SE95_state=0;
							SE95_i2c_error_cnt=0;
						}else{//retry same command from scratch
							SE95_state=1;
						}
					}
				}else{
					SE95_i2c_wait_cnt++;
					if(SE95_i2c_wait_cnt>SE95_I2C_WAIT_CNT_MAX){
						SE95_i2c_error_cnt++;
						TWI_Master_Stop();TWI_MasterSlave_Initialise();I2CErrorCount++; //reset I2C module
						if(SE95_i2c_error_cnt>SE95_I2C_ERROR_CNT_MAX){//ok give up
							SE95_state=0;
							SE95_cmd+=SE95_error;
							SE95_i2c_error_cnt=0;
						}else{//retry same command from scratch
							SE95_state=1;
						}
					}
				}
				break;
	}
	return SE95_state;
}

#endif
