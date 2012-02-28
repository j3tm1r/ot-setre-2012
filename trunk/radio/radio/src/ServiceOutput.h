/*
 * ServiceES.h
 *
 *  Created on: 28 févr. 2012
 *      Author: mbbadau
 */

#ifndef SERVICE_OUTPUT_H_
#define SERVICE_OUTPUT_H_

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


#endif /* SERVICE_OUTPUT_H_ */
