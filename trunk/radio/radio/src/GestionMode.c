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
#include "ServicesES/drv_eeprom.h"

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
static char strDefault[] = "VEILLE";
static char strMRInit[] = "MR_INIT";
static char strMRFIN[] = "MR_FIN";

static char strMRDefault[] = "MR_DEFAULT";
static char strMRFreq[] = "MR_SET_FREQ";
static char strMRVolume[] = "MR_SET_VOL";
static char strMSNbUtil[] = "MS_NB_UTIL";
static char strMSVolume[] = "MS_VOLUME";
static char strMSStation[] = "MS_STATION";

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

extern INT16S GM_To_SO_CmdBuf;
extern INT16S GM_To_SL_CmdBuf;

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
	INT16S bufHandle;
	InputCmd *recvData;

	for (;;) {

		bufHandle = (INT16S) OSQPend(TI_To_GM_MsgQ, 0, &err);
		recvData = (InputCmd *) DeQueue(bufHandle);
		if (recvData != 0) {
			GestionModeStep(recvData->cmdID);
		} else {
			// should not happen !
			sendToScreen("DeQueue error ! ");
		}
	}

}

void GestionModeStep(INT16U event) {

	ServiceMsg servMsg;
	StatMsg statMsg;

	switch (mode) {
	case VEILLE:
		if (event == CMD0) {
			mode = MR_INIT;

			// Notify stat logger that we enter radio mode
			statMsg.msgType = STAT_INIT;
			if (Queue(GM_To_SL_CmdBuf, &statMsg) == 0) {
				OSQPost(GM_To_SL_MsgQ, (void *) GM_To_SL_CmdBuf);
			}

			// Start radio
			// Set frequency
			servMsg.serviceType = SERV_FREQ;
			servMsg.val = currentFreqId;
			if (Queue(GM_To_SO_CmdBuf, &servMsg) == 0) {
				OSQPost(GM_To_SO_MsgQ, (void *) GM_To_SO_CmdBuf);
			}

			// Set volume
			servMsg.serviceType = SERV_VOLUME;
			servMsg.val = currentVolLvl;
			if (Queue(GM_To_SO_CmdBuf, &servMsg) == 0) {
				OSQPost(GM_To_SO_MsgQ, (void *) GM_To_SO_CmdBuf);
			}

			// Send current freq id to logger
			statMsg.msgType = STAT_LOG;
			statMsg.freq = currentFreqId;
			if (Queue(GM_To_SL_CmdBuf, &statMsg) == 0) {
				OSQPost(GM_To_SL_MsgQ, (void *) GM_To_SL_CmdBuf);
			}

			// Send current volume level to logger
			statMsg.msgType = STAT_LOG;
			statMsg.volumeLvl = currentVolLvl;
			if (Queue(GM_To_SL_CmdBuf, &statMsg) == 0) {
				OSQPost(GM_To_SL_MsgQ, (void *) GM_To_SL_CmdBuf);
			}

			sendToScreen(strMRInit);

		} else if (event == CMD1) {
			mode = MS;
			// Transfer event to GestionStat
			GestionStat(event);
		}
		break;
	case MR_INIT:
		if (event == MR_INIT_ACK) {
			mode = MR;
			// Transfer event to GestionRadio
			GestionRadio(event);
		}
		break;
	case MR:
		if (event == CMD0) {
			mode = MR_FIN;

			// Notify stat logger that we leave radio mode
			statMsg.msgType = STAT_END;
			if (Queue(GM_To_SL_CmdBuf, &statMsg) == 0) {
				OSQPost(GM_To_SL_MsgQ, (void *) GM_To_SL_CmdBuf);
			}

			// Stop radio
			// Set volume to 0
			servMsg.serviceType = SERV_VOLUME;
			servMsg.val = 0;
			if (Queue(GM_To_SO_CmdBuf, &servMsg) == 0) {
				OSQPost(GM_To_SO_MsgQ, (void *) GM_To_SO_CmdBuf);
			}

			sendToScreen(strMRFIN);
		} else {
			// Transfer event to GestionRadio
			GestionRadio(event);
		}
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
		} else {
			// Transfer event to GestionStat
			GestionStat(event);
		}
		break;
	default:
		break;
	}
}

void GestionRadio(INT16U event) {

	ServiceMsg 	servMsg;
	StatMsg 	statMsg;

	RadioModeStep(event);

	switch (modeRadio) {
	case MR_DEFAULT:
		//memcpy(stringBuffer, strDefault, strlen(strDefault));
		// TODO print radio and volume information
		//stringBuffer
		sendToScreen(strMRDefault);
		break;
	case MR_SET_FREQ:
		if (event == CMD1) {
			sendToScreen(strMRFreq);
		}
		if (event == CMD2) {
			currentFreqId = (FREQ_NUM + currentFreqId - 1) % FREQ_NUM;
		} else if (event == CMD3) {
			currentFreqId = (currentFreqId + 1) % FREQ_NUM;
		} else {
			// CMD1
			break;
		}
		// Set frequency
		servMsg.serviceType = SERV_FREQ;
		servMsg.val = currentFreqId;
		if (Queue(GM_To_SO_CmdBuf, &servMsg) == 0) {
			OSQPost(GM_To_SO_MsgQ, (void *) GM_To_SO_CmdBuf);
		}
		// Send current freq id to logger
		statMsg.msgType = STAT_LOG;
		statMsg.freq = currentFreqId;
		if (Queue(GM_To_SL_CmdBuf, &statMsg) == 0) {
			OSQPost(GM_To_SL_MsgQ, (void *) GM_To_SL_CmdBuf);
		}

		// TODO print frequency information
		//sendToScreen(strMRFreq);
		break;
	case MR_SET_VOL:
		if (event == CMD1) {
			sendToScreen(strMRVolume);
		}

		if (event == CMD2) {
			if (currentVolLvl != 0) {
				currentVolLvl -= 1;
			}
		} else if (event == CMD3) {
			if (currentVolLvl != VOL_NUM - 1) {
				currentVolLvl += 1;
			}
		} else {
			// CMD1
			break;
		}
		// Set volume
		servMsg.serviceType = SERV_VOLUME;
		servMsg.val = currentVolLvl;
		if (Queue(GM_To_SO_CmdBuf, &servMsg) == 0) {
			OSQPost(GM_To_SO_MsgQ, (void *) GM_To_SO_CmdBuf);
		}
		// Set bargraph
		servMsg.serviceType = SERV_BARGRAPH;
		servMsg.val = currentVolLvl;
		if (Queue(GM_To_SO_CmdBuf, &servMsg) == 0) {
			OSQPost(GM_To_SO_MsgQ, (void *) GM_To_SO_CmdBuf);
		}
		// Send current volume level to logger
		statMsg.msgType = STAT_LOG;
		statMsg.volumeLvl = currentVolLvl;
		if (Queue(GM_To_SL_CmdBuf, &statMsg) == 0) {
			OSQPost(GM_To_SL_MsgQ, (void *) GM_To_SL_CmdBuf);
		}

		//memcpy(stringBuffer, strVolume, strlen(strVolume));
		// TODO print volume information and set bargraph
		//sendToScreen(strMRVolume);
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

	if (event == CMD0) {
		// TODO erase data
		// send data to StatLogger
		sendToScreen("Erasing data...");
		// TMP
		OSTimeDly(OS_TICKS_PER_SEC);
		sendToScreen("Data erased !");
		OSTimeDly(OS_TICKS_PER_SEC);
		// TMP END
	}

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
	ServiceMsg servMsg;
	servMsg.serviceType = SERV_LCD;
	servMsg.msg.pBuffer = (void *) str;
	servMsg.msg.size = strlen(str);
	if (Queue(GM_To_SO_CmdBuf, &servMsg) == 0) {
		OSQPost(GM_To_SO_MsgQ, (void *) GM_To_SO_CmdBuf);
	}
}

