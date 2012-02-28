/*
 * TraitementInput.h
 *
 *  Created on: 28 févr. 2012
 *      Author: mbbadau
 */

#ifndef TRAITEMENTINPUT_H_
#define TRAITEMENTINPUT_H_

#define BUT0	0
#define BUT1	1
#define BUT2	2
#define BUT3	3

typedef union InputEvent {
	INT16U bEvent;
	struct {
		void *pBuffer;
		INT16U size;
	} tcEvent;

} InputEvent;

#endif /* TRAITEMENTINPUT_H_ */
