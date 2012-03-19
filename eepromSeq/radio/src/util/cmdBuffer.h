/*
 * cmdBuffer.h
 *
 *  Created on: 7 mars 2012
 *      Author: lfarina
 */

#ifndef CMDBUFFER_H_
#define CMDBUFFER_H_

#include    <os_cpu.h>

#define CMD_BUFFER_NMAX 	4
#define FIFO					// to map OSQueues

INT16S InitCmdBuffer(INT8U nSlot, INT8U slotSize);
// returns a handle on a CmdBuffer or -1 in case of error

#ifdef FIFO

INT8S Queue(INT16S cmdBufHandle, void *val);

void *DeQueue(INT16S cmdBufHandle);

#else

void *GetNextSlot(INT16S cmdBufHandle);
// returns a pointer on an available slot

#endif

INT8S DestroyCmdBuffer(INT16S cmdBufHandle);
// Destroys the given command buffer


#endif /* CMDBUFFER_H_ */
