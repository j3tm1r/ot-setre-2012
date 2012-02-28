/*
 * ServiceES.c
 *
 *  Created on: 28 févr. 2012
 *      Author: mbbadau
 */

#include "ServiceOutput.h"
#include "Display.h"
#include "includes.h"

void ServiceOutput(void *parg) {

		OS_EVENT *msgQServiceOutput = (OS_EVENT*) parg;
		INT8U err;
		ServiceMsg* data;
		//printDecimal(50);
		while(1) {

			//printDecimal(50);
			data = (ServiceMsg*) OSQPend (msgQServiceOutput, 0, &err);
			if(data->serviceType == SERV_LCD) {
				printDecimal(data->val);
			}
		}

}
