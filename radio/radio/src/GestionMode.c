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
