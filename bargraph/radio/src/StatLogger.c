/*
 * StatLogger.c
 *
 *  Created on: 28 févr. 2012
 *      Author: mbbadau
 */


#include "StatLogger.h"
#include "os_cfg.h"
#include "Display.h"
#include "includes.h"

void StatLogger(void *parg) {

	INT8U err;
	StatMsg *recvData;

	OS_EVENT *TI_To_GM_MsgQ = (OS_EVENT*) parg;

	for(;;) {

		recvData = (StatMsg*) OSQPend (TI_To_GM_MsgQ, 0, &err);

		gotoSecondLine();
		printString("SL :");
		printDecimal(recvData->volumeLvl);

		//OSTimeDly(OS_TICKS_PER_SEC);
	}
}
