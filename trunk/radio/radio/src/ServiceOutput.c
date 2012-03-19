/*
 * ServiceES.c
 *
 *  Created on: 28 févr. 2012
 *      Author: mbbadau
 */
#include <string.h>

#include <os_cpu.h>
#include "ServiceOutput.h"
#include "Display.h"
#include "includes.h"
#include "util/cmdBuffer.h"
#include "ServicesES/drv_eeprom.h"

#define		SPI_FREQ	0
#define 	SPI_VOL		1

#define		VOLUME_CMD	31

// Definitions
// Frequency
static INT16U stationMap[] = { 201, 255, 463, 503, 615, 765, 828 };
// Volume
static INT16U volumeMap[] = { 0, 32, 64, 96, 128, 160, 192, 224, 255 };

// Declarations
// Services
void setFrequencyById(INT8U freqId);
void setVolumeByLvl(INT8U volLvl);
INT8S setFrequency(INT16U nb);
INT8S setVolume(INT8U cmd, INT8U nb);
INT8S setBargraph(INT8U cmd);

// Helpers
INT8S sendOverSPI(INT8U target, INT16U data, INT8U nbits);

extern OS_EVENT *GM_To_SO_MsgQ;
extern INT16S 	GM_To_SO_CmdBuf;

static INT8U err;
static ServiceMsg* data;
static OS_CPU_SR  cpu_sr;
static INT8U i, j;

void ServiceOutput(void *parg) {

	for (;;) {

		OSQPend(GM_To_SO_MsgQ, 0, &err);
		data = (ServiceMsg *) DeQueue(GM_To_SO_CmdBuf);
		if (data == 0) {
			continue;
		}

		OS_ENTER_CRITICAL();
		switch (data->serviceType) {
		case SERV_BARGRAPH:
			setBargraph(data->val); //Test avec valeurs fixes
			break;
		case SERV_EEPROM:
			{
				// Retrieve EEPROM write address
				INT16U baseAddr = *((INT16U *)data->msg.pBuffer);
				INT8U *dataToSend = data->msg.pBuffer + sizeof(INT16U);
				INT16U k;
				for (k = 0; k < data->msg.size - sizeof(INT16U); k++) {
//					clearDisplay();
//					printDecimal(baseAddr);
//					gotoSecondLine();
//					printDecimal(dataToSend[k]);
//					OSTimeDly(100);
					eeprom_byte_write(baseAddr, dataToSend[k]);
					++baseAddr;
				}
			}
			break;
		case SERV_FREQ:
			setFrequency( stationMap[data->val % FREQ_NUM] );
			break;
		case SERV_LCD:
			clearDisplay();
			char *str = data->msg.pBuffer;
			char screenBuffer[N_CHAR_PER_LINE * N_LINE + 2]; // count '\0' and '\n'

			i = 0; j = 0;
			while (i < strlen(str) + 1) {
				if (str[i] == '\n') {
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
			if (data->val >= VOL_NUM) {
				setVolume(VOLUME_CMD, volumeMap[VOL_NUM - 1]);
			} else {
				setVolume(VOLUME_CMD, volumeMap[data->val]);
			}
			//setBargraph(data->val);
			break;
		default:
			// Error
			break;
		}
		OS_EXIT_CRITICAL();

	}

}

void ReadEEPROM(INT16U addr, void *buffer, INT8U size) {

	OS_ENTER_CRITICAL();
	for(i = 0; i<size; ++i) {
		*((INT8U *)buffer + i) = eeprom_random_read(addr+i);
	}
	OS_EXIT_CRITICAL();
}

INT8S setBargraph(INT8U lvl) {

	if(lvl >= VOL_NUM) {
		return -1;
	}

	//Balayage et configuration de chaque LED du Bargraph
	for(i=0;i<8;i++) {

		//Assignation de l'adresse au contrôleur du Bargraph
		P6OUT &= 0b01110001;
		P6OUT |= i<<1;

		//Détermine si D doit être  à 1 ou 0
		if(i < lvl)
			P6OUT |= 0x01;
		else
			P6OUT &= 0b11111110;

		P6OUT |= 0x80;	//Pin Select à 1 pour sélectionner le Bargraph
		OSTimeDly(1);
		P6OUT &= 0x7F;	//Pin Select à 0 pour désélectionner le Bargraph
	}

	return 0;
}

INT8S setFrequency(INT16U nb) {

	if (nb > 1023) {
		return -1;
	}

	INT16U data = (nb << 2) & 0x0fff; // "create" dummy bits and select lowest 12 bits

	P6OUT &= 0x7F; //Pin Select à 0 pour sélectionner la Fréquence

	//clearDisplay();
	//printDecimal(nb);
	//gotoSecondLine();
	//printHex(data);
	sendOverSPI(SPI_FREQ, data, 12);

	return 0;
}

INT8S setVolume(INT8U cmd, INT8U nb) {

	P6OUT &= 0x7F; //Pin Select à 0 pour sélectionner la Fréquence
	P6OUT |= 0x80; //Pin Select à 1 pour sélectionner le Volume
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
	} else if (target == SPI_VOL) {
		SEL_ON;
	} else {
		return -1;
	}

	// Three state gate : disable and re-enable output
	CS_ON;
	Delayx100us(1);
	CS_OFF;

	// i // currentBit
	j = nbits;
	while (j > 0) {
		i = (data >> (j - 1)) & 0x1;
		if (i) {
			// currentBit = 1
			P6OUT |= 0x10; // DIN=1 	SCLK=0	 CS=0
			P6OUT |= 0x20; // SCLK=1
			P6OUT &= ~0x20; // SCLK=0
		} else {
			// currentBit = 0
			P6OUT &= ~0x70; // DIN=0 	SCLK=0	 CS=0
			P6OUT |= 0x20; // SCLK=1
			P6OUT &= ~0x20; // SCLK=0
		}
		--j;
	}

	// Three state gate : disable output
	CS_ON;
	//TODO OSTimeDly delay(200,10);

	return 0;
}
