/*
 * BarGraph.h
 *
 *  Created on: Mar 6, 2012
 *      Author: ot
 */

#ifndef BARGRAPH_H_
#define BARGRAPH_H_

#include <io.h>
#include <signal.h>
#include <iomacros.h>
#include <msp430x14x.h> // a voir si utile


#define			D_ON				P6OUT |= BIT0     	//P6.0
#define			D_OFF				P6OUT &= ~BIT0     	//P6.0
#define			S0_ON				P6OUT |= BIT1     	//P6.1
#define			S0_OFF				P6OUT &= ~BIT1     	//P6.1
#define			S1_ON				P6OUT |= BIT2     	//P6.2
#define			S1_OFF				P6OUT &= ~BIT2     	//P6.2
#define			S2_ON				P6OUT |= BIT3     	//P6.3
#define			S2_OFF				P6OUT &= ~BIT3     	//P6.3
#define			DIN_ON				P6OUT |= BIT4     	//P6.4
#define			DIN_OFF				P6OUT &= ~BIT4     	//P6.4
#define			SCLK_ON				P6OUT |= BIT5     	//P6.5
#define			SCLK_OFF			P6OUT &= ~BIT5     	//P6.5
#define			CS_ON				P6OUT |= BIT6     	//P6.6
#define			CS_OFF				P6OUT &= ~BIT6     	//P6.6
#define			SEL_ON				P6OUT |= BIT7     	//P6.7
#define			SEL_OFF				P6OUT &= ~BIT7     	//P6.7


void PRINT_BAR(unsigned char segment, unsigned char etat);
void RISE_BAR();
void FALL_BAR();


#endif /* BARGRAPH_H_ */
