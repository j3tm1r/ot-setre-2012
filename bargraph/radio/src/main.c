/*
 *********************************************************************************************************
 * AUTHORS
 *
 *
 *                                                uC/OS-II
 *                                          The Real-Time Kernel
 *
 *                              (c) Copyright 2002, Micrium, Inc., Weston, FL
 *                                           All Rights Reserved
 *
 *                                                TI MSP430
 *********************************************************************************************************
 */

int global_pb_gd = 0;
int count_int_me;

#include <io.h>
#include <signal.h>
#include <iomacros.h>
#include <msp430x14x.h> // a voir si utile
#include "includes.h"
#include "radio_cfg.h"
#include "GestionMode.h"
#include "ServiceOutput.h"
#include "StatLogger.h"
#include "TraitementInput.h"
#include "Display.h"

/*
 *********************************************************************************************************
 *                                               CONSTANTS
 *********************************************************************************************************
 */

#define  TASK_STK_SIZE                  64       /* Size of each task's stacks (# of OS_STK entries)   */

#define          STATUS_LED_ON      P2OUT &= ~BIT1    //STATUS_LED - P2.1
#define          STATUS_LED_OFF     P2OUT |= BIT1     //STATUS_LED - P2.1	
//Port Output Register 'P1OUT, P2OUT':
#define P1OUT_INIT      0                       // Init Output data of port1
#define P2OUT_INIT      0                       // Init Output data of port2
//Port Direction Register 'P1DIR, P2DIR':
#define P2DIR_INIT      0xff                    // Init of Port2 Data-Direction Reg (Out=1 / Inp=0)
//Selection of Port or Module -Function on the Pins 'P1SEL, P2SEL'
#define P1SEL_INIT      0                       // P1-Modules:
#define P2SEL_INIT      0                       // P2-Modules:
//Interrupt capabilities of P1 and P2
#define P1IE_INIT       1                       // Interrupt Enable (0=dis 1=enabled)
#define P2IE_INIT       0                       // Interrupt Enable (0=dis 1=enabled)
#define P1IES_INIT      1                       // Interrupt Edge Select (0=pos 1=neg)
#define P2IES_INIT      0                       // Interrupt Edge Select (0=pos 1=neg)
#define WDTCTL_INIT     WDTPW|WDTHOLD

//Valeur assigné au message en cas d'erreur de lecture pendant les interruptions
#define InputError		4

#define	MSG_Q_SIZE		10
/*
 *********************************************************************************************************
 *                                               VARIABLES
 *********************************************************************************************************
 */

static OS_STK StkTraitementInput[TASK_STK_SIZE];
static OS_STK StkGestionMode[TASK_STK_SIZE];
static OS_STK StkServiceOutput[TASK_STK_SIZE];
static OS_STK StkLoggerStat[TASK_STK_SIZE];

OS_EVENT *ISR_To_TI_MsgQ;
OS_EVENT *TI_To_GM_MsgQ;
OS_EVENT *GM_To_SO_MsgQ;
OS_EVENT *GM_To_SL_MsgQ;

/*
 *********************************************************************************************************
 *                                           FUNCTION PROTOTYPES
 *********************************************************************************************************
 */

//void   Enable_XT2(void);                /* Enable XT2 and use it as the clock source          */
interrupt (PORT1_VECTOR) ButtInterrupt(void);
interrupt (PORT2_VECTOR) TelInterrupt(void);

/*
 *********************************************************************************************************
 *                                                MAIN
 *********************************************************************************************************
 */

int main(void) {

	WDTCTL = WDTCTL_INIT; //Init watchdog timer

	P6OUT = P1OUT_INIT; //Init output data of port1
	P6OUT = P1OUT_INIT; //Init output data of port2

	P6SEL = P1SEL_INIT; //Select port or module -function on port1
	P6SEL = P2SEL_INIT; //Select port or module -function on port2

	P6DIR = P1DIR_INIT; //Init port direction register of port1

	P1IES = P1IES_INIT; //init port interrupts
	P2IES = P2IES_INIT;

	P2IE = P2IE_INIT;
// changement au vue de tournier , 3 lignes
	P2SEL = 0;
	P2OUT = 0;
	P2DIR = ~BIT0; //only P2.0 is input

	/*Initialisation ineruptions Buttons et Irda*/

	//Pour avoir les pins en interruptions, il faut configurer
	P1SEL = 0; //
	P2SEL = 0; // sélection "input/output" (0) au lieu de "périphérique" (1)

	P1DIR = 0x00;
	P2DIR = 0x00;
	P1IES = 0xFF;

	P2IES = 0; //-> savoir si c'est un front montant (0) ou descendant (1)
	P1IFG = 0;
	P2IFG = 0;
	//	il faut utiliser eint(); pour enable global interrupt, P1IE = 1 et P2IE = 1
	/*Fin initialisation*/

	InitPortsDisplay();
	initDisplay();
	clearDisplay();
	printString("Starting");

	WDTCTL = WDTPW + WDTHOLD; /* Disable the watchdog timer   */

	// TIMERA Configuration             /* Configure TIMERA for the system Tick source. */
	//
	TACTL = TASSEL1 + TACLR; /* Clear the Timer and set SMCLK as the source. */
	TACTL |= 0x00C0; /* Input divider is /8.  */
	TACCTL0 = CCIE; /* Enable the TACCR0 interrupt. */
	TACCR0 = 913; /* Load the TACCR0 register. Value must be defined by testing */

	OSInit(); /* Initialize uC/OS-II */

	void *ISR_To_TI_Buffer[MSG_Q_SIZE];
	void *TI_To_GM_Buffer[MSG_Q_SIZE];
	void *GM_To_SO_Buffer[MSG_Q_SIZE];
	void *GM_To_SL_Buffer[MSG_Q_SIZE];

	ISR_To_TI_MsgQ = OSQCreate(&ISR_To_TI_Buffer[0], MSG_Q_SIZE);
	TI_To_GM_MsgQ = OSQCreate(&TI_To_GM_Buffer[0], MSG_Q_SIZE);
	GM_To_SO_MsgQ = OSQCreate(&GM_To_SO_Buffer[0], MSG_Q_SIZE);
	GM_To_SL_MsgQ = OSQCreate(&GM_To_SL_Buffer[0], MSG_Q_SIZE);

	INT8U prio = 20;
	task_TI_Param tiParam;
	tiParam.ISR_To_TI_MsgQ = ISR_To_TI_MsgQ;
	tiParam.TI_To_GM_MsgQ = TI_To_GM_MsgQ;
	prio = 6; // most important priority
	OSTaskCreate(TraitementInput, (void *) &tiParam,
			&StkTraitementInput[TASK_STK_SIZE - 1], prio);

	task_GM_Param gmParam;
	gmParam.GM_To_SL_MsgQ = GM_To_SL_MsgQ;
	gmParam.GM_To_SO_MsgQ = GM_To_SO_MsgQ;
	gmParam.TI_To_GM_MsgQ = TI_To_GM_MsgQ;
	prio = 9;
	OSTaskCreate(GestionMode, (void *) &gmParam,
			&StkGestionMode[TASK_STK_SIZE - 1], prio);

	prio = 11;
	OSTaskCreate(ServiceOutput, (void *) GM_To_SO_MsgQ,
			&StkServiceOutput[TASK_STK_SIZE - 1], prio);

	prio = 13;
	OSTaskCreate(StatLogger, (void *) GM_To_SL_MsgQ,
			&StkLoggerStat[TASK_STK_SIZE - 1], prio);

	clearDisplay();
	printString("Start OS");
	count_int_me = 0;

	P1IE = ~BIT0;
	eint();
	TACTL |= MC1; /* Start the Timer in Continuous mode. */
	OSStart();
	return (0);
}

/*
 *********************************************************************************************************
 *                                            STARTUP TASK
 *********************************************************************************************************
 */

interrupt (PORT1_VECTOR) ButtInterrupt(void) {

	INT8U err;
	INT8U P4Buffer;
	InputEvent msg;
	OS_CPU_SR cpu_sr = 0;

	//désactiver les interruptions
	P1IE = 0;
	OS_ENTER_CRITICAL(); //save cpu status register locally end restore it when finished
	//OSIntEnter(); DOESNT WORK

	//récupérer les informations des boutons
	Delayx100us(10);

	msg.bEvent  = BUTERR;
	msg.msgType = IT_BUTTON;
	P4Buffer = P4IN;

	if (!(P4Buffer & 0x10)) {
		msg.bEvent = BUT0;
	}
	if (!(P4Buffer & 0x20)) {
		msg.bEvent = BUT1;
	}
	if (!(P4Buffer & 0x40)) {
		msg.bEvent = BUT2;
	}
	if (!(P4Buffer & 0x80)) {
		msg.bEvent = BUT3;
	}
	//Pour éviter de rester bloqué en cas d'erreur on incrémente une variable et on la compare avec une valeur arbitraire
	//On assigne une valeur "ERREUR" au message, on pourra le traiter de façon particulière


	if (msg.bEvent != BUTERR) {
		err = OSQPost(ISR_To_TI_MsgQ, (void *) &msg);
	}
	//OSIntExit(); DOESNT WORK
	OS_EXIT_CRITICAL();

	//remise du sémaphore à 0
	P1IFG = 0;
	//réactiver les interruptions
	P1IE = 0xFF;

}

//todo: a finir
interrupt (PORT2_VECTOR) TelInterrupt(void) {
	//désactiver les interruptions
	P2IE = 0;

	//remise des sémaphores à 0
	P2IFG = 0;

	//récupération des infos -> lesquelles et comment? Comment marche la liaison série?

	//transmission par MB ou MQ au traitement input

	//réactiver les interruptions
	P2IE = 1;
}

/*
 void Enable_XT2(void)
 {
 int i;
 volatile int j;

 i=1;
 while(i)
 {
 _BIS_SR(OSCOFF);
 BCSCTL1 = 0;            // XT2 is On
 IFG1 &= ~OFIFG;         // Clear the Oscillator Fault interrupt flag.

 for(j=0;j<1000;j++);    // Wait for a little bit.

 if(!(IFG1 & OFIFG))     // If OFIFG remained cleared we are ready to go.
 {                       // Otherwise repeat the process until it stays cleared.
 i = 0;
 }
 }

 BCSCTL2 = 0x88;             // Select XT2CLK for MCLK and SMCLK
 }
 */
