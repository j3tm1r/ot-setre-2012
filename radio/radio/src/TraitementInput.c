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

extern INT16S TI_To_GM_CmdBuf;
extern OS_EVENT *ISR_To_TI_MsgQ;
extern OS_EVENT *TI_To_GM_MsgQ;

extern INT16S 	TI_To_GM_CmdBuf;
extern INT16S 	ISR_To_TI_CmdBuf;

static InputCmd 	cmd;
static INT8U 		err;
static InputEvent 	*event;

void TraitementInput(void *parg) {

	for (;;) {

		OSQPend(ISR_To_TI_MsgQ, 0, &err);
		event = (InputEvent *) DeQueue(ISR_To_TI_CmdBuf);
		if (event == 0) {
			printString("TI DeQueue error!");
			continue;	// should never happened
		}

		switch (event->msgType) {
		case IT_BUTTON:
//			clearDisplay();
//			printString("TI");
//			printDecimal(event->bEvent);

			switch (event->bEvent) {
			case BUT0:
				cmd.cmdID = CMD0;
				break;
			case BUT1:
				cmd.cmdID = CMD1;
				break;
			case BUT2:
				cmd.cmdID = CMD2;
				break;
			case BUT3:
				cmd.cmdID = CMD3;
				break;
			default:
				// error
				break;
			}
			break;
		case IT_TLC:
			switch (event->tcEvent) {
			case BUT0:
				cmd.cmdID = CMD0;
				break;
			case BUT1:
				cmd.cmdID = CMD1;
				break;
			case BUT2:
				cmd.cmdID = CMD2;
				break;
			case BUT3:
				cmd.cmdID = CMD3;
				break;
			default:
				// error
				break;
			}
			break;
		default:
			break;
		}

		if (Queue(TI_To_GM_CmdBuf, &cmd) == 0) {
			err = OSQPost(TI_To_GM_MsgQ, (void *) TI_To_GM_CmdBuf);
		}
	}
}
