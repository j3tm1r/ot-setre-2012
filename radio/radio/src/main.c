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
#define P1DIR_INIT      0xff                    // Init of Port1 Data-Direction Reg (Out=1 / Inp=0)
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
/*
 *********************************************************************************************************
 *                                               VARIABLES
 *********************************************************************************************************
 */

OS_STK TaskStartStk[TASK_STK_SIZE];
OS_STK TaskStartStk2[TASK_STK_SIZE];

OS_STK StkServiceOutput;
OS_STK StkGestionMode;

/*
 *********************************************************************************************************
 *                                           FUNCTION PROTOTYPES
 *********************************************************************************************************
 */

void TaskStart(void *data); /* Function prototypes of Startup task                */

void TaskStart2(void *data); /* test task             */
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
	P6DIR = P2DIR_INIT; //Init port direction register of port2

	P1IES = P1IES_INIT; //init port interrupts
	P2IES = P2IES_INIT;
	P1IE = 0xff;
	P2IE = P2IE_INIT;
// changement au vue de tournier , 3 lignes
	P2SEL = 0;
	P2OUT = 0;
	P2DIR = ~BIT0; //only P2.0 is input

	/*Initialisation ineruptions Buttons et Irda*/

	//Pour avoir les pins en interruptions, il faut configurer
	P1SEL = 0; //
	P2SEL = 0; // sélection "input/output" (0) au lieu de "périphérique" (1)
	P1DIR = ~BIT0;
	P2DIR = ~BIT0;
	P1IES = 1;
	P2IES = 0; //-> savoir si c'est un front montant (0) ou descendant (1)
	P1IFG = 0;
	P2IFG = 0;
	//	il faut utiliser eint(); pour enable global interrupt, P1IE = 1 et P2IE = 1
	/*Fin initialisation*/

	eint();
	InitPorts();
	initDisplay();
	clearDisplay();
	printString("Starting");

	WDTCTL = WDTPW + WDTHOLD; /* Disable the watchdog timer   */

	//  P6SEL = 0x00;                       /* Port1 is set to GPIO         */
	//   P6DIR = 0x01;                       /* P1.0 is the only output.     */
	//  P6OUT = 0x00;                       /* P1.0 initially low.          */

	// TIMERA Configuration             /* Configure TIMERA for the system Tick source. */
	//
	TACTL = TASSEL1 + TACLR; /* Clear the Timer and set SMCLK as the source. */
	TACTL |= 0x00C0; /* Input divider is /8.  		*/
	TACCTL0 = CCIE; /* Enable the TACCR0 interrupt. */
	TACCR0 = 2304; /* Load the TACCR0 register.    	*/

	OSInit(); /* Initialize uC/OS-II                     */

	/*  P6OUT = 0;*/

	void *CommMsg[10];
	OS_EVENT *msgQServiceOutput = OSQCreate(&CommMsg[0], 10);
	INT8U prio = 0;

	OSTaskCreate(GestionMode, (void *) msgQServiceOutput,
			&TaskStartStk[TASK_STK_SIZE - 1], prio);

	prio = 5;

	OSTaskCreate(ServiceOutput, (void *) msgQServiceOutput,
			&TaskStartStk2[TASK_STK_SIZE - 1], prio);
	clearDisplay();
	printString("Start OS");
	count_int_me = 0;
	OSStart();
	return (0);
}

/*
 *********************************************************************************************************
 *                                            STARTUP TASK
 *********************************************************************************************************
 */

interrupt (PORT1_VECTOR) ButtInterrupt(void) {
	INT8U poll = 0;
	uint8_t P4Buffer;
	InputEvent Message;
	OS_CPU_SR cpu_sr = 0;

	//désactiver les interruptions
	P1IE = 0;
	OS_ENTER_CRITICAL(); /*save cpu status register locally end restore it when finished*/
	OSIntEnter();

	//remise du sémaphore à 0
	P1IFG = 0;
	clearDisplay();
	printDecimal(count_int_me++);
	//récupérer les informations des boutons
	Message.bEvent = 4;

	//while (Message.bEvent == 4)
	{
		P4Buffer = P4IN;
		gotoSecondLine();
		printString("In");
		if (!(P4Buffer & 0x10)) {
			Message.bEvent = 0;

		}
		if (!(P4Buffer & 0x20)) {
			Message.bEvent = 1;

		}
		if (!(P4Buffer & 0x40)) {
			Message.bEvent = 2;

		}
		if (!(P4Buffer & 0x80)) {
			Message.bEvent = 3;
		}

		//Pour éviter de rester bloqué en cas d'erreur on incrémente une variable et on la compare avec une valeur arbitraire
		//On assigne une valeur "ERREUR" au message, on pourra le traiter de façon particulière
	}
	printDecimal(Message.bEvent);
	printByte(P4IN);

	//todo:les transmettre par MailBox ou MessageQueue au TraitementInput

	OSIntExit();
	OS_EXIT_CRITICAL();
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
