/*
 * TraitementInput.c
 *
 *  Created on: 28 févr. 2012
 *      Author: mbbadau
 */

#include "TraitementInput.h"
#include "GestionMode.h"
#include "ServicesES/Display.h"

#include "util/cmdBuffer.h"

extern INT8S TI_To_GM_CmdBuf;

void TraitementInput(void *parg) {
	INT8U err;
	InputCmd *cmd;

	task_TI_Param *param = (task_TI_Param*) parg;
	OS_EVENT *ISR_To_TI_MsgQ = param->ISR_To_TI_MsgQ;
	OS_EVENT *TI_To_GM_MsgQ  = param->TI_To_GM_MsgQ;

	for (;;) {

		InputEvent *event = (InputEvent*) OSQPend(ISR_To_TI_MsgQ, 0, &err);
		cmd = (InputCmd *) GetNextSlot(TI_To_GM_CmdBuf);

		switch (event->msgType) {
		case IT_BUTTON:
//			clearDisplay();
//			printString("TI");
//			printDecimal(event->bEvent);

			switch (event->bEvent) {
			case BUT0:
				cmd->cmdID = CMD0;
				break;
			case BUT1:
				cmd->cmdID = CMD1;
				break;
			case BUT2:
				cmd->cmdID = CMD2;
				break;
			case BUT3:
				cmd->cmdID = CMD3;
				break;
			default:
				// error
				break;
			}
			break;
		case IT_TLC:
			break;
		default:
			break;
		}

		err = OSQPost(TI_To_GM_MsgQ, (void *) cmd);
	}
}
