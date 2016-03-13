/*****************************************************************************
*
* Atmel Corporation
*
* File              : TWI_Master.c
* Compiler          : AVR GCC 4.x
* Revision          : $Revision: 1.14 $
* Date              : $Date: 09. July 2013$
* Updated by        : $Author: ltwa $
* Updated by        : $Author: Marcel Langner, July 2013 $
*
* Support mail      : langner.marcel@myiq.de
*
* Supported devices : All devices with a TWI module can be used.
*
* AppNote           : AVR315 - TWI Master Implementation
*
* Description       : This is a sample driver for the TWI hardware modules.
*                     It is interrupt driven. All functionality is controlled through 
*                     passing information to and from functions. See main.c for samples
*                     of how to use the driver.
*
*
****************************************************************************/

#include <avr/io.h>
#include <avr/interrupt.h>
#include "TWI_Master.h"

static unsigned char TWI_buf[ TWI_BUFFER_SIZE ];    // Transceiver buffer

unsigned char TWI_RX_buf_ptr=0;    // Receiver buffer
unsigned char TWI_RX_buf[ TWI_RX_BUFFER_SIZE ];    // Receiver buffer

static unsigned char TWI_msgSize;                   // Number of bytes to be transmitted.
static unsigned char TWI_state = TWI_NO_STATE;      // State byte. Default set to TWI_NO_STATE.
#define TWI_MASTER 0
#define TWI_SLAVE 1
volatile unsigned char TWI_mode=TWI_MASTER;
union TWI_statusReg TWI_statusReg = {0};            // TWI_statusReg is defined in TWI_Master.h
volatile unsigned char TWI_busy=0;


/****************************************************************************
Call this function to set up the TWI master to its initial standby state.
Remember to enable interrupts from the main application after initializing the TWI.
****************************************************************************/
void TWI_MasterSlave_Initialise(void)
{
  TWI_mode=TWI_SLAVE;
  TWBR = TWI_TWBR;                                  // Set bit rate register (Baudrate). Defined in header file.
// TWSR = TWI_TWPS;                                  // Not used. Driver presumes prescaler to be 00.
  TWDR = 0xFF;                                      // Default content = SDA released.
  TWAR = 0x32;
  /*TWCR = (1<<TWEN)|                                 // Enable TWI-interface and release TWI pins.
         (0<<TWIE)|(0<<TWINT)|                      // Disable Interupt.
         (0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // No Signal requests.
         (0<<TWWC);                                 */
  TWCR = (1<<TWEN)|                                 // Enable TWI-interface and release TWI pins.
         (1<<TWIE)|(1<<TWINT)|                      // Disable Interupt.
         (1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // No Signal requests.
         (0<<TWWC);                                
}

/****************************************************************************
Call this function to stop the TWI module
****************************************************************************/
void TWI_Master_Stop(void)
{
  TWCR = (0<<TWEN)|                                 // Disable TWI-interface and release TWI pins.
         (0<<TWIE)|(0<<TWINT)|                      // Disable Interupt.
         (0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // No Signal requests.
         (0<<TWWC);                                 //
	TWI_busy=0;
}

/****************************************************************************
Call this function to test if the TWI_ISR is busy transmitting.
****************************************************************************/
unsigned char TWI_Transceiver_Busy( void )
{
  //return ( TWCR & (1<<TWIE) );                  // IF TWI Interrupt is enabled then the Transceiver is busy
  return (TWI_busy);
}

/****************************************************************************
Call this function to fetch the state information of the previous operation. The function will hold execution (loop)
until the TWI_ISR has completed with the previous operation. If there was an error, then the function 
will return the TWI State code. 
****************************************************************************/
unsigned char TWI_Get_State_Info( void )
{
  while ( TWI_Transceiver_Busy() );             // Wait until TWI has completed the transmission.
  return ( TWI_state );                         // Return error state.
}

/****************************************************************************
Call this function to send a prepared message. The first byte must contain the slave address and the
read/write bit. Consecutive bytes contain the data to be sent, or empty locations for data to be read
from the slave. Also include how many bytes that should be sent/read including the address byte.
The function will hold execution (loop) until the TWI_ISR has completed with the previous operation,
then initialize the next operation and return.
****************************************************************************/
void TWI_Start_Transceiver_With_Data( unsigned char *msg, unsigned char msgSize )
{
  unsigned char temp;
  while ( TWI_Transceiver_Busy() );             // Wait until TWI is ready for next transmission.
  TWI_mode=TWI_MASTER;

  TWI_msgSize = msgSize;                        // Number of data to transmit.
  TWI_buf[0]  = msg[0];                         // Store slave address with R/W setting.
  if (!( msg[0] & (TRUE<<TWI_READ_BIT) ))       // If it is a write operation, then also copy data.
  {
    for ( temp = 1; temp < msgSize; temp++ )
      TWI_buf[ temp ] = msg[ temp ];
  }
  TWI_statusReg.all = 0;      
  TWI_state         = TWI_NO_STATE ;
  TWI_busy=1;
  TWCR = (1<<TWEN)|                             // TWI Interface enabled.
         (1<<TWIE)|(1<<TWINT)|                  // Enable TWI Interupt and clear the flag.
         (0<<TWEA)|(1<<TWSTA)|(0<<TWSTO)|       // Initiate a START condition.
         (0<<TWWC);                             //
}

/****************************************************************************
Call this function to resend the last message. The driver will reuse the data previously put in the transceiver buffers.
The function will hold execution (loop) until the TWI_ISR has completed with the previous operation,
then initialize the next operation and return.
****************************************************************************/
void TWI_Start_Transceiver( void )
{
  while ( TWI_Transceiver_Busy() );             // Wait until TWI is ready for next transmission.
  TWI_mode=TWI_MASTER;
  TWI_statusReg.all = 0;      
  TWI_state         = TWI_NO_STATE ;
  TWI_busy=1;
  TWCR = (1<<TWEN)|                             // TWI Interface enabled.
         (1<<TWIE)|(1<<TWINT)|                  // Enable TWI Interupt and clear the flag.
         (0<<TWEA)|(1<<TWSTA)|(0<<TWSTO)|       // Initiate a START condition.
         (0<<TWWC);                             //
}

/****************************************************************************
Call this function to read out the requested data from the TWI transceiver buffer. I.e. first call
TWI_Start_Transceiver to send a request for data to the slave. Then Run this function to collect the
data when they have arrived. Include a pointer to where to place the data and the number of bytes
requested (including the address field) in the function call. The function will hold execution (loop)
until the TWI_ISR has completed with the previous operation, before reading out the data and returning.
If there was an error in the previous transmission the function will return the TWI error code.
****************************************************************************/
unsigned char TWI_Get_Data_From_Transceiver( unsigned char *msg, unsigned char msgSize )
{
  unsigned char i;

  while ( TWI_Transceiver_Busy() );             // Wait until TWI is ready for next transmission.

  if( TWI_statusReg.lastTransOK )               // Last transmission competed successfully.              
  {                                             
    for ( i=0; i<msgSize; i++ )                 // Copy data from Transceiver buffer.
    {
      msg[ i ] = TWI_buf[ i ];
    }
  }
  return( TWI_statusReg.lastTransOK );                                   
}

unsigned char TWI_Get_Data_From_Receiver( unsigned char *d){
	unsigned char i;
	cli();
	if((TWI_busy==0)&&(TWI_mode==TWI_SLAVE)&&(TWI_RX_buf_ptr)){
		for(i=0;i<TWI_RX_buf_ptr;i++){
			d[i]=TWI_RX_buf[i];
		}
		TWI_RX_buf_ptr=0;
		sei();
		return i-1;
	}else{
		sei();
		return 0;
	}

}

// ********** Interrupt Handlers ********** //
/****************************************************************************
This function is the Interrupt Service Routine (ISR), and called when the TWI interrupt is triggered;
that is whenever a TWI event has occurred. This function should not be called directly from the main
application.
****************************************************************************/
/*#pragma vector=TWI_vect
__interrupt void TWI_ISR(void)*/
ISR(TWI_vect)
{
  static unsigned char TWI_bufPtr;
  
  switch (TWSR)
  {
    case TWI_START:             // START has been transmitted  
    case TWI_REP_START:         // Repeated START has been transmitted
      TWI_bufPtr = 0;                                     // Set buffer pointer to the TWI Address location
    case TWI_MTX_ADR_ACK:       // SLA+W has been tramsmitted and ACK received
    case TWI_MTX_DATA_ACK:      // Data byte has been tramsmitted and ACK received
      if (TWI_bufPtr < TWI_msgSize)
      {
        TWDR = TWI_buf[TWI_bufPtr++];
        TWCR = (1<<TWEN)|                                 // TWI Interface enabled
               (1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interupt and clear the flag to send byte
               (0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           //
               (0<<TWWC);                                 //  
      }else                    // Send STOP after last byte
      {
        TWI_statusReg.lastTransOK = TRUE;                 // Set status bits to completed successfully. 
		TWI_busy=0;
		TWI_mode=TWI_SLAVE;
        TWCR = (1<<TWEN)|                                 // TWI Interface enabled
               (1<<TWIE)|(1<<TWINT)|                      // Disable TWI Interrupt and clear the flag
               (1<<TWEA)|(0<<TWSTA)|(1<<TWSTO)|           // Initiate a STOP condition.
               (0<<TWWC);                                
      }
      break;
    case TWI_MRX_DATA_ACK:      // Data byte has been received and ACK tramsmitted
      TWI_buf[TWI_bufPtr++] = TWDR;
    case TWI_MRX_ADR_ACK:       // SLA+R has been tramsmitted and ACK received
      if (TWI_bufPtr < (TWI_msgSize-1) )                  // Detect the last byte to NACK it.
      {
        TWCR = (1<<TWEN)|                                 // TWI Interface enabled
               (1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interupt and clear the flag to read next byte
               (1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // Send ACK after reception
               (0<<TWWC);                                 //  
      }else                    // Send NACK after next reception
      {
        TWCR = (1<<TWEN)|                                 // TWI Interface enabled
               (1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interupt and clear the flag to read next byte
               (0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // Send NACK after reception
               (0<<TWWC);                                 // 
      }    
      break; 
    case TWI_MRX_DATA_NACK:     // Data byte has been received and NACK tramsmitted
      TWI_buf[TWI_bufPtr] = TWDR;
      TWI_statusReg.lastTransOK = TRUE;                 // Set status bits to completed successfully. 
	  TWI_busy=0;
	  TWI_mode=TWI_SLAVE;
      TWCR = (1<<TWEN)|                                 // TWI Interface enabled
             (1<<TWIE)|(1<<TWINT)|                      // Disable TWI Interrupt and clear the flag
             (1<<TWEA)|(0<<TWSTA)|(1<<TWSTO)|           // Initiate a STOP condition.
             (0<<TWWC);                                
      break;      
    case TWI_ARB_LOST:          // Arbitration lost
      TWCR = (1<<TWEN)|                                 // TWI Interface enabled
             (1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interupt and clear the flag
             (0<<TWEA)|(1<<TWSTA)|(0<<TWSTO)|           // Initiate a (RE)START condition.
             (0<<TWWC);                                 //
      break;

	case TWI_SRX_ADR_DATA_ACK:  // Previously addressed with own SLA+W; data has been received; ACK has been returned
					TWI_busy=1;
					TWI_RX_buf[TWI_RX_buf_ptr]=TWDR;
					TWI_RX_buf_ptr=(TWI_RX_buf_ptr+1)%TWI_RX_BUFFER_SIZE;
					TWCR = (1<<TWEN)|                                 // Enable TWI-interface and release TWI pins
						 (1<<TWIE)|(1<<TWINT)|                      // Disable Interupt
						 (1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // No Signal requests
						 (0<<TWWC);                                 //
					
					break;
	case TWI_SRX_ADR_ACK: // Own SLA+W has been received ACK has been returned
					TWI_busy=1;
					TWI_RX_buf_ptr=0;
					TWCR = (1<<TWEN)|                                 // Enable TWI-interface and release TWI pins
						 (1<<TWIE)|(1<<TWINT)|                      // Disable Interupt
						 (1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // No Signal requests
						 (0<<TWWC);                                 //
					break;

    case TWI_SRX_ADR_DATA_NACK: // Previously addressed with own SLA+W; data has been received; NOT ACK has been returned
    case TWI_MTX_ADR_NACK:      // SLA+W has been tramsmitted and NACK received
    case TWI_MRX_ADR_NACK:      // SLA+R has been tramsmitted and NACK received    
    case TWI_MTX_DATA_NACK:     // Data byte has been tramsmitted and NACK received
//    case TWI_NO_STATE              // No relevant state information available; TWINT = “0”
    case TWI_BUS_ERROR:         // Bus error due to an illegal START or STOP condition
    default:     
      TWI_state = TWSR;                                 // Store TWSR and automatically sets clears noErrors bit.
      TWI_mode=TWI_SLAVE;	                            // Reset TWI Interface
	  TWI_busy=0;
      TWCR = (1<<TWEN)|                                 // Enable TWI-interface and release TWI pins
             (1<<TWIE)|(1<<TWINT)|                      // Disable Interupt
             (1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // No Signal requests
             (0<<TWWC);                                 //
  }
}
