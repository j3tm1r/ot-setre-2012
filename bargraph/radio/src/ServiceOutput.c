/*
 * ServiceES.c
 *
 *  Created on: 28 févr. 2012
 *      Author: mbbadau
 */

#include "ServiceOutput.h"
#include "Display.h"
#include "includes.h"

#define		SPI_FREQ	0
#define 	SPI_VOL		1

#define		VOLUME_CMD	31

// Definitions
// Frequency
static INT16U stationMap[] = {
	201,
	255,
	463,
	503,
	615,
	765,
	828
};


// Declarations
// Services
void  setFrequencyById(INT8U freqId);
INT8S setFrequency(INT16U nb);
INT8S setVolume(INT8U cmd, INT8U nb);

// Helpers
INT8S sendOverSPI(INT8U target, INT16U data, INT8U nbits);

void ServiceOutput(void *parg) {

		OS_EVENT *msgQServiceOutput = (OS_EVENT*) parg;
		INT8U err;
		ServiceMsg* data;

		for (;;) {


			data = (ServiceMsg*) OSQPend (msgQServiceOutput, 0, &err);

			switch(data->serviceType) {
			case SERV_BARGRAPH:
				break;
			case SERV_EEPROM:
				break;
			case SERV_FREQ:
				setFrequencyById(data->val);
				break;
			case SERV_LCD:
				clearDisplay();
				//gotoSecondLine();
				// Parse string '\n' gotoSecondLine() TODO
				printString("SO:");
				printDecimal(data->val);

				char *str = data->msg.pBuffer;
				char screenBuffer[N_CHAR_PER_LINE * N_LINE + 2];	// count '\0' and '\n'
				INT8U strLen = data->msg.size;
				int i = 0;
				while(i < strLen) {
					if(str[i] != '\n') {
						screenBuffer[i] = str[i];
					}
				}

				//printString(data->msg.pBuffer);
				break;
			case SERV_VOLUME:
				//setVolume(VOLUME_CMD, data->val);
				clearDisplay();
				printString("SO :");
				printDecimal(data->val);
				break;
			default:
				// Error
				break;
			}

		}

}

void  setFrequencyById(INT8U freqId) {

	INT16U stationFreq = stationMap[ freqId % FREQ_NUM];
	setFrequency(stationFreq);
}

INT8S setFrequency(INT16U nb) {

	if(nb>1023) {
		return -1;
	}

	INT16U data = (nb << 2) & 0x0fff; // "create" dummy bits and select lowest 12 bits

	clearDisplay();
	printDecimal(nb);
	gotoSecondLine();
	printHex(data);
	sendOverSPI(SPI_FREQ, data, 12);

	return 0;
}

INT8S setVolume(INT8U cmd, INT8U nb) {

	INT16U data = (cmd << 8) | nb;
	// Send command & data
	sendOverSPI(SPI_VOL, data, 16);

	return 0;
}

/////////////////////////////////////////////////////////////////
// HELPERS /////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

// nbits=[1,15], nbits de poids faible
INT8S sendOverSPI(INT8U target, INT16U data, INT8U nbits) {
	// Select SPI path
	if (target == SPI_FREQ) {
		SEL_OFF;
	} else if(target == SPI_VOL) {
		SEL_ON;
	} else {
		return -1;
	}

	// Three state gate : disable and re-enable output
	CS_ON;
	Delayx100us(1);
	CS_OFF;

	INT8U currentBit;
	INT8U k = nbits;
	while (k > 0)
	{
		currentBit = (data >> (k-1)) & 0x1;
		if (currentBit)
		{
			// currentBit = 1
			P6OUT |=0x10;	// DIN=1 	SCLK=0	 CS=0
			P6OUT |=0x20;	// SCLK=1
			P6OUT &=~0x20;	// SCLK=0
		}
		else
		{
			// currentBit = 0
			P6OUT &=~0x70;	// DIN=0 	SCLK=0	 CS=0
			P6OUT |=0x20;	// SCLK=1
			P6OUT &=~0x20;	// SCLK=0
		}
		--k;
	}

	// Three state gate : disable output
	CS_ON;
	//TODO OSTimeDly delay(200,10);

	/*// Deselect SPI path
	if (target == SPI_FREQ) {
		SEL_ON;
	} else if(target == SPI_VOL) {
		SEL_OFF;
	}*/

	return 0;
}
