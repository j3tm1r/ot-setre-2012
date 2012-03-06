/*
 * StatLogger.h
 *
 *  Created on: 28 févr. 2012
 *      Author: mbbadau
 */

#ifndef STATLOGGER_H_
#define STATLOGGER_H_

#include <os_cpu.h>

#define STAT_INIT	0
#define STAT_LOG	1
#define STAT_END	2

typedef struct StatMsg {
	INT16U msgType;
	union {
		INT16U volumeLvl;
		INT16U freq;
	};
} StatMsg;

void StatLogger(void *parg);

#endif /* STATLOGGER_H_ */
