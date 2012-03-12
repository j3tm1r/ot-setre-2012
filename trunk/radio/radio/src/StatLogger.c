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
#include "GestionMode.h"
#include "util/cmdBuffer.h"

extern INT16S TI_To_GM_CmdBuf;

void StatLogger(void *parg) {

	INT8U err;
	INT16S bufHandle;
	StatMsg *recvData;
	InputCmd ackCmd;

	task_SL_Param *param = (task_SL_Param*) parg;
	OS_EVENT *TI_To_GM_MsgQ = param->TI_To_GM_MsgQ;
	OS_EVENT *GM_To_SL_MsgQ = param->GM_To_SL_MsgQ;

	for (;;) {

		bufHandle = (INT16S) OSQPend(GM_To_SL_MsgQ, 0, &err);
		recvData = (StatMsg *) DeQueue(bufHandle);
		if (recvData == 0) {
			continue;
		}

		if (recvData->msgType == STAT_INIT) {
			ackCmd.cmdID = MR_INIT_ACK;
			if (Queue(TI_To_GM_CmdBuf, &ackCmd) == 0) {
				OSQPost(TI_To_GM_MsgQ, (void *) TI_To_GM_CmdBuf);
			}

		} else if (recvData->msgType == STAT_END) {
			ackCmd.cmdID = MR_FIN_ACK;
			if (Queue(TI_To_GM_CmdBuf, &ackCmd) == 0) {
				OSQPost(TI_To_GM_MsgQ, (void *) TI_To_GM_CmdBuf);
			}
		} else {
			// TODO Log
		}

//		gotoSecondLine();
//		printString("SL :");
//		printDecimal(recvData->volumeLvl);
		/*
		 #define MR_INIT_ACK	4
		 #define MR_FIN_ACK	5
		 */

//OSTimeDly(OS_TICKS_PER_SEC);
	}
}
