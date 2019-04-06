
#include "io430.h"
#include <string.h>
#include "in430.h"

void deger_goster(unsigned int);
void a0_init(void);
unsigned int volt[2];
volatile char tick;
unsigned int tick1;


char* x;
int val=0;
unsigned int i=0;
char string1[200]={0};
unsigned char rx_flag=0;

int a=175,b=212;
void init_serial()
{
    P1SEL = BIT1 + BIT2 ;                     // P1.1 = RXD, P1.2=TXD
    P1SEL2 = BIT1 + BIT2 ;                     // P1.1 = RXD, P1.2=TXD
    UCA0CTL1 |= UCSSEL_2;                     // SMCLK
    UCA0BR0 = 104;                            // 1MHz 9600
    UCA0BR1 = 0;                              // 1MHz 9600
    UCA0MCTL = UCBRS0;                        // Modulation UCBRSx = 1
    UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
    IE2 |= UCA0RXIE;

}

void serial_putsc(unsigned char value)
{
    while (!(IFG2&UCA0TXIFG));                 // USCI_A0 TX buffer ready?
    UCA0TXBUF = value;                     // TX -> RXed character
}

void serial_puts(char* String)
{
    while(*String != '\n')
    {
        serial_putsc(*String);
        String++;
    }
    serial_putsc(*String);
}

void clear_buffer(void)
{
    for(i=199;i>0;i--)
    {
        string1[i] = 0;
    }
    string1[0] = 0;
    i=0;
    rx_flag=0;
}

unsigned int wifiCommand_ack(char* sCmd,int waitTm,char* sTerm)
{
    char lp = 0;
    while(lp < waitTm)
    {
        serial_puts(sCmd);
        _BIS_SR(LPM0_bits + GIE);
        while(waitTm--)
            __delay_cycles(1000);
        if(strstr(string1,sTerm) != 0)
        {
            for(;i>0;i--)
                string1[i] = 0;
            string1[0] = 0;
            i=0;
            rx_flag=0;
            return 1;
        }
        __delay_cycles(1000000);
        lp++;
    }
    for(;i>0;i--)
        string1[i] = 0;
    string1[0] = 0;
    i=0;
    rx_flag=0;
    return 0;
}

void wifiCommand_val(char* sCmd,int waitTm)
{
    serial_puts(sCmd);
    _BIS_SR(LPM0_bits + GIE);
    while(waitTm--)
        __delay_cycles(1000);
    //strcpy(string1,ret);
}

void init_esp()
{
    P1DIR|=BIT0;
    P1OUT&=~BIT0;
    __delay_cycles(10000);
    P1OUT|=BIT0;
    __delay_cycles(3000000);
    wifiCommand_ack("AT\r\n",10,"OK");
    wifiCommand_ack("AT+CIPCLOSE\r\n",10,"OK");
    val = wifiCommand_ack("AT+RST\r\n",2000,"ready");
    wifiCommand_ack("AT+CWMODE=3\r\n",10,"OK");
//    wifiCommand_val("AT+GMR\r\n",50);
//    clear_buffer();
}

void mode1()
{
    wifiCommand_ack("AT+CIPMUX=1\r\n",10,"OK");
    wifiCommand_ack("AT+CIPSERVER=1,9998\r\n",10,"OK");
}

unsigned char receive_data()
{
    if(rx_flag==1)
    {
        __delay_cycles(100000);
        return 1;
    }
    return 0;
}

void update_settings(short int* no_of_feeds,short int* feed_m,short int* feed_h,short int* feed_units)
{
    char* temp;
    unsigned int index;
    //decode protocol
    temp = strchr(string1,'f');
    index = temp-string1+1;
    *no_of_feeds = string1[index]-48;
    for(i=0; i<*no_of_feeds; i++)
    {
        temp = strchr(string1,'a'+i);
        index = temp-string1+1;
        *(feed_h+i) = (string1[index]-48)*10+(string1[index+1]-48);
        *(feed_m+i) = (string1[index+2]-48)*10+(string1[index+3]-48);
        feed_units[i] = string1[index+4]-48;
    }
    //prtocol decoded
    clear_buffer();
    rx_flag=0;
}





void print(char *text)
{
	unsigned int i = 0;
	while(text[i] != '\0')
	{
		while (!(IFG2&UCA0TXIFG));		// Check if TX is ongoing
		UCA0TXBUF = text[i];			// TX -> Received Char + 1
		i++;
	}
}

void printNumber(unsigned int num)
{
	char buf[6];
	char *str = &buf[5];

	*str = '\0';

	do
	{
		unsigned long m = num;
		num /= 10;
		char c = (m - 10 * num) + '0';
		*--str = c;
	} while(num);

	print(str);
}

int bekle(int x){
  
  while(x>0){
    x--;
      
     __delay_cycles(100000);
  }

}
int derece(int x){
    
  return x;

}

void main(void)
{
	WDTCTL = WDTPW + WDTHOLD;			                // Stop Watchdog
	if (CALBC1_1MHZ==0xFF)				                // Check if calibration constant erased
	{
		while(1);						// do not load program
	}
	DCOCTL = 0;							// Select lowest DCO settings
	BCSCTL1 = CALBC1_1MHZ;				                // Set DCO to 1 MHz
	DCOCTL = CALDCO_1MHZ;

	P1SEL = BIT1 + BIT2 ;				                // Select UART RX/TX function on P1.1,P1.2
	P1SEL2 = BIT1 + BIT2;

	UCA0CTL1 |= UCSSEL_3;				                // UART Clock -> SMCLK
	UCA0BR0 = 8;						        // Baud Rate Setting for 1MHz 115200
	UCA0BR1 = 0;						        // Baud Rate Setting for 1MHz 115200
	UCA0MCTL = UCBRS_6;				        	// Modulation Setting for 1MHz 115200
	UCA0CTL1 &= ~UCSWRST;				                // Initialize UART Module
       IE2 |= UCA0RXIE;                                                 // Enable USCI_A0 RX interrupt
       
        TA0CTL = TASSEL_2 + TACLR;
        TA0CCR0 = 50000-1;
        TA0CCTL0 = CCIE;
       

        a0_init();
  
        TA0CTL |= MC_1;
       
      __enable_interrupt();
   
      wifiCommand_ack("AT\r\n",10,"OK");
      bekle(20); clear_buffer();
      
      wifiCommand_ack("AT+RST\r\n",2000,"ready");
       bekle(20);              clear_buffer();
       
      wifiCommand_ack("AT+CWMODE=3\r\n",10,"OK");
      bekle(20);clear_buffer();
      
      wifiCommand_ack("AT+CWJAP=\"12345678\",\"manyaklopes\"\r\n",5000,"OK");
      bekle(20);      clear_buffer();
      
      wifiCommand_ack("AT+CIPMUX=0\r\n",10,"OK");
      bekle(20);
  
   
        
      
  /*
    wifiCommand_ack("AT+CIPSTART=\"TCP\",\"184.106.153.149\",80",50,"OK"); 
    wifiCommand_ack("AT+CIPSEND=60",50,">"); 
    wifiCommand_ack("GET /update?key=GENNUNE55L9BJDTO&field1=55\r\n\r\n",50,"OK"); 
    wifiCommand_ack("AT+CIPSTART=\"TCP\",\"184.106.153.149\",80  ",50,"OK"); 
    */
        
        
	while(1)
	{
              
            wifiCommand_ack("AT+CIPSTART=\"TCP\",\"184.106.153.149\",80\r\n",50,"OK");
    //  wifiCommand_ack("AT+CIPSTART=\"TCP\",\"184.106.153.149\",80",50,"OK"); 
            bekle(20);  clear_buffer();
      
     // wifiCommand_ack("AT+CIPSEND=65",10,">"); 
            wifiCommand_ack("AT+CIPSEND=100\r\n",10,">");
      
            bekle(20);  clear_buffer();
      
       // wifiCommand_ack("GET /update?key=GENNUNE55L9BJDTO&field1=55\r\n\r\n",50,"OK"); 
      // x = strcat("GET https://api.thingspeak.com/update?api_key=GENNUNE55L9BJDTO&field1=","100&field2=50");
    
       print("GET https://api.thingspeak.com/update?api_key=GENNUNE55L9BJDTO&field1=");
       printNumber((unsigned int)volt[1]*5);
       print("&field2=");
       printNumber((((unsigned int)volt[0] - 673)*423)/1024);
       print("\r\n\r\n");
      // print("GET https://api.thingspeak.com/update?api_key=GENNUNE55L9BJDTO&field1=150&field2=50\r\n\r\n");
       
        clear_buffer(); 
        bekle(20);
        wifiCommand_ack("AT+CIPCLOSE",10,"OK");
        bekle(20);clear_buffer();
     // wifiCommand_ack("GET /update?api_key=GENNUNE55L9BJDTO&field1=55\r\n\r\nAT+CIPCLOSE",1000,"OK"); 
	bekle(300);	
        }
}



void a0_init()
{
   ADC10CTL0 &= ~ENC;
  ADC10CTL0 =  SREF_1 + ADC10SHT_3 + REFON + ADC10ON + MSC + ADC10IE;
  ADC10CTL1 = INCH_10 + ADC10SSEL_2 + ADC10DIV_0 + CONSEQ_1;
  ADC10AE0  = BIT3 ;
  ADC10DTC1 = 0x02;        //2 Çevrim
}




#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
    string1[i++] = UCA0RXBUF;
    _BIC_SR_IRQ(LPM0_bits);
    rx_flag=1;
}





// TimerA0 Kesme Vektörü
#pragma vector=TIMER0_A0_VECTOR
__interrupt void ta0_isr(void)
{
  tick++;
  if(tick==10)
  {
   
    ADC10CTL0 &= ~ENC;
    
    while (ADC10CTL1 & BUSY);             // Wait if ADC10 core is active
    ADC10SA = (unsigned int)&volt;
    ADC10CTL0 |= ENC + ADC10SC;
    
    tick=0;
  }
}



// ADC10 Kesme Vektörü
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
  //__bic_SR_register_on_exit(CPUOFF);        // Clear CPUOFF bit from 0(SR)
  __no_operation();
}





