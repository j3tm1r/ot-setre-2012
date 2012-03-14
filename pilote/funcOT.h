
void initDisplay() {
  InitOsc();
  InitPorts();
  InitLCD();
  clearDisplay();
}
void putc(char c) {
    SEND_CHAR(c);
}
void clearDisplay() {
    SEND_CMD(CLR_DISP);
    Delayx100us(10);
}
void gotoSecondLine() {
//    SEND_CMD(CLR_DISP);
    SEND_CMD(DD_RAM_ADDR2);
}
void printString(char *String) {
  while(*String)
    putc(*String++);
}
char HexDigit(int digitvalue) {
  if (digitvalue < 10)
    return(digitvalue + '0');
  else
    return(digitvalue + 'A' - 10);
}
void printByte(unsigned int theByte) {
  char HexBuffer[3];
  HexBuffer[2] = 0;
  HexBuffer[1] = HexDigit(theByte & 0x000f);
  theByte = theByte >> 4;
  HexBuffer[0] = HexDigit(theByte & 0x000f);
  printString(HexBuffer);
}
void printHex(unsigned int Number) {
  char HexBuffer[5];
  HexBuffer[4] = 0;
  HexBuffer[3] = HexDigit(Number & 0x000f);
  Number = Number >> 4;
  HexBuffer[2] = HexDigit(Number & 0x000f);
  Number = Number >> 4;
  HexBuffer[1] = HexDigit(Number & 0x000f);
  Number = Number >> 4;
  HexBuffer[0] = HexDigit(Number & 0x000f);
  printString(HexBuffer);
}
void printDecimal(int Number) {
  // need to move to long int to account for
  // negative 32768
  char DecimalBuffer[7];
  long lNumber = Number;
  DecimalBuffer[6] = 0;
  if (lNumber < 0) {
    DecimalBuffer[0] = '-';
    lNumber = -lNumber;
  } else
    DecimalBuffer[0] = '+';
  DecimalBuffer[5] = (lNumber % 10)+'0';
  lNumber = lNumber / 10;
  DecimalBuffer[4] = (lNumber % 10)+'0';
  lNumber = lNumber / 10;
  DecimalBuffer[3] = (lNumber % 10)+'0';
  lNumber = lNumber / 10;
  DecimalBuffer[2] = (lNumber % 10)+'0';
  lNumber = lNumber / 10;
  DecimalBuffer[1] = (lNumber % 10)+'0';
  printString(DecimalBuffer);
}
void InitOsc(void){
  WDTCTL = WDTPW | WDTHOLD;                      // stop watchdog timer

  BCSCTL1 |= XTS;                                // XT1 as high-frequency
  _BIC_SR(OSCOFF);                               // turn on XT1 oscillator

  do                                             // wait in loop until crystal is stable
    IFG1 &= ~OFIFG;
  while (IFG1 & OFIFG);

  BCSCTL1 |= DIVA0;                              // ACLK = XT1 / 2
  BCSCTL1 &= ~DIVA1;

  IE1 &= ~WDTIE;                                 // disable WDT int.
  IFG1 &= ~WDTIFG;                               // clear WDT int. flag

  WDTCTL = WDTPW | WDTTMSEL | WDTCNTCL | WDTSSEL | WDTIS1; // use WDT as timer, flag each
                                                           // 512 pulses from ACLK

  while (!(IFG1 & WDTIFG));                      // count 1024 pulses from XT1 (until XT1's
                                                 // amplitude is OK)

  IFG1 &= ~OFIFG;                                // clear osc. fault int. flag
  BCSCTL2 |= SELM0 | SELM1;                      // set XT1 as MCLK
}
void InitPorts(void){
  P1SEL = 0;                                     //
  P1OUT = 0;                                     //
  P1DIR = BIT5 | BIT6;                           //enable only Relay outputs
  
  P2SEL = 0;
  P2OUT = 0;
  P2DIR = ~BIT0;                                // enable only P2.0 as output

  P3SEL |= BIT4 | BIT5;                         //enable UART0
  P3DIR |= BIT4;                                //enable TXD0 as output
  P3DIR &= ~BIT5;                               //enable RXD0 as input

  P4SEL = 0;
  P4OUT = 0;
  P4DIR = BIT2 | BIT3;                          //only buzzer pins are outputs

  P6SEL = 0;
  P6OUT = 0;
  P6DIR = 0xff;                                  // all output
}
void Delay (unsigned int a){
  int k;
  for (k=0 ; k != a; ++k) {
    _NOP();
    _NOP();
    _NOP();
    _NOP();
  }
}
void Delayx100us(unsigned char b){
  int j;
  for (j=0; j!=b; ++j) Delay (_100us);
}
void _E(void){
        bitset(P2OUT,E);		//toggle E for LCD
	Delay(_10us);
	bitclr(P2OUT,E);
}
void SEND_CHAR (unsigned char d){
        int temp;
	Delayx100us(5);                 //.5ms	
	temp = d & 0xf0;		//get upper nibble	
	LCD_Data &= 0x0f;
	LCD_Data |= temp;
	bitset(P2OUT,RS);     	        //set LCD to data mode
	_E();                           //toggle E for LCD
	temp = d & 0x0f;
	temp = temp << 4;               //get down nibble
	LCD_Data &= 0x0f;
	LCD_Data |= temp;
	bitset(P2OUT,RS);   	        //set LCD to data mode
	_E();                           //toggle E for LCD
}
void SEND_CMD (unsigned char e){
        int temp;
	Delayx100us(10);                //10ms
	temp = e & 0xf0;		//get upper nibble	
	LCD_Data &= 0x0f;
	LCD_Data |= temp;               //send CMD to LCD
	bitclr(P2OUT,RS);     	        //set LCD to CMD mode
	_E();                           //toggle E for LCD
	temp = e & 0x0f;
	temp = temp << 4;               //get down nibble
	LCD_Data &= 0x0f;
	LCD_Data |= temp;
	bitclr(P2OUT,RS);   	        //set LCD to CMD mode
	_E();                           //toggle E for LCD
}
void InitLCD(void){
    bitclr(P2OUT,RS);
    Delayx100us(250);                   //Delay 100ms
    Delayx100us(250);
    Delayx100us(250);
    Delayx100us(250);
    LCD_Data |= BIT4 | BIT5;            //D7-D4 = 0011
    LCD_Data &= ~BIT6 & ~BIT7;
    _E();                               //toggle E for LCD
    Delayx100us(100);                   //10ms
    _E();                               //toggle E for LCD
    Delayx100us(100);                   //10ms
    _E();                               //toggle E for LCD
    Delayx100us(100);                   //10ms
    LCD_Data &= ~BIT4;
    _E();                               //toggle E for LCD

    SEND_CMD(DISP_ON);
    SEND_CMD(CLR_DISP);
    Delayx100us(250);
    Delayx100us(250);
    Delayx100us(250);
    Delayx100us(250);
}
void PRINT_BAR(unsigned char segment, unsigned char etat) {
	SEL_ON;	
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
		Delayx100us(1500);
		PRINT_BAR(i,led_off);
	}
}
void FALL_BAR(void) {
	unsigned char i=0;
	unsigned char led_on=1;
	unsigned char led_off=0;
for (i=7; i>0 ;i--)
	{
		PRINT_BAR(i,led_on);
		Delayx100us(1500);
		PRINT_BAR(i,led_off);
	}
}
void print_DAC(int nb){
	int i,k=0;
	int n=0;
	char bin[11];
	SEL_OFF;		// select CNA path
	CS_ON;
	//delay (20,1);	// (10,1) ->82µs
	Delayx100us(1);
	CS_OFF;
	i=1024;
	k=0;
	if (nb>1023) nb=1023;
	if (nb<0) nb=0;
	n=nb;
		while (k<11)
		{
			if ((n-i)<0) 
			{  
				P6OUT &=~0x70;	// DIN=0 	SCLK=0	 CS=0 	
			
				P6OUT |=0x20;	//SCLK=1
			
				P6OUT &=~0x20;	// SCLK=0
			
				bin[k]='0';
			}
			else if ((n-i)>=0)
			{
				P6OUT |=0x10;	// DIN=1 	SCLK=0	 CS=0 
							
				P6OUT |=0x20;	//SCLK=1
				
				P6OUT &=~0x20;	// SCLK=0
				
				bin[k]='1';
				n=n-i;
			}
			i=i/2;
			k=k+1;  
		}		
		// dumy LSB 2 bits
	P6OUT &=~0x70;	// DIN=0 	SCLK=0	 CS=0 
	P6OUT |=0x20;	//SCLK=1
	P6OUT &=~0x20;	// SCLK=0

	P6OUT &=~0x70;	// DIN=0 	SCLK=0	 CS=0 	
	P6OUT |=0x20;	//SCLK=1
	P6OUT &=~0x20;	// SCLK=0

	
	CS_ON;	
	delay (200,10);
	SEL_ON;		// deselect CNA path
	}	
void PRINT_POT(int cmd, int nb)		{
// value should be 0<val<255
	int i,k=0;
	int n=0;
	char bin[8];
	SEL_ON;				// select POT-NUM path
	CS_ON;
	//delay (20,1);	// (10,1) ->82µs
	Delayx100us(1);
	CS_OFF;
	
	// cmde byte for POT :8 higher 16bits  
	// 0b 0001.0001 > write data on POT0
	// 0b 0010.0001 > shutdown POT0
	// 0b 0011.000x > no command executed	
	i=128;
	k=0;
	if (cmd>255) cmd=255;
	if (cmd<0) cmd=0;
	n=cmd;
		while (k<8)
		{
			if ((n-i)<0) 
			{  
				P6OUT &=~0x70;	// DIN=0 	SCLK=0	 CS=0 	
			
				P6OUT |=0x20;	//SCLK=1
			
				P6OUT &=~0x20;	// SCLK=0
			
				bin[k]='0';
			}
			else if ((n-i)>=0)
			{
				P6OUT |=0x10;	// DIN=1 	SCLK=0	 CS=0 
							
				P6OUT |=0x20;	//SCLK=1
				
				P6OUT &=~0x20;	// SCLK=0
				
				bin[k]='1';
				n=n-i;
			}
			i=i/2;
			k=k+1;  
		}	
	
	i=128;
	k=0;
	if (nb>255) nb=255;
	if (nb<0) nb=0;
	n=nb;
		while (k<8)
		{
			if ((n-i)<0) 
			{  
				P6OUT &=~0x70;	// DIN=0 	SCLK=0	 CS=0 	
			
				P6OUT |=0x20;	//SCLK=1
			
				P6OUT &=~0x20;	// SCLK=0
			
				bin[k]='0';
			}
			else if ((n-i)>=0)
			{
				P6OUT |=0x10;	// DIN=1 	SCLK=0	 CS=0 
							
				P6OUT |=0x20;	//SCLK=1
				
				P6OUT &=~0x20;	// SCLK=0
				
				bin[k]='1';
				n=n-i;
			}
			i=i/2;
			k=k+1;  
		}	
		
	CS_ON;	
	delay (200,10);
	SEL_OFF;		// deselect POT path
}		
void delay(unsigned int minor, unsigned int major) {
  int i,j;
  for(j=0; j < major; j++)
    {
      for (i = 0; i<minor; i++) 
	   {
	     nop();
	     nop();
	   }
    }
}

int DelayCounter;
