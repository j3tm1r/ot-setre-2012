#define PERIOD 1000
#define HIGH_TIME 50
#define bitset(var,bitno) ((var) |= 1 << (bitno))
#define bitclr(var,bitno) ((var) &= ~(1 << (bitno)))

#define          B1                 BIT4&P4IN         //B1 - P4.4
#define          B2                 BIT5&P4IN         //B2 - P4.5
#define          B3                 BIT6&P4IN         //B3 - P4.6
#define          B4                 BIT7&P4IN         //B4 - P4.7
#define          FREQ               BIT0&P1IN         //FREQuency input - P1.0
#define          DI1                BIT1&P1IN         //Digital Input 1 - P1.1
#define          DI2                BIT2&P1IN         //Digital Input 2 - P1.2
#define          DI3                BIT3&P1IN         //Digital Input 3 - P1.3
#define          DI4                BIT4&P1IN         //Digital Input 4 - P1.4
#define          DALLAS             BIT7&P1IN         //DALLAS input - P1.7
#define          P20                BIT0&P2IN         //P20 input
#define          SDA                BIT0&P4IN         //SDA
#define          SCL                BIT1&P4IN         //SCL
#define          STATUS_LED_ON      P2OUT &= ~BIT1    //STATUS_LED - P2.1
#define          STATUS_LED_OFF     P2OUT |= BIT1     //STATUS_LED - P2.1
#define          RELAY1_ON          P1OUT |= BIT5     //RELAY1 - P1.5
#define          RELAY1_OFF         P1OUT &= ~BIT5    //RELAY1 - P1.5
#define          RELAY2_ON          P1OUT |= BIT6     //RELAY1 - P1.6
#define          RELAY2_OFF         P1OUT &= ~BIT6    //RELAY1 - P1.6
#define          BUZ1_ON            P4OUT |= BIT2     //P4.2
#define          BUZ1_OFF           P4OUT &= ~BIT2    //P4.2
#define          BUZ2_ON            P4OUT |= BIT3     //P4.3
#define          BUZ2_OFF           P4OUT &= ~BIT3    //P4.3
#define          LCD_Data           P2OUT
#define          _100us             100                //
#define          _10us              10                 //
#define          E                  3                 //P2.3
#define          RS                 2                 //P2.2
#define          CR                 0x0d
#define          LF                 0x0a
#define          BUTTON_TIME        100

//============================================================================= //
//Port Output Register 'P1OUT':
#define P1OUT_INIT      0                       // Init Output data of port1
//Port Direction Register 'P1DIR':
#define P1DIR_INIT      0x7f                    // Init of Port1 Data-Direction Reg (Out=1 / Inp=0)
//Selection of Port or Module -Function on the Pins 'P1SEL'
#define P1SEL_INIT      0                       // P1-Modules:
//Interrupt capabilities of P1 and P2
#define P1IE_INIT       1                       // Interrupt Enable (0=dis 1=enabled)
#define P1IES_INIT      1                       // Interrupt Edge Select (0=pos 1=neg)

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
#define			P20_OFF			P2OUT &= ~BIT0     	//P2.0
#define			TXD0_ON				P3OUT |= BIT4     	//P3.4
#define			TXD0_OFF			P3OUT &= ~BIT4     	//P3.4

#define		DISP_ON			0x0c	        //LCD control constants
#define		DISP_OFF		0x08	        //
#define		CLR_DISP		0x01    	//
#define		CUR_HOME		0x02	        //
#define		ENTRY_INC		0x06            //
#define		DD_RAM_ADDR		0x80	        //
#define		DD_RAM_ADDR2		0xc0	        //
#define		DD_RAM_ADDR3		0x28	        //
#define		CG_RAM_ADDR		0x40	        //
