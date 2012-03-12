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
#define SERV_EEPROM		1
#define SERV_FREQ		2
#define SERV_LCD		3
#define SERV_VOLUME		4

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


void ServiceOutput(void *parg);

// Volume
#define FREQ_NUM		7
#define DEFAULT_FREQ_ID	0	// [0..6]

#define VOL_NUM		9
#define DEFAULT_VOL_LVL	3	// [0..8]


#endif /* SERVICE_OUTPUT_H_ */