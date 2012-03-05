/*
 * TraitementInput.c
 *
 *  Created on: 28 févr. 2012
 *      Author: mbbadau
 */

#include "TraitementInput.h"
#include "GestionMode.h"
#include "ServicesES/Display.h"

extern OS_EVENT *ISR_To_TI_MsgQ;
extern OS_EVENT *TI_To_GM_MsgQ;

void TraitementInput(void *parg) {
	INT8U err;
	InputCmd cmd;

	//TACTL |= MC1;           /* Start the Timer in Continuous mode. */

	for (;;) {

		InputEvent *event = (InputEvent*) OSQPend(ISR_To_TI_MsgQ, 0, &err);
		switch (event->msgType) {
		case IT_BUTTON:
			clearDisplay();
			printString("TI");
			printDecimal(event->bEvent);

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

			err = OSQPost(TI_To_GM_MsgQ, (void *) &cmd);
			break;
		case IT_TLC:
			// TODO
			break;
		default:
			//error
			break;
		}

	}
}
