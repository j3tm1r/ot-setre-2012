/*
 * cmdBuffer.c
 *
 *  Created on: 7 mars 2012
 *      Author: lfarina
 */

#include "cmdBuffer.h"

#include <stdlib.h>

typedef struct CmdBuffer {
	void 	*data;
	INT8U	chunkSize;
	INT8U	nChunks;
	INT8S	curChunk;
} CmdBuffer;

static INT8U 		availableSlots [CMD_BUFFER_NMAX] = {1};		// Initialize all slots to 0
static CmdBuffer 	cmdBuffer [CMD_BUFFER_NMAX];
static INT8U		nCmdBuffer = 0;


INT8S InitCmdBuffer(INT8U nSlot, INT8U slotSize) {
	if(nCmdBuffer == CMD_BUFFER_NMAX) {
		return -1;
	}
	// get first available slot in cmdBuffer table
	int i;
	for (i=0; i < CMD_BUFFER_NMAX; ++i) {
		if(availableSlots[i] == 1) {
			break;
		}
	}

	cmdBuffer[i].chunkSize = slotSize;
	cmdBuffer[i].nChunks = nSlot;
	cmdBuffer[i].curChunk = 0;
	cmdBuffer[i].data = malloc(nSlot * slotSize);
	if (cmdBuffer[i].data == 0) {
		return -1;
	}

	// reserve current slot
	availableSlots[i] = 0;
	// increment CmdBuffer use counter
	++nCmdBuffer;

	return i;
}

void *GetNextSlot(INT8S cmdBufHandle) {
	if(cmdBufHandle < 0 || cmdBufHandle >= CMD_BUFFER_NMAX ) {
		return 0;
	}
	void *nextSlot;
	CmdBuffer *curBuf = &cmdBuffer[cmdBufHandle];

	// get a pointer to next chunk of data in the buffer
	curBuf->curChunk = (curBuf->curChunk + 1) % curBuf->nChunks;
	nextSlot = curBuf->data + curBuf->curChunk * curBuf->chunkSize;

	return nextSlot;
}

INT8S DestroyCmdBuffer(INT8S cmdBufHandle) {
	if(cmdBufHandle < 0 || cmdBufHandle >= CMD_BUFFER_NMAX ) {
		return -1;
	}

	free(cmdBuffer[cmdBufHandle].data);

	availableSlots [cmdBufHandle] = 1;
	--nCmdBuffer;

	return 0;
}




