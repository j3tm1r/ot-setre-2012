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
#include <string.h>

#include "GestionMode.h"
#include "ServiceOutput.h"
#include "StatLogger.h"
#include "os_cfg.h"
#include "includes.h"
#include "Display.h"
#include "util/cmdBuffer.h"

///////////////////////////////////////////////////////////////////  PRIVE
//------------------------------------------------------------- Constantes
// GestionMode states
#define VEILLE			0
#define MR_INIT			1
#define MR				2
#define MR_FIN			3
#define MS				4

// MR States
#define MR_DEFAULT		0
#define MR_SET_FREQ		1
#define MR_SET_VOL		2

// MS States
#define MS_NB_UTIL		0
#define MS_VOLUME		1
#define MS_STATION		2

#define MS_VOL_SCREEN_NUM		2	// [0-5] & [6-7]
#define MS_FREQ_SCREEN_NUM		FREQ_NUM

//------------------------------------------------------------------ Types

//---------------------------------------------------- Variables statiques
// GestionMode State Machine
static INT16U mode = VEILLE;

//// GESTION STATISTIQUES
// GestionStat State Machine
static INT16U modeStat = MS_NB_UTIL;

//// GESTION RADIO
// GestionRadio State Machine
static INT16U modeRadio = MR_DEFAULT;
static INT8S volStateCounter = 0;
static INT8S freqStateCounter = 0;

static INT8S currentFreqId = DEFAULT_FREQ_ID;
static INT8S currentVolLvl = DEFAULT_VOL_LVL;

static OS_EVENT *TI_To_GM_MsgQ;
static OS_EVENT *GM_To_SO_MsgQ;
static OS_EVENT *GM_To_SL_MsgQ;

//
static char strDefault[]= "VEILLE";
static char strMRInit[] = "MR_INIT";
static char strMRFIN[] 	= "MR_FIN";

static char strMRDefault[]= "MR_DEFAULT";
static char strMRFreq[]   = "MR_SET_FREQ";
static char strMRVolume[] = "MR_SET_VOL";
static char strMSNbUtil[] = "MS_NB_UTIL";
static char strMSVolume[] = "MS_VOLUME";
static char strMSStation[]= "MS_STATION";


// Helpers

char stringBuffer[N_CHAR_PER_LINE * N_LINE + 2];

// Declarations
void GestionModeStep(INT16U event);
void GestionRadio(INT16U event);
void RadioModeStep(INT16U event);
void GestionStat(INT16U event);
void StatModeStep(INT16U event);
void ModeVeille();

void sendToScreen(const char *str);

extern INT8S GM_To_SO_CmdBuf;
extern INT8S GM_To_SL_CmdBuf;

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

void GestionMode(void *parg) {

	task_GM_Param *param = (task_GM_Param*) parg;
	TI_To_GM_MsgQ = param->TI_To_GM_MsgQ;
	GM_To_SO_MsgQ = param->GM_To_SO_MsgQ;
	GM_To_SL_MsgQ = param->GM_To_SL_MsgQ;

	INT8U err;
	InputCmd *recvData;

	for (;;) {

		recvData = (InputCmd*) OSQPend(TI_To_GM_MsgQ, 0, &err);

		GestionModeStep(recvData->cmdID);

	}

}

void GestionModeStep(INT16U event) {

	ServiceMsg *servMsg;
	StatMsg *statMsg;

	switch (mode) {
	case VEILLE:
		if (event == CMD0) {
			mode = MR_INIT;

			// Notify stat logger that we enter radio mode
			statMsg = (StatMsg*) GetNextSlot(GM_To_SL_CmdBuf);
			statMsg->msgType = STAT_INIT;
			OSQPost(GM_To_SL_MsgQ, (void *) statMsg);

			// Start radio
			// Set frequency
			servMsg = (ServiceMsg*) GetNextSlot(GM_To_SO_CmdBuf);
			servMsg->serviceType = SERV_FREQ;
			servMsg->val = currentFreqId;
			OSQPost(GM_To_SO_MsgQ, (void *) servMsg);

			// Set volume
			servMsg = (ServiceMsg*) GetNextSlot(GM_To_SO_CmdBuf);
			servMsg->serviceType = SERV_VOLUME;
			servMsg->val = currentVolLvl;
			OSQPost(GM_To_SO_MsgQ, (void *) servMsg);

			// Send current freq id to logger
			statMsg = (StatMsg*) GetNextSlot(GM_To_SL_CmdBuf);
			statMsg->msgType = STAT_LOG;
			statMsg->freq = currentFreqId;
			OSQPost(GM_To_SL_MsgQ, (void *) statMsg);

			// Send current volume level to logger
			statMsg = (StatMsg*) GetNextSlot(GM_To_SL_CmdBuf);
			statMsg->msgType = STAT_LOG;
			statMsg->volumeLvl = currentVolLvl;
			OSQPost(GM_To_SL_MsgQ, (void *) statMsg);

			sendToScreen(strMRInit);

		} else if (event == CMD1) {
			mode = MS;
		}
		break;
	case MR_INIT:
		if (event == MR_INIT_ACK) {
			mode = MR;
		}
		break;
	case MR:
		if (event == CMD0) {
			mode = MR_FIN;

			// Notify stat logger that we leave radio mode
			statMsg = (StatMsg*) GetNextSlot(GM_To_SL_CmdBuf);
			statMsg->msgType = STAT_END;
			OSQPost(GM_To_SL_MsgQ, (void *) statMsg);

			// Stop radio
			// Set volume to 0
			servMsg = (ServiceMsg*) GetNextSlot(GM_To_SO_CmdBuf);
			servMsg->serviceType = SERV_VOLUME;
			servMsg->val = 0;
			OSQPost(GM_To_SO_MsgQ, (void *) servMsg);

			sendToScreen(strMRFIN);
		}
		// Transfer event to GestionStat
		GestionStat(event);
		break;
	case MR_FIN:
		if (event == MR_FIN_ACK) {
			mode = VEILLE;
			ModeVeille();
		}
		break;
	case MS:
		if (event == CMD1) {
			mode = VEILLE;
			ModeVeille();
		}
		// Transfer event to GestionStat
		GestionStat(event);
		break;
	default:
		break;
	}
}

void GestionRadio(INT16U event) {

	ServiceMsg *servMsg;
	StatMsg *statMsg;

	RadioModeStep(event);

	switch (modeRadio) {
	case MR_DEFAULT:
		//memcpy(stringBuffer, strDefault, strlen(strDefault));
		// TODO print radio and volume information
		//stringBuffer
		sendToScreen(strMRDefault);
		break;
	case MR_SET_FREQ:
		if (event == CMD2) {
			currentFreqId = (FREQ_NUM + currentFreqId - 1) % FREQ_NUM;
		} else if (event == CMD3) {
			currentFreqId = (currentFreqId + 1) % FREQ_NUM;
		} else {
			// error: should never happen
		}
		// Set frequency
		servMsg = (ServiceMsg*) GetNextSlot(GM_To_SO_CmdBuf);
		servMsg->serviceType = SERV_FREQ;
		servMsg->val = currentFreqId;
		OSQPost(GM_To_SO_MsgQ, (void *) servMsg);
		// Send current freq id to logger
		statMsg = (StatMsg*) GetNextSlot(GM_To_SL_CmdBuf);
		statMsg->msgType = STAT_LOG;
		statMsg->freq = currentFreqId;
		OSQPost(GM_To_SL_MsgQ, (void *) statMsg);

		//memcpy(stringBuffer, strFreq, strlen(strFreq));
		// TODO print frequency information
		//stringBuffer
		sendToScreen(strMRFreq);
		break;
	case MR_SET_VOL:
		if (event == CMD2) {
			currentVolLvl = (VOL_NUM + currentVolLvl - 1) % VOL_NUM;
		} else if (event == CMD3) {
			currentVolLvl = (currentVolLvl + 1) % VOL_NUM;
		} else {
			// error: should never happen
		}
		// Set volume
		servMsg = (ServiceMsg*) GetNextSlot(GM_To_SO_CmdBuf);
		servMsg->serviceType = SERV_VOLUME;
		servMsg->val = currentVolLvl;
		OSQPost(GM_To_SO_MsgQ, (void *) servMsg);
		// Send current volume level to logger
		statMsg = (StatMsg*) GetNextSlot(GM_To_SL_CmdBuf);
		statMsg->msgType = STAT_LOG;
		statMsg->volumeLvl = currentVolLvl;
		OSQPost(GM_To_SL_MsgQ, (void *) statMsg);

		//memcpy(stringBuffer, strVolume, strlen(strVolume));
		// TODO print volume information and set bargraph
		sendToScreen(strMRVolume);
		break;
	default:
		break;
	}
}

void RadioModeStep(INT16U event) {

	switch (modeRadio) {
	case MR_DEFAULT:
		if (event == CMD1) {
			modeRadio = MR_SET_FREQ;
		}
		break;
	case MR_SET_FREQ:
		if (event == CMD1) {
			modeRadio = MR_SET_VOL;
		}
		break;
	case MR_SET_VOL:
		if (event == CMD1) {
			modeRadio = MR_DEFAULT;
		}
		break;
	default:
		break;
	}
}

void GestionStat(INT16U event) {

	StatModeStep(event);

	switch (modeStat) {
	case MS_NB_UTIL:

		//memcpy(stringBuffer, strNbUtil, strlen(strNbUtil));
		sendToScreen(strMSNbUtil);

		break;
	case MS_VOLUME:
		volStateCounter = (MS_VOL_SCREEN_NUM + volStateCounter - 1)
				% MS_VOL_SCREEN_NUM;
		//TODO read EEPROM
		//TODO Display data

		//memcpy(stringBuffer, strVolume, strlen(strVolume));
		sendToScreen(strMSVolume);

		break;
	case MS_STATION:
		freqStateCounter = (MS_FREQ_SCREEN_NUM + freqStateCounter - 1)
				% MS_FREQ_SCREEN_NUM;
		//TODO read EEPROM
		//TODO Display data

		//memcpy(stringBuffer, strStation, strlen(strStation));
		sendToScreen(strMSStation);

		break;
	default:
		break;
	}

}

void ModeVeille() {
	// LPM
	sendToScreen(strDefault);
}

void StatModeStep(INT16U event) {
	switch (modeStat) {
	case MS_NB_UTIL:
		if (event == CMD2) {
			modeStat = MS_VOLUME;
		} else if (event == CMD3) {
			modeStat = MS_STATION;
		}
		break;
	case MS_VOLUME:
		if (event == CMD2) {
			modeStat = MS_STATION;
		} else if (event == CMD3) {
			modeStat = MS_NB_UTIL;
		}
		break;
	case MS_STATION:
		if (event == CMD2) {
			modeStat = MS_NB_UTIL;
		} else if (event == CMD3) {
			modeStat = MS_VOLUME;
		}
		break;
	default:
		break;
	}
}

void sendToScreen(const char *str) {
	ServiceMsg *servMsg;
	servMsg = (ServiceMsg*) GetNextSlot(GM_To_SO_CmdBuf);
	servMsg->serviceType = SERV_LCD;
	servMsg->msg.pBuffer = (void *) str;
	servMsg->msg.size = strlen(str);
	OSQPost(GM_To_SO_MsgQ, (void *) servMsg);
}

