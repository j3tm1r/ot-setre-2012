/*
 * ServiceES.h
 *
 *  Created on: 28 févr. 2012
 *      Author: mbbadau
 */

#ifndef SERVICE_OUTPUT_H_
#define SERVICE_OUTPUT_H_

#include <os_cpu.h>

#define SERV_BARGRAPH	0
#define SERV_FREQ		1
#define SERV_LCD		2
#define SERV_VOLUME		3

// Volume
#define FREQ_NUM		7
#define DEFAULT_FREQ_ID	0	// [0..6]

#define VOL_NUM		9
#define DEFAULT_VOL_LVL	3	// [0..8]

// ServiceES asynchrone
typedef struct ServiceMsg {
	INT16U serviceType;
	union {
		INT16U val;
		struct {
			void *pBuffer;
			INT16U size;
		} msg;
	};

} ServiceMsg;

typedef struct StorageIndex {
	INT16U dataOffset;
	INT16U sessionNum;
	//INT16U dummy[2]; // just in case
} StorageIndex;

// 40o
typedef struct Session {
	INT16U timePerFreq[FREQ_NUM];
	INT16U timePerVolLvl[2];
} Session;

typedef struct Station {
	INT16U 	freqCarte;
	char 	*freqReel;
} Station;

void PrintScreen(char *str);
void SetFreqById(INT16U val);
void SetVolumeByLvl(INT16U val);
INT8S SetBargraph(INT8U lvl);
void ReadEEPROM(INT16U addr, void *buffer, INT8U size);
void WriteEEPROM(INT16U addr, void *buffer, INT8U size);


#endif /* SERVICE_OUTPUT_H_ */
