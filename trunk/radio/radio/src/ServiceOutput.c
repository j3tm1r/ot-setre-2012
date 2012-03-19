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
Station stationMap[] = { { 201, "80.1FM " }, { 255, "85.1FM " }, { 463,
		"95.2FM " }, { 503, "101.1FM" }, { 615, "105.1FM" }, { 765, "106.3FM" },
		{ 828, "107.1FM" }, };
// Volume
static INT16U volumeMap[] = { 0, 32, 64, 96, 128, 160, 192, 224, 255 };

// Helpers
INT8S sendOverSPI(INT8U target, INT16U data, INT8U nbits);


extern OS_CPU_SR cpu_sr;

static INT8U i, j;
static char screenBuffer[N_CHAR_PER_LINE * N_LINE + 2]; // count '\0' and '\n'

void PrintScreen(char *str) {
	OS_ENTER_CRITICAL();
	clearDisplay();
	i = 0;
	j = 0;
	while (i < strlen(str) + 1) {
		if (str[i] == '\n') {
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
	OS_EXIT_CRITICAL();
}

void ReadEEPROM(INT16U addr, void *buffer, INT8U size) {

	OS_ENTER_CRITICAL();
	INT8U i;
	for (i = 0; i < size; ++i) {
		*((INT8U *) buffer + i) = eeprom_random_read(addr + i);
	}
	OS_EXIT_CRITICAL();
}

void WriteEEPROM(INT16U addr, void *buffer, INT8U size) {
	// Retrieve EEPROM write address
	OS_ENTER_CRITICAL();
	INT8U i;
	for (i = 0; i < size; i++) {
		eeprom_byte_write(addr, ((INT8U *) buffer)[i]);
		++addr;
	}
	OS_EXIT_CRITICAL();
}

void PRINT_BAR(unsigned char segment, unsigned char etat) {
    SEL_ON;    
    if (segment> 8)    {segment=0;}
    if (etat>1)     {etat=0;}
    if (etat==1) D_ON;
    if (etat==0) D_OFF;
    switch (segment)
    {
        case (0): 
        {    S0_OFF;        //line selection 1/8 (msb) S2 S1 S0 (lsb)
            S1_OFF;
            S2_OFF;
            break;
        }
        case (1): 
        {    S0_ON;        //line selection 1/8 (msb) S2 S1 S0 (lsb)
            S1_OFF;
            S2_OFF;
            break;
        }
        case (2): 
        {    S0_OFF;        //line selection 1/8 (msb) S2 S1 S0 (lsb)
            S1_ON;
            S2_OFF;
            break;
        }
        case (3): 
        {    S0_ON;        //line selection 1/8 (msb) S2 S1 S0 (lsb)
            S1_ON;
            S2_OFF;
            break;
        }
        case (4): 
        {    S0_OFF;        //line selection 1/8 (msb) S2 S1 S0 (lsb)
            S1_OFF;
            S2_ON;
            break;
        }
        case (5): 
        {    S0_ON;        //line selection 1/8 (msb) S2 S1 S0 (lsb)
            S1_OFF;
            S2_ON;
            break;
        }
        case (6): 
        {    S0_OFF;        //line selection 1/8 (msb) S2 S1 S0 (lsb)
            S1_ON;
            S2_ON;
            break;
        }
        case (7): 
        {    S0_ON;        //line selection 1/8 (msb) S2 S1 S0 (lsb)
            S1_ON;
            S2_ON;
            break;
        }
    
        default: break;
    }
    SEL_OFF;    //LATCH
    SEL_ON;    
}
void RISE_BAR(void) {
    unsigned char i=0;
    unsigned char led_on=1;
    unsigned char led_off=0;
    for (i=0; i<8 ;i++)
    {
        PRINT_BAR(i,led_on);
        Delayx100us(200);
        PRINT_BAR(i,led_off);
    }
}
void FALL_BAR(void) {
    unsigned char i=0;
    unsigned char led_on=1;
    unsigned char led_off=0;
    for (i=7; i>0 ;i--)
    {
        PRINT_BAR(i,led_on);
        Delayx100us(200);
        PRINT_BAR(i,led_off);
    }
}


INT8S SetBargraph(INT8U lvl) {

    if (lvl >= VOL_NUM) {
        return -1;
    }


    for(i=0;i<8;i++)
    {
       PRINT_BAR( i, 0);
    }
    
    for(i=0;i<lvl;i++)
    {
       PRINT_BAR( i, 1);
    }    //Balayage et configuration de chaque LED du Bargraph

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

void SetFreqById(INT16U val) {
	OS_ENTER_CRITICAL();
	setFrequency(stationMap[val % FREQ_NUM].freqCarte);
	OS_EXIT_CRITICAL();
}

void SetVolumeByLvl(INT16U val) {
	OS_ENTER_CRITICAL();
	if (val >= VOL_NUM) {
		setVolume(VOLUME_CMD, volumeMap[VOL_NUM - 1]);
	} else {
		setVolume(VOLUME_CMD, volumeMap[val]);
	}
	OS_EXIT_CRITICAL();
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
