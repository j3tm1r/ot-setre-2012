/*
 * StatLogger.c
 *
 *  Created on: 28 févr. 2012
 *      Author: mbbadau
 */
#include <ucos_ii.h>
#include <string.h>

#include "StatLogger.h"
#include "os_cfg.h"
#include "Display.h"
#include "includes.h"
#include "GestionMode.h"
#include "util/cmdBuffer.h"
#include "ServiceOutput.h"

extern INT16S TI_To_GM_CmdBuf;
extern INT16S GM_To_SO_CmdBuf;

static OS_EVENT *GM_To_SO_MsgQ;

static INT8S curVolLvl = -1;
static INT8S curFreq = -1;
static INT16U curSessionIdx = -1;

static INT16U lastTickFreq;
static INT16U lastTickVol;

extern OS_EVENT *TI_To_GM_MsgQ;
extern OS_EVENT *GM_To_SL_MsgQ;
extern OS_EVENT *GM_To_SO_MsgQ;
extern INT8U statLoggerPrio;

void updateStoredData();

void StatLogger(void *parg) {

	INT8U err;
	INT16S bufHandle;
	StatMsg *recvData;
	InputCmd ackCmd;
	ServiceMsg servMsg;
	StorageIndex storIndex;

	for (;;) {

		bufHandle = (INT16S) OSQPend(GM_To_SL_MsgQ,
				TIMEOUT_SEC * OS_TICKS_PER_SEC, &err);
		if (err == OS_TIMEOUT) {
			// Update stored data
			updateStoredData();
		} else {
			// Received a message
			recvData = (StatMsg *) DeQueue(bufHandle);

			if (recvData != 0 && recvData->msgType == STAT_INIT) {
				// Init
				ackCmd.cmdID = MR_INIT_ACK;
				if (Queue(TI_To_GM_CmdBuf, &ackCmd) == 0) {
					OSQPost(TI_To_GM_MsgQ, (void *) TI_To_GM_CmdBuf);
				}
				// Update time ref
				lastTickFreq = lastTickVol = OSTimeGet();

				// Update store index in EEPROM
				ReadEEPROM(0, &storIndex, sizeof(storIndex));
				// TODO /page += 64o
				curSessionIdx = storIndex.sessionNum;
				storIndex.sessionNum = curSessionIdx + 1;

				servMsg.serviceType = SERV_EEPROM;
				servMsg.msg.pBuffer = &storIndex;
				servMsg.msg.size = sizeof(storIndex);

				if (Queue(GM_To_SO_CmdBuf, &servMsg) == 0) {
					OSQPost(GM_To_SO_MsgQ, (void *) GM_To_SO_CmdBuf);
				}

				// Initialize session data in EEPROM
				Session curSession;
				memset(&curSession, 0, sizeof(curSession));
				servMsg.serviceType = SERV_EEPROM;
				servMsg.msg.pBuffer = &curSession;
				servMsg.msg.size = sizeof(curSession);

				if (Queue(GM_To_SO_CmdBuf, &servMsg) == 0) {
					OSQPost(GM_To_SO_MsgQ, (void *) GM_To_SO_CmdBuf);
				}
			} else if (recvData != 0 && recvData->msgType == STAT_END) {
				ackCmd.cmdID = MR_FIN_ACK;
				if (Queue(TI_To_GM_CmdBuf, &ackCmd) == 0) {
					OSQPost(TI_To_GM_MsgQ, (void *) TI_To_GM_CmdBuf);
				}
				// Update stored data
				updateStoredData();
				// Self-suspend
				OSTaskSuspend(statLoggerPrio);

			} else if (recvData != 0 && recvData->msgType == STAT_LOG) {
				// Update stored data
				curVolLvl = recvData->volumeLvl;
				curFreq = recvData->freq;
				updateStoredData();
			}
		}
	}
}

void updateStoredData() {
	if (curVolLvl == -1 || curFreq == -1) {
		return;
	}
	// Read current session from EEPROM
	ServiceMsg servMsg;
	Session curSession;

	INT16U curSessionAddr = sizeof(StorageIndex)
			+ curSessionIdx * sizeof(Session);
	ReadEEPROM(curSessionAddr, &curSession, sizeof(curSession));

	// Update values
	// Write back session to EEPROM
	if (curVolLvl >= 0 && curVolLvl < VOL_NUM - 2) {
		curSession.timePerVolLvl[0] += ((INT16U)OSTimeGet() - lastTickVol);
		lastTickVol = OSTimeGet();
	} else if (curVolLvl >= VOL_NUM - 2 && curVolLvl < VOL_NUM) {
		curSession.timePerVolLvl[1] += ((INT16U)OSTimeGet() - lastTickVol);
		lastTickVol = OSTimeGet();
	}
	curSession.timePerVolLvl[curFreq] += ((INT16U)OSTimeGet() - lastTickFreq);
	lastTickFreq = OSTimeGet();

	servMsg.serviceType = SERV_EEPROM;
	servMsg.msg.pBuffer = &curSession;
	servMsg.msg.size = sizeof(curSession);

	if (Queue(GM_To_SO_CmdBuf, &servMsg) == 0) {
		OSQPost(GM_To_SO_MsgQ, (void *) GM_To_SO_CmdBuf);
	}
}
