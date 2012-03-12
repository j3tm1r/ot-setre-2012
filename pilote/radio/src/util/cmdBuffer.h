/*
 * cmdBuffer.h
 *
 *  Created on: 7 mars 2012
 *      Author: lfarina
 */

#ifndef CMDBUFFER_H_
#define CMDBUFFER_H_

#include    <os_cpu.h>

#define CMD_BUFFER_NMAX 	5

INT8S InitCmdBuffer(INT8U nSlot, INT8U slotSize);
// returns a handle on a CmdBuffer or -1 in case of error

void *GetNextSlot(INT8S cmdBufHandle);
// returns a pointer on an available slot

INT8S DestroyCmdBuffer(INT8S cmdBufHandle);
// Destroys the given command buffer


#endif /* CMDBUFFER_H_ */
