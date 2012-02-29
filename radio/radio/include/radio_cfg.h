/*
 * config.h
 *
 *  Created on: 27 févr. 2012
 *      Author: mbbadau
 */

#ifndef CONFIG_H_
#define CONFIG_H_


// Helper
// Eclipse run conf : ../simulator/wsim-otsetre --ui exe/main.elf

#define UNUSED(p) ((void)(p))

//============================================================================	//
//				CONFIGURATION CARTE EXTENSION  EASYWEB2			//
//============================================================================	//

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

#define			P20_ON				P2OUT |= BIT0     	//P2.0
#define			P20_OFF				P2OUT &= ~BIT0     	//P2.0
#define			TXD0_ON				P3OUT |= BIT4     	//P3.4
#define			TXD0_OFF			P3OUT &= ~BIT4     	//P3.4

#define			DISP_ON				0x0c	        //LCD control constants
#define			DISP_OFF			0x08	        //
#define			CLR_DISP			0x01    	//
#define			CUR_HOME			0x02	        //
#define			ENTRY_INC			0x06            //
#define			DD_RAM_ADDR			0x80	        //
#define			DD_RAM_ADDR2		0xc0	        //
#define			DD_RAM_ADDR3		0x28	        //
#define			CG_RAM_ADDR			0x40	        //

#endif /* CONFIG_H_ */
