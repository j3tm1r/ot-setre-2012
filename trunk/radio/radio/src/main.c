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
#include "util/cmdBuffer.h"

/*
 *********************************************************************************************************
 *                                               CONSTANTS
 *********************************************************************************************************
 */

#define  TI_STK_SIZE                64       /* Size of each task's stacks (# of OS_STK entries)   */
#define  GM_STK_SIZE                128
#define  SO_STK_SIZE                160
#define  SL_STK_SIZE                64

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

#define	MSG_Q_SIZE		3
/*
 *********************************************************************************************************
 *                                               VARIABLES
 *********************************************************************************************************
 */

static OS_STK StkTraitementInput[TI_STK_SIZE];
static OS_STK StkGestionMode[GM_STK_SIZE];
static OS_STK StkServiceOutput[SO_STK_SIZE];
static OS_STK StkLoggerStat[SL_STK_SIZE];

INT16S ISR_To_TI_CmdBuf;
INT16S TI_To_GM_CmdBuf;
INT16S GM_To_SO_CmdBuf;
INT16S GM_To_SL_CmdBuf;

OS_EVENT *ISR_To_TI_MsgQ;
OS_EVENT *TI_To_GM_MsgQ;
OS_EVENT *GM_To_SO_MsgQ;
OS_EVENT *GM_To_SL_MsgQ;

INT8U statLoggerPrio;

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

	WDTCTL = WDTCTL_INIT; // Init watchdog timer

	P6OUT = P1OUT_INIT; // Init output data of port1
	P6OUT = P1OUT_INIT; // Init output data of port2

	P6SEL = P1SEL_INIT; // Select port or module -function on port1
	P6SEL = P2SEL_INIT; // Select port or module -function on port2

	P6DIR = 0xFF; // Init port direction register of port6

	P1IES = P1IES_INIT; // init port interrupts
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
	//      il faut utiliser eint(); pour enable global interrupt, P1IE = 1 et P2IE = 1
	/*Fin initialisation*/

	// Configure port 4 pin 1 (I2C SCL)
	P4DIR |= BIT1;
	P4OUT |= BIT1;

	// Init buzzer
	P4SEL = 0;
	P4OUT &= ~BIT2;
	P4OUT &= ~BIT3;
	P4DIR |= BIT2;
	P4DIR |= BIT3; //only buzzer pins are outputs

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

	ISR_To_TI_CmdBuf = InitCmdBuffer(MSG_Q_SIZE, sizeof(InputEvent));
	TI_To_GM_CmdBuf = InitCmdBuffer(MSG_Q_SIZE, sizeof(InputCmd));
	GM_To_SO_CmdBuf = InitCmdBuffer(MSG_Q_SIZE, sizeof(ServiceMsg));
	GM_To_SL_CmdBuf = InitCmdBuffer(MSG_Q_SIZE, sizeof(StatMsg));

	INT8U prio = 20;
	prio = 6; // most important priority
	OSTaskCreate(TraitementInput, NULL, &StkTraitementInput[TI_STK_SIZE - 1],
			prio);

	statLoggerPrio = prio = 13;
	OSTaskCreate(StatLogger, NULL, &StkLoggerStat[SL_STK_SIZE - 1], prio);

	prio = 9;
	OSTaskCreate(GestionMode, NULL, &StkGestionMode[GM_STK_SIZE - 1], prio);

	prio = 11;
	OSTaskCreate(ServiceOutput, NULL, &StkServiceOutput[SO_STK_SIZE - 1], prio);

	clearDisplay();
	printString("Start OS");
	count_int_me = 0;



	/*
	 * Configuration for sending data through irda
	 * */

	//Set SWRST
	U0CTL = SWRST;

	//Initialize registers

	//      BCSCTL2 |= SELS; //select XT2CLK as SMCLK source
	//      BCSCTL2 |= DIVS0;
	//      BCSCTL2 |= DIVS1; //divise SSMCLK par 8 -> réglée à 1MHz

	U0CTL |= CHAR; // data on 8bits and we keep no parity, one stop bit, , disable listen mode, UART mode, no multi-processor protocol
	U0TCTL |= SSEL0 | SSEL1; //select SMCLK pour l'IRDA
	U0RCTL &= ~(URXEIE | URXWIE); //disable interrupts on receive erroneous character and wake-up
	//Baud Rate 14399
	U0BR1 = 0x02;
	U0BR0 = 0x2C; //division of SMCLK by 556
	U0MCTL = 0b00111111;
	/*sending data*/
	//U0CTL |= 0b00011001;
	//U0TCTL |= 0b00010000;
	//Enable USART module via the MEx SFRs (URXEx and/or UTXEx)
	ME1 &= ~UTXE0;
	ME1 |= URXE0;

	//Clear SWRST
	U0CTL &= ~SWRST;

	//Enable interrupts
	IE1 &= ~UTXIE0;
	IE1 |= URXIE0;

	/*
	 * */

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
//	ServiceMsg msgV;

	//désactiver les interruptions
	P1IE = 0;
	OS_ENTER_CRITICAL(); //save cpu status register locally end restore it when finished
	//OSIntEnter(); DOESNT WORK

	//récupérer les informations des boutons
	Delayx100us(10);

//	msgV.serviceType = SERV_FREQ;

	msg.bEvent = BUTERR;
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
		if (Queue(ISR_To_TI_CmdBuf, &msg) == 0) {
			err = OSQPost(ISR_To_TI_MsgQ, (void *) ISR_To_TI_CmdBuf);
		}
	}
	//OSQPost(GM_To_SO_MsgQ, (void *) &msgV);
	//OSIntExit(); DOESNT WORK
	OS_EXIT_CRITICAL();

	//remise du sémaphore à 0
	P1IFG = 0;
	//réactiver les interruptions
	P1IE = 0xFF;
}

interrupt (USART0RX_VECTOR ) TelInterrupt(void) {
	INT8U recvd;
	InputEvent msg;
	OS_CPU_SR cpu_sr = 0;
	INT8U err;

	OS_ENTER_CRITICAL(); //save cpu status register locally end restore it when finished

	//Disable interrupts
	IE1 &= ~UTXIE0;
	IE1 &= ~URXIE0;

	recvd = U0RXBUF;
	//      msgV.serviceType = SERV_FREQ;

	msg.msgType = IT_TLC;
	msg.tcEvent = recvd;

	if (Queue(ISR_To_TI_CmdBuf, &msg) == 0) {
		err = OSQPost(ISR_To_TI_MsgQ, (void *) ISR_To_TI_CmdBuf);
	}
//      clearDisplay();
//      printString("Rcvd : ");
//      printDecimal(recvd);

	//Enable interrupts
	IE1 &= ~UTXIE0;
	IE1 |= URXIE0;

	OS_EXIT_CRITICAL();
}

