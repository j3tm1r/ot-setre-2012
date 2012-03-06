/*
 * BarGraph.c
 *
 *  Created on: Mar 6, 2012
 *      Author: ot
 */

#include "BarGraph.h"


void PRINT_BAR(unsigned char segment, unsigned char etat) {
	SEL_ON;
	if (segment> 8)	{segment=0;}
	if (etat>1) 	{etat=0;}
	if (etat==1) D_ON;
	if (etat==0) D_OFF;
	switch (segment)
	{
		case (0):
		{	S0_OFF;		//line selection 1/8 (msb) S2 S1 S0 (lsb)
			S1_OFF;
			S2_OFF;
			break;
		}
		case (1):
		{	S0_ON;		//line selection 1/8 (msb) S2 S1 S0 (lsb)
			S1_OFF;
			S2_OFF;
			break;
		}
		case (2):
		{	S0_OFF;		//line selection 1/8 (msb) S2 S1 S0 (lsb)
			S1_ON;
			S2_OFF;
			break;
		}
		case (3):
		{	S0_ON;		//line selection 1/8 (msb) S2 S1 S0 (lsb)
			S1_ON;
			S2_OFF;
			break;
		}
		case (4):
		{	S0_OFF;		//line selection 1/8 (msb) S2 S1 S0 (lsb)
			S1_OFF;
			S2_ON;
			break;
		}
		case (5):
		{	S0_ON;		//line selection 1/8 (msb) S2 S1 S0 (lsb)
			S1_OFF;
			S2_ON;
			break;
		}
		case (6):
		{	S0_OFF;		//line selection 1/8 (msb) S2 S1 S0 (lsb)
			S1_ON;
			S2_ON;
			break;
		}
		case (7):
		{	S0_ON;		//line selection 1/8 (msb) S2 S1 S0 (lsb)
			S1_ON;
			S2_ON;
			break;
		}

		default: break;
	}
	SEL_OFF;	//LATCH
	SEL_ON;
}
void RISE_BAR(void) {
	unsigned char i=0;
	unsigned char led_on=1;
	unsigned char led_off=0;
for (i=0; i<8 ;i++)
	{
		PRINT_BAR(i,led_on);
//		Delayx100us(2);
		//PRINT_BAR(i,led_off);
	}
}
void FALL_BAR(void) {
	unsigned char i=0;
	unsigned char led_on=1;
	unsigned char led_off=0;
for (i=7; i>0 ;i--)
	{
	//	PRINT_BAR(i,led_on);
	//	Delayx100us(2);
		PRINT_BAR(i,led_off);
	}
}
