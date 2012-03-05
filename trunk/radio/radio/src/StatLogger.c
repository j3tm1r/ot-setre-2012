/*
 * StatLogger.c
 *
 *  Created on: 28 févr. 2012
 *      Author: mbbadau
 */


#include "StatLogger.h"
#include "os_cfg.h"

void StatLogger(void *parg) {
	for(;;) {
		OSTimeDly(OS_TICKS_PER_SEC);
	}
}
