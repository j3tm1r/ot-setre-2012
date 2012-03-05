/*************************************************************************
 ModeStateMachine  -  description
 -------------------
 début                : 15 févr. 2012
 copyright            : (C) 2012 par ubunt11
 *************************************************************************/

//---------- Réalisation du module <GestionMode> (fichier GestionMode.c) -----
/////////////////////////////////////////////////////////////////  INCLUDE
//-------------------------------------------------------- Include système
//------------------------------------------------------ Include personnel
#include "GestionMode.h"
#include "ServiceOutput.h"
#include "os_cfg.h"
#include "includes.h"
#include "Display.h"

///////////////////////////////////////////////////////////////////  PRIVE
//------------------------------------------------------------- Constantes

//------------------------------------------------------------------ Types

//---------------------------------------------------- Variables statiques
INT16U mode = VEILLE;

//------------------------------------------------------ Fonctions privées
//static type nom ( liste de paramètres )
// Mode d'emploi :
//
// Contrat :
//
// Algorithme :
//
//{
//} //----- fin de nom

//////////////////////////////////////////////////////////////////  PUBLIC
//---------------------------------------------------- Fonctions publiques

void ModeStep(INT16U event) {
	switch (mode) {
	case VEILLE:
		if(event == CMD0) {
			mode = MR_INIT;
		} else if (event == CMD1) {
			mode = MS;
		}
		break;
	case MR_INIT:
		if(event == MR_INIT_ACK) {
			mode = MR;
		}
		break;
	case MR:
		if(event == CMD0) {
			mode = MR_FIN;
		}
		break;
	case MR_FIN:
		if(event == MR_FIN_ACK) {
			mode = VEILLE;
		}
		break;
	case MS:
		if(event == CMD1) {
			mode = VEILLE;
		}
		break;
	default:
		break;
	}
}


void GestionMode(void *parg) {

		task_GM_Param *param = (task_GM_Param*) parg;
		OS_EVENT *TI_To_GM_MsgQ = param->TI_To_GM_MsgQ;
		OS_EVENT *GM_To_SO_MsgQ = param->GM_To_SO_MsgQ;
		OS_EVENT *GM_To_SL_MsgQ = param->GM_To_SL_MsgQ;

		INT8U err;
		InputCmd *recData;
		ServiceMsg sendData;
		//data.val = 0;

		for(;;) {

			recData = (InputCmd*) OSQPend (TI_To_GM_MsgQ, 0, &err);

			gotoSecondLine();
			printString("GM");
			printDecimal(recData->cmdID);

			/*data.val++;
			data.serviceType = SERV_LCD;

			err = OSQPost (msgQServiceOutput, (void *)&data);*/
			STATUS_LED_ON;

			OSTimeDly(2*OS_TICKS_PER_SEC);

			STATUS_LED_OFF;

		}

}
