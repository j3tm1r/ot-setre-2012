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

// Helpers
static ServiceMsg screenService;

char stringBuffer[N_CHAR_PER_LINE * N_LINE + 2];

// Declarations
void ModeRadioStep(INT16U event);
void ModeStatStep(INT16U event);
void sendToScreen(const char *str);

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

void GestionRadio(INT16U event);
void GestionStat(INT16U event);

void ModeStep(INT16U event) {

	switch (mode) {
	case VEILLE:
		if (event == CMD0) {
			mode = MR_INIT;
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
		}
		break;
	case MR_FIN:
		if (event == MR_FIN_ACK) {
			mode = VEILLE;
		}
		break;
	case MS:
		if (event == CMD1) {
			mode = VEILLE;
		}
		break;
	default:
		break;
	}
}

void ModeRadioStep(INT16U event) {

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

void ModeStatStep(INT16U event) {
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

void GestionStat(INT16U event) {

	char strNbUtil[] = "MS_NB_UTIL";
	char strVolume[] = "MS_VOLUME";
	char strStation[] = "MS_STATION";

	ModeStatStep(event);

	switch (modeStat) {
	case MS_NB_UTIL:

		//memcpy(stringBuffer, strNbUtil, strlen(strNbUtil));
		sendToScreen(strNbUtil);

		break;
	case MS_VOLUME:
		volStateCounter = (MS_VOL_SCREEN_NUM + volStateCounter - 1)
				% MS_VOL_SCREEN_NUM;
		//TODO read EEPROM
		//TODO Display data

		//memcpy(stringBuffer, strVolume, strlen(strVolume));
		sendToScreen(strVolume);

		break;
	case MS_STATION:
		freqStateCounter = (MS_FREQ_SCREEN_NUM + freqStateCounter - 1)
				% MS_FREQ_SCREEN_NUM;
		//TODO read EEPROM
		//TODO Display data

		//memcpy(stringBuffer, strStation, strlen(strStation));
		sendToScreen(strStation);

		break;
	default:
		break;
	}

}

void GestionRadio(INT16U event) {

	char strDefault[] = "MR_DEFAULT";
	char strFreq[] = "MR_SET_FREQ";
	char strVolume[] = "MR_SET_VOL";
	ServiceMsg serviceData;
	StatMsg statData;

	ModeRadioStep(event);

	switch (modeRadio) {
	case MR_DEFAULT:
		//memcpy(stringBuffer, strDefault, strlen(strDefault));
		// TODO print radio and volume information
		//stringBuffer
		sendToScreen(strDefault);
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
		serviceData.serviceType = SERV_FREQ;
		serviceData.val = currentFreqId;
		OSQPost(GM_To_SO_MsgQ, (void *) &serviceData);
		// Send current freq id to logger
		statData.msgType = STAT_LOG;
		statData.freq = currentFreqId;
		OSQPost(GM_To_SL_MsgQ, (void *) &statData);

		//memcpy(stringBuffer, strFreq, strlen(strFreq));
		// TODO print frequency information
		//stringBuffer
		sendToScreen(strFreq);
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
		serviceData.serviceType = SERV_VOLUME;
		serviceData.val = currentVolLvl;
		OSQPost(GM_To_SO_MsgQ, (void *) &serviceData);
		// Send current volume level to logger
		statData.msgType = STAT_LOG;
		statData.volumeLvl = currentVolLvl;
		OSQPost(GM_To_SL_MsgQ, (void *) &statData);

		//memcpy(stringBuffer, strVolume, strlen(strVolume));
		// TODO print volume information and set bargraph
		sendToScreen(strVolume);
		break;
	default:
		break;
	}
}

void GestionMode(void *parg) {

	task_GM_Param *param = (task_GM_Param*) parg;
	TI_To_GM_MsgQ = param->TI_To_GM_MsgQ;
	GM_To_SO_MsgQ = param->GM_To_SO_MsgQ;
	GM_To_SL_MsgQ = param->GM_To_SL_MsgQ;

	INT8U err;
	InputCmd *recvData;
	ServiceMsg serviceData;
	StatMsg statData;


	char strDefault[] = "VEILLE";
	char strMRInit[] = "MR_INIT";
	char strMR[] = "MR";
	char strMRFIN[] = "MR_FIN";
	char strMS[] = "MS";



	for (;;) {

		recvData = (InputCmd*) OSQPend(TI_To_GM_MsgQ, 0, &err);

		ModeStep(recvData->cmdID);

		// ATTENTION : il faudrait s'assurer que l'on ne rentre pas
		// deux fois successivement dans les cas VEILLE, MR_INIT, MR_FIN
		switch (mode) {
		case VEILLE:
			// LPM
			sendToScreen(strDefault);

			break;
		case MR_INIT:
			// Notify stat logger that we enter radio mode
			statData.msgType = STAT_INIT;
			OSQPost(GM_To_SL_MsgQ, (void *) &statData);

			// Start radio
			// Set frequency
			serviceData.serviceType = SERV_FREQ;
			serviceData.val = currentFreqId;
			OSQPost(GM_To_SO_MsgQ, (void *) &serviceData);

			// Set volume
			serviceData.serviceType = SERV_VOLUME;
			serviceData.val = currentVolLvl;
			OSQPost(GM_To_SO_MsgQ, (void *) &serviceData);
/*
			// Send current freq id to logger
			statData.msgType = STAT_LOG;
			statData.freq = currentFreqId;
			OSQPost(GM_To_SL_MsgQ, (void *) &statData);

			// Send current volume level to logger
			statData.msgType = STAT_LOG;
			statData.volumeLvl = currentVolLvl;
			OSQPost(GM_To_SL_MsgQ, (void *) &statData);
*/
			sendToScreen(strMRInit);


			break;
		case MR:
			// Transfer event to GestionRadio
			GestionRadio(recvData->cmdID);
			sendToScreen(strMR);

			break;
		case MR_FIN:
			// Notify stat logger that we leave radio mode
			statData.msgType = STAT_END;
			OSQPost(GM_To_SL_MsgQ, (void *) &statData);

			// Stop radio
			// Set volume to 0
			serviceData.serviceType = SERV_VOLUME;
			serviceData.val = 0;
			OSQPost(GM_To_SO_MsgQ, (void *) &serviceData);

			sendToScreen(strMRFIN);

			break;
		case MS:
			// Transfer event to GestionStat
			GestionStat(recvData->cmdID);
			sendToScreen(strMS);

			break;
		default:
			break;
		}

		/*gotoSecondLine();
		 printString("GM");
		 printDecimal(recvData->cmdID);

		 serviceData.serviceType = SERV_FREQ;
		 serviceData.val			= 828;

		 statData.msgType = STAT_LOG;
		 statData.volumeLvl = 241;

		 err = OSQPost(GM_To_SO_MsgQ, (void *) &serviceData);
		 err = OSQPost(GM_To_SL_MsgQ, (void *) &statData);*/

		//OSTimeDly(2*OS_TICKS_PER_SEC);
	}

}

void sendToScreen(const char *str) {
	screenService.serviceType = SERV_LCD;
	screenService.msg.pBuffer = (void *) str;
	screenService.msg.size = strlen(str);
	OSQPost(GM_To_SO_MsgQ, (void *) &screenService);
}

