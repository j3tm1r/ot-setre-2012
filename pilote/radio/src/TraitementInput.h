/*
 * TraitementInput.h
 *
 *  Created on: 28 févr. 2012
 *      Author: mbbadau
 */

#ifndef TRAITEMENTINPUT_H_
#define TRAITEMENTINPUT_H_

#include <os_cpu.h>
#include <ucos_ii.h>

#define BUT0	0
#define BUT1	1
#define BUT2	2
#define BUT3	3
#define BUTERR	4

typedef struct task_TI_Param {
	OS_EVENT *ISR_To_TI_MsgQ;
	OS_EVENT *TI_To_GM_MsgQ;
} task_TI_Param;

enum TI_MsgType {
	IT_BUTTON, IT_TLC
};

typedef struct InputEvent {
	enum TI_MsgType msgType;

	union {
		INT16U bEvent;
		struct {
			void *pBuffer;
			INT16U size;
		} tcEvent;
	};
} InputEvent;

void TraitementInput(void *parg);

#endif /* TRAITEMENTINPUT_H_ */
