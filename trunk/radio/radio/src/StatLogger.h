/*
 * StatLogger.h
 *
 *  Created on: 28 févr. 2012
 *      Author: mbbadau
 */

#ifndef STATLOGGER_H_
#define STATLOGGER_H_

#include <os_cpu.h>
#include <ucos_ii.h>

#define STAT_INIT	0
#define STAT_LOG	1
#define STAT_END	2

typedef struct task_SL_Param {
	OS_EVENT *TI_To_GM_MsgQ;
	OS_EVENT *GM_To_SL_MsgQ;
} task_SL_Param;

typedef struct StatMsg {
	INT16U msgType;
	union {
		INT16U volumeLvl;
		INT16U freq;
	};
} StatMsg;

void StatLogger(void *parg);

#endif /* STATLOGGER_H_ */
