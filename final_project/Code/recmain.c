/*  -- Reciever Side --
************************************************************************
 ECE 362 - Mini-Project C Source File - Fall 2015
***********************************************************************
                                                                                                                          
 Team ID: < ? >

 Project Name: < ? >

 Team Members:

   - Team/Doc Leader: < ? >      Signature: ______________________
   
   - Software Leader: < ? >      Signature: ______________________

   - Interface Leader: < ? >     Signature: ______________________

   - Peripheral Leader: < ? >    Signature: ______________________


 Academic Honesty Statement:  In signing above, we hereby certify that we 
 are the individuals who created this HC(S)12 source file and that we have
 not copied the work of any other student (past or present) while completing 
 it. We understand that if we fail to honor this agreement, we will receive 
 a grade of ZERO and be subject to possible disciplinary action.

***********************************************************************

 The objective of this Mini-Project is to .... < ? >


***********************************************************************

 List of project-specific success criteria (functionality that will be
 demonstrated):

 1.

 2.

 3.

 4.

 5.

***********************************************************************

  Date code started: < ? >

  Update history (add an entry every time a significant change is made):

  Date: < ? >  Name: < ? >   Update: < ? >

  Date: < ? >  Name: < ? >   Update: < ? >

  Date: < ? >  Name: < ? >   Update: < ? >


***********************************************************************
*/

#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include <mc9s12c32.h>

/* All functions after main should be initialized here */
char inchar(void);
void outchar(char x);
void shiftout(char x);
void lcdwait(void);
void send_byte(char x);
void send_i(char x);
void chgline(char x);
void print_c(char x);
void pmsglcd(char[]);
void rdisp();
unsigned char bci();



/* Variable declarations */
int y = 0;
int dataRecvd  = 0;
int overallRating = 0;
int foodRating = 0;
int srvcRating = 0;
int tin       = 0;        // SCI transmit display buffer IN pointer
int tout      = 0;
int test = 0;
int testSCI = 0;
int x = 0;
unsigned char temp = 0;
char recv = 0;

#define TSIZE 2        // transmit buffer size (80 characters)
char tbuf[TSIZE];        // SCI transmit display buffer

                                                                                                         

/* Special ASCII characters */
#define CR 0x0D                // ASCII return 
#define LF 0x0A                // ASCII new line 

/* LCD COMMUNICATION BIT MASKS (note - different than previous labs) */
#define RS 0x10                // RS pin mask (PTT[4])
#define RW 0x20                // R/W pin mask (PTT[5])
#define LCDCLK 0x40        // LCD EN/CLK pin mask (PTT[6])

/* LCD INSTRUCTION CHARACTERS */
#define LCDON 0x0F        // LCD initialization command
#define LCDCLR 0x01        // LCD clear display command
#define TWOLINE 0x38        // LCD 2-line enable command
#define CURMOV 0xFE        // LCD cursor move instruction
#define LINE1 = 0x80        // LCD line 1 cursor position
#define LINE2 = 0xC0        // LCD line 2 cursor position

                                    
/*                                    
***********************************************************************
 Initializations
***********************************************************************
*/

void  initializations(void) {

/* Set the PLL speed (bus clock = 24 MHz) */
  CLKSEL = CLKSEL & 0x80; //; disengage PLL from system
  PLLCTL = PLLCTL | 0x40; //; turn on PLL
  SYNR = 0x02;            //; set PLL multiplier
  REFDV = 0;              //; set PLL divider
  while (!(CRGFLG & 0x08)){  }
  CLKSEL = CLKSEL | 0x80; //; engage PLL

/* Disable watchdog timer (COPCTL register) */
  COPCTL = 0x40   ; //COP off; RTI and COP stopped in BDM-mode

/* Initialize asynchronous serial port (SCI) for 4800 baud, interrupts off initially */
  SCIBDH =  0x01; //set baud rate to 9600
  SCIBDL =  0x38; //24,000,000 / 16 / 312 = 4800 (approx)  
  SCICR1 =  0x00; //$9C = 156
  SCICR2 =  0x0C; //initialize SCI for program-driven operation
  //SCICR2 =  0x24;
  DDRB   =  0x10; //set PB4 for output mode
  PORTB  =  0x10; //assert DTR pin on COM port

/* Initialize peripherals */

/* Initialize SPI for baud rate of 6 Mbs */
 SPIBR = 0x01;
 SPICR1 = 0x50;
 SPICR2 = 0x00; 

/* Initialize digital I/O port pins */
 DDRT = DDRT | 0xFF;
 ATDDIEN = 0xFF;
 DDRM = 0xFF;

/* Initialize the LCD
     - pull LCDCLK high (idle)
     - pull R/W' low (write state)
     - turn on LCD (LCDON instruction)
     - enable two-line mode (TWOLINE instruction)
     - clear LCD (LCDCLR instruction)
     - wait for 2ms so that the LCD can wake up     
*/
PTT_PTT4 = 1;
PTT_PTT3 = 0;
send_i(LCDON);
send_i(TWOLINE);
send_i(LCDCLR);
lcdwait();

/* Initilaize ATD 
      - will use port AN0
      - Conversion length = 1
*/
ATDCTL2 = 0x80;
ATDCTL3 = 0x08;
ATDCTL4 = 0x85;

/* Initialize TIM Ch 7 (TC7) for periodic interrupts every 10.000 ms
     - enable timer subsystem
     - set channel 7 for output compare
     - set appropriate pre-scale factor and enable counter reset after OC7
     - set up channel 7 to generate 1 ms interrupt rate
     - initially disable TIM Ch 7 interrupts      
*/
  
  TSCR1 = 0x80;
  TSCR2 = 0x0C;
  TIOS = 0x80;
  TC7 = 15000;
  TIE = 0x80;
  
  /* initilize PWM ch0 */

  MODRR = 0x01;
  PWME = 0x01;
  PWMPOL = 0x01;  
  PWMCTL = 0x00;  
  PWMCAE = 0x00;  
  PWMPER0 = 0xFF; 
  PWMDTY0 = 0x00;
  PWMCLK = 0x00;  
  PWMPRCLK = 0x01;
  PWMSCLA = 0x00;
  PWMSCLB = 0x00;


            
/* Initialize interrupts */
              
              
}

                                                                                      
/*                                                                                      
***********************************************************************
Main
***********************************************************************
*/
void main(void) {
  
  DisableInterrupts
        initializations();                                                                              
        EnableInterrupts;
        send_i(LCDCLR);
        chgline(0x80);
        pmsglcd("Hell's Ratings!");
 //       rdisp(); 
          temp = inchar();

 for(;;) {
   /* temp = bci();
  if (temp != 0) {
    testSCI = 10;
  }
    

  //SCICR2_RIE = 1;
  if ( (tin+1) % TSIZE == tout){
    
  temp = bci();
  if (temp != 0) {
    testSCI = 10;
  }
  } */
  
  
  

  
  

  
   } /* loop forever */
   
}   /* do not leave main */




/*
***********************************************************************                       
 RTI interrupt service routine: RTI_ISR
************************************************************************
*/

interrupt 7 void RTI_ISR(void)
{
          // clear RTI interrupt flagt 
          CRGFLG = CRGFLG | 0x80; 
 

}

/*
***********************************************************************                       
  TIM interrupt service routine                          
***********************************************************************
*/

interrupt 15 void TIM_ISR(void)
{
          // clear TIM CH 7 interrupt flag 
         TFLG1 = TFLG1 | 0x80;
         ATDCTL5 = 0x10;           // Start ATD !
  while(!ATDSTAT0_SCF){}
  
  y = 10 * ATDDR0H / 51 + 204;
  PWMDTY0 = ATDDR0H;
  

 

}

/*
***********************************************************************                       
  SCI interrupt service routine                                                   
***********************************************************************
*/

interrupt 20 void SCI_ISR(void)
{
    /*if ( SCISR1_RDRF == 1) {
    //if ( SCISR1_TDRE == 1) {
    
    testSCI = 40;
    if ( (tin+1) % TSIZE != tout) {
        tbuf[tin] = SCIDRL;
        test = tbuf[tin];
        tin = (tin + 1) % TSIZE;
        testSCI = 30;
      
    }
      else {
        SCICR2_RIE = 0;
        testSCI = 31;
      }
    
  } */
  if ( SCISR1 == 0x20) {
  testSCI = SCIDRL;
  }
  
  /*if ( SCISR1_RDRF == 1) {
    recv = SCIDRL;
  
  
  }*/
}

unsigned char bci(){
  
  unsigned char x;
  while (tin == tout) {  //SCICR2_RIE = 1;
}
  x = tbuf[tout];
  tout = (tout + 1) % TSIZE;
  
  //x = recv;

  return x;
}


void rdisp() {

  send_i(LCDCLR);
  pmsglcd("Food;");
  print_c(foodRating + 48);
  pmsglcd("/3");
  
  pmsglcd("Srvc:");
  print_c(srvcRating + 48);
  pmsglcd("/3");
  
  chgline(0xC0);
  pmsglcd("  Overall:");
  print_c(overallRating + 48);
  pmsglcd("/5");

  
}

/*
***********************************************************************
  shiftout: Transmits the character x to external shift 
            register using the SPI.  It should shift MSB first.  
             
            MISO = PM[4]
            SCK  = PM[5]
***********************************************************************
*/
void shiftout(char x)

{
 
  // read the SPTEF bit, continue if bit is 1
  // write data to SPI data register
  // wait for 30 cycles for SPI data to shift out
  int h = 0;
   while(SPISR_SPTEF != 1) {
   }
   
   SPIDR = x;
   for (h = 0;h<30;h++) {
    asm{
      nop;
    }
   }

}

/*
***********************************************************************
  lcdwait: Delay for approx 2 ms
***********************************************************************
*/

void lcdwait()
{
  unsigned long wait;
  for (wait = 0;wait < 10000; wait++) {
  }
}

/*
***********************************************************************
  send_byte: writes character x to the LCD
***********************************************************************
*/

void send_byte(char x)
{
     // shift out character
     // pulse LCD clock line low->high->low
     // wait 2 ms for LCD to process data
     
     shiftout(x);
     PTT_PTT4 = 0;
     lcdwait();
     PTT_PTT4 = 1;
     
} 

/*
***********************************************************************
  send_i: Sends instruction byte x to LCD  
***********************************************************************
*/

void send_i(char x)
{
        // set the register select line low (instruction data)
        // send byte
        PTT_PTT2 = 0;
        send_byte(x);
}

/*
***********************************************************************
  chgline: Move LCD cursor to position x
  NOTE: Cursor positions are encoded in the LINE1/LINE2 variables
***********************************************************************
*/

void chgline(char x)
{
  send_i(CURMOV);
  send_i(x);

}

/*
***********************************************************************
  print_c: Print (single) character x on LCD            
***********************************************************************
*/
 
void print_c(char x)
{
  PTT_PTT2 = 1;
  send_byte(x);

}

/*
***********************************************************************
  pmsglcd: print character string str[] on LCD
***********************************************************************
*/

void pmsglcd(char str[])
{
  int j = 0;
  while(str[j]) {
    print_c(str[j]);
    j++;
  }

}

/*
***********************************************************************
 Character I/O Library Routines for 9S12C32 
***********************************************************************
 Name:         inchar
 Description:  inputs ASCII character from SCI serial port and returns it
 Example:      char ch1 = inchar();
***********************************************************************
*/

char inchar(void) {
  /* receives character from the terminal channel */
        while (!(SCISR1 & 0x20)); /* wait for input */
    return SCIDRL;
}

/*
***********************************************************************
 Name:         outchar    (use only for DEBUGGING purposes)
 Description:  outputs ASCII character x to SCI serial port
 Example:      outchar('x');
***********************************************************************
*/

void outchar(char x) {
  /* sends a character to the terminal channel */
    while (!(SCISR1 & 0x80));  /* wait for output buffer empty */
    SCIDRL = x;
}