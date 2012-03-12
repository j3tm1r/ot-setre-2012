/*
 * ServiceES.c
 *
 *  Created on: 28 févr. 2012
 *      Author: mbbadau
 */
#include <string.h>

#include "ServiceOutput.h"
#include "Display.h"
#include "includes.h"
#include "util/cmdBuffer.h"

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
// Volume
static INT16U volumeMap[] = {
	0,
	32,
	64,
	96,
	128,
	160,
	192,
	224,
	255
};

// Bargraph
static INT16U bargraphMap[] = {
	0,
	0x01,
	0x03,
	0x07,
	0x0F,
	0x1F,
	0x3F,
	0x7F,
	0xFF
};


// Declarations
// Services
void  setFrequencyById(INT8U freqId);
void  setVolumeByLvl(INT8U volLvl);
INT8S setFrequency(INT16U nb);
INT8S setVolume(INT8U cmd, INT8U nb);
INT8S setBargraph(INT8U cmd);

// Helpers
INT8S sendOverSPI(INT8U target, INT16U data, INT8U nbits);

void ServiceOutput(void *parg) {

		OS_EVENT *msgQServiceOutput = (OS_EVENT*) parg;
		INT8U err;
		INT16S bufHandle;
		ServiceMsg* data;

		for (;;) {

			bufHandle = (INT16S) OSQPend (msgQServiceOutput, 0, &err);
			data = (ServiceMsg *) DeQueue(bufHandle);
			if (data == 0) {
				continue;
			}

			switch(data->serviceType) {
			case SERV_BARGRAPH:
				setBargraph(data->val); //Test avec valeurs fixes
				break;
			case SERV_EEPROM:
				break;
			case SERV_FREQ:
				setFrequencyById(data->val);
				break;
			case SERV_LCD:
				clearDisplay();
				char *str = data->msg.pBuffer;
				char screenBuffer[N_CHAR_PER_LINE * N_LINE + 2];	// count '\0' and '\n'
				UNUSED(screenBuffer);
				INT8U strLen = strlen(str);//data->msg.size;
				int i = 0;
				int j = 0;
				while(i < strLen + 1) {
					if(str[i] == '\n') {
//						if(i > N_CHAR_PER_LINE) {
//							// '\n' must be before end of first line
//							break;
//						}
						screenBuffer[j] = '\0';
						printString(screenBuffer);
						gotoSecondLine();
						j = 0;
					} else if (str[i] == '\0') {
						screenBuffer[j] = '\0';
						printString(screenBuffer);
					} else {
						screenBuffer[j] = str[i];
						++j;
					}
					++i;
				}
				break;
			case SERV_VOLUME:
				setVolumeByLvl(data->val);
				setBargraph(data->val);
				break;
			default:
				// Error
				break;
			}

		}

}

INT8S setBargraph(INT8U lvl) {

	if(lvl >= VOL_NUM) {
		return -1;
	}

	INT8U i;
	INT8U etatBargraph = bargraphMap[lvl];

	//Balayage et configuration de chaque LED du Bargraph
	for(i=0;i<8;i++) {
		//Assignation de l'adresse au contrôleur du Bargraph
		P6OUT &= 0b11110001;
		P6OUT |= i<<1;

		P6OUT |= 0x80;	//Pin Select à 1 pour sélectionner le Bargraph

		//Détermine si D doit être  à 1 ou 0
		P6OUT &= 0b11111110;
		P6OUT |= ((etatBargraph & (0b00000001 << i)) >> i);

		OSTimeDly(1);
		P6OUT &= 0x7F;	//Pin Select à 0 pour désélectionner le Bargraph
	}

	return 0;
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

	P6OUT &= 0x7F;	//Pin Select à 0 pour sélectionner la Fréquence

	//clearDisplay();
	//printDecimal(nb);
	//gotoSecondLine();
	//printHex(data);
	sendOverSPI(SPI_FREQ, data, 12);

	return 0;
}

void  setVolumeByLvl(INT8U volLvl) {
	INT16U volume;
	if (volLvl >= VOL_NUM ) {
		volume = volumeMap[VOL_NUM-1];
	} else {
		volume = volumeMap[volLvl];
	}
	setVolume(VOLUME_CMD, volume);
}

INT8S setVolume(INT8U cmd, INT8U nb) {

	P6OUT &= 0x7F;	//Pin Select à 0 pour sélectionner la Fréquence
	P6OUT |= 0x80;	//Pin Select à 1 pour sélectionner le Volume
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
