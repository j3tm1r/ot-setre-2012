/*************************************************************************
                           GestionMode  -  description
                             -------------------
    début                : 15 févr. 2012
    copyright            : (C) 2012 par ubunt11
*************************************************************************/

//---------- Interface du module <ModeStateMachine> (fichier ModeStateMachine.h) ---------
#if ! defined ( GESTION_MODE_H )
#define GESTION_MODE_H

//------------------------------------------------------------------------ 
// Rôle du module <ModeStateMachine>
//
//
//------------------------------------------------------------------------ 

/////////////////////////////////////////////////////////////////  INCLUDE
//--------------------------------------------------- Interfaces utilisées
#include <os_cpu.h>
#include <ucos_ii.h>

//------------------------------------------------------------- Constantes

// cmd
#define CMD0		0
#define CMD1		1
#define CMD2		2
#define CMD3		3
#define MR_INIT_ACK	4
#define MR_FIN_ACK	5

//------------------------------------------------------------------ Types 

typedef struct InputCmd {
	INT16U cmdID;
} InputCmd;

//////////////////////////////////////////////////////////////////  PUBLIC
//---------------------------------------------------- Fonctions publiques
void ModeStep(INT16U event);
void GestionMode(void *parg);

#endif // GESTION_MODE_H
