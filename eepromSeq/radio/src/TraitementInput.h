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

enum TI_MsgType {
	IT_BUTTON, IT_TLC
};

typedef struct InputEvent {
    enum TI_MsgType msgType;

    union {
        INT16U bEvent;
        INT8U tcEvent;
    };
} InputEvent;

void TraitementInput(void *parg);

#endif /* TRAITEMENTINPUT_H_ */
