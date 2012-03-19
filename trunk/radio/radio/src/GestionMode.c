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
#include <ucos_ii.h>
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

//
static char strDefault[] = "B0 radio\nB1 statistics"; //"VEILLE";
//static char strMRInit[] 	= 	"MR_INIT";
//static char strMRFIN[] 		= 	"MR_FIN";

static char strMRDefault[] = "Radio OT SETRE";
static char strMRFreq[] = "Prev/Next Freq";
static char strMRVolume[] = "Current Volume";
static char strMSNbUtil[] = "Stats Usage";
static char strMSVolume[] = "Stats Volume";
static char strMSStation[] = "Stats Channel";
static char secondLine = '\n';

static char printBuffer[N_CHAR_PER_LINE * N_LINE + 2];

// Helpers
char stringBuffer[N_CHAR_PER_LINE * N_LINE + 2];

// Declarations
void GestionModeStep(INT16U event);
void GestionRadio(INT16U event);
void RadioModeStep(INT16U event);
void GestionStat(INT16U event);
void StatModeStep(INT16U event);
void ModeVeille();

extern INT16S GM_To_SL_CmdBuf;

extern INT8U statLoggerPrio;
extern OS_EVENT *TI_To_GM_MsgQ;
extern OS_EVENT *GM_To_SL_MsgQ;

extern INT16S TI_To_GM_CmdBuf;
extern INT16S GM_To_SL_CmdBuf;

extern Station stationMap[];

extern INT16S curSessionIdx;

void TickToTime(INT32U ticks, char* buffer);

static StatMsg statMsg;
static StorageIndex storIndex;
static InputCmd *recvData;
static INT8U err;
static Session curSession;
static INT16U curSessionAddr;

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
	ModeVeille();
	for (;;) {

		OSQPend(TI_To_GM_MsgQ, 0, &err);
		recvData = (InputCmd *) DeQueue(TI_To_GM_CmdBuf);
		if (recvData != 0) {
			GestionModeStep(recvData->cmdID);
		} else {
			// should not happen !
			PrintScreen("DeQueue error ! ");
		}
	}

}

void GestionModeStep(INT16U event) {

	INT8S i;

	switch (mode) {
	case VEILLE:
		if (event == CMD0) {
			mode = MR_INIT;

			// Wake-up stat logger
			OSTaskResume(statLoggerPrio);

			// Notify stat logger that we enter radio mode
			statMsg.msgType = STAT_INIT;
			if (Queue(GM_To_SL_CmdBuf, &statMsg) == 0) {
				OSQPost(GM_To_SL_MsgQ, (void *) GM_To_SL_CmdBuf);
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
		} else if (event == CMD1) {
			mode = MS;
			// Transfer event to GestionStat
			GestionStat(event);
		}
		break;
	case MR_INIT:
		if (event == MR_INIT_ACK) {
			//// Start radio
			// Bargraph animation
			for (i = 0; i < VOL_NUM; i++) {
				SetBargraph(i);
				OSTimeDly(10);
			}
			for (i = VOL_NUM - 1; i >= currentVolLvl; i--) {
				SetBargraph(i);
				OSTimeDly(10);
			}

			// Set frequency
			SetFreqById(currentFreqId);

			// Set volume
			SetVolumeByLvl(currentVolLvl);

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
			SetVolumeByLvl(0);

			//PrintScreen(strMRFIN);
		} else {
			// Transfer event to GestionRadio
			GestionRadio(event);
		}
		break;
	case MR_FIN:
		if (event == MR_FIN_ACK) {

			for (i = currentVolLvl; i < VOL_NUM; i++) {

				SetBargraph(i);
				OSTimeDly(10);
			}

			for (i = VOL_NUM; i > 0; i--) {

				SetBargraph(i - 1);
				OSTimeDly(10);
			}

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

	RadioModeStep(event);

	switch (modeRadio) {
	case MR_DEFAULT:

		// print radio and volume information
		memset(printBuffer, 0, sizeof(printBuffer));
		strncpy(printBuffer, strMRDefault, strlen(strMRDefault));
		strncpy(printBuffer + strlen(printBuffer), &secondLine, 1);
		strncpy(printBuffer + strlen(printBuffer),
				stationMap[currentFreqId].freqReel,
				strlen(stationMap[currentFreqId].freqReel));
		PrintScreen(printBuffer);

		//PrintScreen(strMRDefault);

		break;
	case MR_SET_FREQ:
		if (event == CMD2) {
			currentFreqId = (FREQ_NUM + currentFreqId - 1) % FREQ_NUM;
		} else if (event == CMD3) {
			currentFreqId = (currentFreqId + 1) % FREQ_NUM;
		}

		memset(printBuffer, 0, sizeof(printBuffer));
		strncpy(printBuffer, strMRFreq, strlen(strMRFreq));
		strncpy(printBuffer + strlen(printBuffer), &secondLine, 1);
		strncpy(printBuffer + strlen(printBuffer),
				stationMap[currentFreqId].freqReel,
				strlen(stationMap[currentFreqId].freqReel));
		PrintScreen(printBuffer);

		// Set frequency
		SetFreqById(currentFreqId);
		// Send current freq id to logger
		statMsg.msgType = STAT_LOG;
		statMsg.freq = currentFreqId;
		if (Queue(GM_To_SL_CmdBuf, &statMsg) == 0) {
			OSQPost(GM_To_SL_MsgQ, (void *) GM_To_SL_CmdBuf);
		}
		break;
	case MR_SET_VOL:

		if (event == CMD2) {
			if (currentVolLvl != 0) {
				currentVolLvl -= 1;
			}
		} else if (event == CMD3) {
			if (currentVolLvl != VOL_NUM - 1) {
				currentVolLvl += 1;
			}
		}

		// print volume information and set bargraph
		memset(printBuffer, 0, sizeof(printBuffer));
		strncpy(printBuffer, strMRVolume, strlen(strMRVolume));
		strncpy(printBuffer + strlen(printBuffer), &secondLine, 1);
		DecimalToString(currentVolLvl, printBuffer + strlen(printBuffer), 2); //size : N_CHAR_PER_LINE * N_LINE + 2
		strncpy(printBuffer + strlen(printBuffer), "/8", strlen("/8"));
		PrintScreen(printBuffer);

		// Set volume
		SetVolumeByLvl(currentVolLvl);

		// Set bargraph
		SetBargraph(currentVolLvl);
		// Send current volume level to logger
		statMsg.msgType = STAT_LOG;
		statMsg.volumeLvl = currentVolLvl;
		if (Queue(GM_To_SL_CmdBuf, &statMsg) == 0) {
			OSQPost(GM_To_SL_MsgQ, (void *) GM_To_SL_CmdBuf);
		}

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

		PrintScreen("Erasing data...");

		memset(&storIndex, 0, sizeof(storIndex));
		storIndex.dataOffset = sizeof(storIndex);
		storIndex.sessionNum = 0;
		WriteEEPROM(0, &storIndex, sizeof(storIndex));
		curSessionIdx = -1;

		OSTimeDly(OS_TICKS_PER_SEC / 4);
		PrintScreen("Data erased !");
		OSTimeDly(OS_TICKS_PER_SEC);
	}

	switch (modeStat) {
	case MS_NB_UTIL:

		ReadEEPROM(0, &storIndex, sizeof(storIndex));
		memset(printBuffer, 0, sizeof(printBuffer));
		strncpy(printBuffer, strMSNbUtil, strlen(strMSNbUtil));
		strncpy(printBuffer + strlen(printBuffer), &secondLine, 1);
		strncpy(printBuffer + strlen(printBuffer), "Num of uses: ",
				strlen("Num of uses: "));
		DecimalToString(storIndex.sessionNum, printBuffer + strlen(printBuffer),
				4); //size : N_CHAR_PER_LINE * N_LINE + 2
		PrintScreen(printBuffer);

		break;
	case MS_VOLUME:

		//TODO read EEPROM & display data

		curSessionAddr = sizeof(StorageIndex) + curSessionIdx * sizeof(Session);
		ReadEEPROM(curSessionAddr, &curSession, sizeof(curSession));

		memset(printBuffer, 0, sizeof(printBuffer));
		strncpy(printBuffer, strMSVolume, strlen(strMSVolume));
		strncpy(printBuffer + strlen(printBuffer), &secondLine, 1);
		if (curSessionIdx >= 0) {
			if (volStateCounter == 0) {
				strncpy(printBuffer + strlen(printBuffer), "VOL 1-6 ",
						strlen("VOL 1-6 "));
			} else if (volStateCounter == 1) {
				strncpy(printBuffer + strlen(printBuffer), "VOL 7-8 ",
						strlen("VOL 7-8 "));

			}
			TickToTime(curSession.timePerVolLvl[volStateCounter], printBuffer);

		} else {
			strncpy(printBuffer + strlen(printBuffer), "No data",
					strlen("No data"));
		}
		PrintScreen(printBuffer);

		break;
	case MS_STATION:

		//TODO read EEPROM & display data

		curSessionAddr = sizeof(StorageIndex) + curSessionIdx * sizeof(Session);
		ReadEEPROM(curSessionAddr, &curSession, sizeof(curSession));

		//memcpy(stringBuffer, strStation, strlen(strStation));
		memset(printBuffer, 0, sizeof(printBuffer));

		strncpy(printBuffer, strMSStation, strlen(strMSStation));
		strncpy(printBuffer + strlen(printBuffer), &secondLine, 1);
		if (curSessionIdx >= 0) {
			strncpy(printBuffer + strlen(printBuffer),
					stationMap[freqStateCounter].freqReel,
					strlen(stationMap[freqStateCounter].freqReel));
			strncpy(printBuffer + strlen(printBuffer), " ", 1);
			TickToTime(curSession.timePerFreq[freqStateCounter], printBuffer);
		} else {
			strncpy(printBuffer + strlen(printBuffer), "No data",
					strlen("No data"));
		}
		PrintScreen(printBuffer);

		break;
	default:
		break;
	}

}

void TickToTime(INT32U ticks, char* buffer) {

	INT16U hours = (INT16U) (ticks /(OS_TICKS_PER_SEC * 3600 ));
	INT16U mins  = ticks /(OS_TICKS_PER_SEC * 60 ) - hours * 60;
	INT16U sec   = ticks /(OS_TICKS_PER_SEC) - mins * 60;

	DecimalToString(hours, buffer + strlen(buffer), 3);
	strncpy(printBuffer + strlen(printBuffer), ":", 1);
	DecimalToString(mins, buffer + strlen(buffer), 3);
	strncpy(printBuffer + strlen(printBuffer), ":", 1);
	DecimalToString(sec, buffer + strlen(buffer), 3);
}

void ModeVeille() {
	// LPM
	PrintScreen(strDefault);
}

void StatModeStep(INT16U event) {
	switch (modeStat) {
	case MS_NB_UTIL:
		if (event == CMD3) {
			modeStat = MS_VOLUME;
			volStateCounter = 0;
		} else if (event == CMD2) {
			modeStat = MS_STATION;
			freqStateCounter = MS_FREQ_SCREEN_NUM - 1;
		}
		break;
	case MS_VOLUME:

		if (event == CMD3) {
			volStateCounter = (volStateCounter + 1) % MS_VOL_SCREEN_NUM;
			if (volStateCounter == 0) {
				modeStat = MS_STATION;
				freqStateCounter = 0;
			}
		} else if (event == CMD2) {
			volStateCounter = (MS_VOL_SCREEN_NUM + volStateCounter - 1)
					% MS_VOL_SCREEN_NUM;

			if (volStateCounter == MS_VOL_SCREEN_NUM - 1) {
				modeStat = MS_NB_UTIL;
			}
		}
		break;
	case MS_STATION:
		if (event == CMD3) {
			freqStateCounter = (freqStateCounter + 1) % MS_FREQ_SCREEN_NUM;
			if (freqStateCounter == 0) {
				modeStat = MS_NB_UTIL;
			}
		} else if (event == CMD2) {
			freqStateCounter = (MS_FREQ_SCREEN_NUM + freqStateCounter - 1)
					% MS_FREQ_SCREEN_NUM;
			if (freqStateCounter == MS_FREQ_SCREEN_NUM - 1) {

				modeStat = MS_VOLUME;
				volStateCounter = MS_VOL_SCREEN_NUM - 1;
			}
		}
		break;
	default:
		break;
	}
}

