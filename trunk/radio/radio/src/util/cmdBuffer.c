/*
 * cmdBuffer.c
 *
 *  Created on: 7 mars 2012
 *      Author: lthouvenin1
 */

#include "cmdBuffer.h"

#include <os_cpu.h>
#include <stdlib.h>
#include <string.h>

#ifdef FIFO
typedef struct FifoNode
{
  void 		*dataPtr;
} FifoNode;
#endif

typedef struct CmdBuffer {
	void 	*data;
	INT8U	chunkSize;
	INT8U	nChunks;
	INT8S	curChunk;
#ifdef FIFO
	FifoNode *fifoNode;
	INT8U	fifoCounter;
#endif
} CmdBuffer;

static INT8U 		requestedSlots [CMD_BUFFER_NMAX];		// Slots are initialized to 0
static CmdBuffer 	cmdBuffer [CMD_BUFFER_NMAX];
static INT8U		nCmdBuffer = 0;
static OS_CPU_SR  cpu_sr;


void *GetNextSlot(INT16S cmdBufHandle);


INT16S InitCmdBuffer(INT8U nSlot, INT8U slotSize) {
	OS_ENTER_CRITICAL();
	if(nCmdBuffer == CMD_BUFFER_NMAX) {
		return -1;
	}
	// get first available slot in cmdBuffer table
	INT16S i;
	for (i=0; i < CMD_BUFFER_NMAX; ++i) {
		if(requestedSlots[i] == 0) {
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

#ifdef FIFO
	cmdBuffer[i].fifoNode = (FifoNode *) malloc(nSlot * sizeof(FifoNode));
	cmdBuffer[i].fifoCounter = 0;
#endif

	// reserve current slot
	requestedSlots[i] = 1;
	// increment CmdBuffer use counter
	++nCmdBuffer;

	OS_EXIT_CRITICAL();

	return i;
}

void *GetNextSlot(INT16S cmdBufHandle) {
#ifndef FIFO
	OS_ENTER_CRITICAL();

	if(cmdBufHandle < 0 || cmdBufHandle >= CMD_BUFFER_NMAX
			|| requestedSlots [cmdBufHandle] == 0) {
		return 0;
	}
#endif
	void *nextSlot;
	CmdBuffer *curBuf = &cmdBuffer[cmdBufHandle];

	// get a pointer to next chunk of data in the buffer
	curBuf->curChunk = (curBuf->curChunk + 1) % curBuf->nChunks;
	nextSlot = curBuf->data + curBuf->curChunk * curBuf->chunkSize;

#ifndef FIFO
	OS_EXIT_CRITICAL();
#endif

	return nextSlot;
}

#ifdef FIFO

INT8S Queue(INT16S hdl, void *val) {
	OS_ENTER_CRITICAL();
	if(hdl < 0 || hdl >= CMD_BUFFER_NMAX
			|| requestedSlots [hdl] == 0) {
		return -1;
	}
	INT8U fifoMaxSize = cmdBuffer[hdl].nChunks;
	INT8U fifoCurSize = cmdBuffer[hdl].fifoCounter;
	if(fifoCurSize >= fifoMaxSize) {
		return -2;
	}
	// compute first chunk address
//	INT8U first = (cmdBuffer[hdl].nChunks - cmdBuffer[hdl].fifoCounter) * sizeof(FifoNode);
//	INT8U last  = cmdBuffer[hdl].nChunks * sizeof(FifoNode);
	// slide fifo nodes to the left before adding a new one
	INT8U i = 0;
	FifoNode *tmpOldNode, *tmpNewNode;
	while (i < fifoCurSize) {
		tmpOldNode = &cmdBuffer[hdl].fifoNode[fifoMaxSize-fifoCurSize+i];
		tmpNewNode = &cmdBuffer[hdl].fifoNode[fifoMaxSize-fifoCurSize-1+i];
		tmpNewNode->dataPtr = tmpOldNode->dataPtr;
		++i;
	}
	// Insert new value in last slot
	void *dataPtr = GetNextSlot(hdl);
	if(dataPtr == 0) {
		return -3;
	}
	memcpy(dataPtr, val, cmdBuffer[hdl].chunkSize);
	tmpOldNode = &cmdBuffer[hdl].fifoNode[fifoMaxSize-1];
	tmpOldNode->dataPtr = dataPtr;	// last slot

	// Update fifo counter
	cmdBuffer[hdl].fifoCounter++;
	OS_EXIT_CRITICAL();
	return 0;
}

void * DeQueue(INT16S hdl) {
	OS_ENTER_CRITICAL();
	if(hdl < 0 || hdl >= CMD_BUFFER_NMAX ) {
		return 0;
	}
	INT8U fifoMaxSize = cmdBuffer[hdl].nChunks;
	INT8U fifoCurSize = cmdBuffer[hdl].fifoCounter;
	if(fifoCurSize <= 0) {
		return 0;
	}

	// Update fifo counter
	cmdBuffer[hdl].fifoCounter--;

	FifoNode *first = &cmdBuffer[hdl].fifoNode[fifoMaxSize-fifoCurSize];
	OS_EXIT_CRITICAL();
	return first->dataPtr;
}

#endif

INT8S DestroyCmdBuffer(INT16S cmdBufHandle) {
	OS_ENTER_CRITICAL();
	if(cmdBufHandle < 0 || cmdBufHandle >= CMD_BUFFER_NMAX ) {
		return -1;
	}

	free(cmdBuffer[cmdBufHandle].data);
#ifdef FIFO
	free(cmdBuffer[cmdBufHandle].fifoNode);
#endif

	requestedSlots [cmdBufHandle] = 0;
	--nCmdBuffer;

	OS_EXIT_CRITICAL();
	return 0;
}




