/*  -- Reciever Side --
************************************************************************
 ECE 362 - Mini-Project C Source File - Fall 2015
***********************************************************************
                                                                                                                          
  Team ID: < Team 20 >

 Project Name: < Rate and Go >

 Team Members:

   - Team/Doc Leader: < Dom Celiano >      Signature: ______________________
   
   - Software Leader: < Iaman Alkhalaf>      Signature: ______________________

   - Interface Leader: < Trevor Bonesteel >     Signature: ______________________

   - Peripheral Leader: < John Mahony >    Signature: ______________________

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
int wait = 0;
void convert(unsigned char x,unsigned char y,unsigned char z);



/* Variable declarations */
int y = 0;
int dataRecvd  = 0;
unsigned char foodRating = 0;
unsigned char srvcRating = 0;
unsigned char starRating = 0;

unsigned char foodRating2 = 0;
unsigned char srvcRating2 = 0;
unsigned char starRating2 = 0;

int tin       = 0;        // SCI transmit display buffer IN pointer
int tout      = 0;
int test = 0;
int testSCI = 0;
int x = 0;
unsigned char temp = 0;
char recv = 0;
unsigned char testch;
unsigned char read1;
unsigned char read2;
unsigned char read3;
unsigned char testbci;

#define TSIZE 4        // transmit buffer size (80 characters)
unsigned char tbuf[TSIZE];        // SCI transmit display buffer

                                                                                                         

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
  SCIBDH =  0x07; //set baud rate to 9600
  SCIBDL =  0x50; //24,000,000 / 16 / 312 = 4800 (approx)  
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
  pmsglcd("Rate and Go!");
  SCICR2_RIE = 1;

 for(;;) {
  testbci = bci();
  read1 = bci();
  read2 = bci();
  read3 = bci();
  
  if (testbci == 113 || testbci == 220 || read3 == 139) {  //if we receive a start bit
  foodRating = read1;
  srvcRating = read2;
  starRating = read3;
  dataRecvd = 1;
  PWMDTY0 = y;
  
  } else if (read1 == 113 || read1 == 220 || read3 == 139) {
  foodRating = read2;
  srvcRating = read3;
  starRating = testbci;
  dataRecvd = 1;
  PWMDTY0 = y;
  
  } else if  (read2 == 113 || read2 == 220 || read3 == 139) {
  foodRating = read3;
  srvcRating = testbci;
  starRating = read1;
  dataRecvd = 1;
  PWMDTY0 = y;
  
  } else if (read3 == 113 || read3 == 220 || read3 == 139){
  foodRating = testbci;
  srvcRating = read1;
  starRating = read2;
  dataRecvd = 1;
  PWMDTY0 = y;
 }
 
   if ( foodRating == 231 || foodRating == 174 || foodRating == 157) {   //if food rating == 1
    foodRating = '1';
  } else if ( foodRating == 155 || foodRating == 110 || foodRating == 115) {
    foodRating = '2';
  } else if ( foodRating == 230 || foodRating == 153 || foodRating == 46) {
    foodRating = '3';
  } 
  
  if ( srvcRating == 231 || srvcRating == 174 || srvcRating == 157) {
    srvcRating = '1';
  } else if ( srvcRating == 155 || srvcRating == 110 || srvcRating == 115) {
    srvcRating = '2';
  } else if ( srvcRating == 230 || srvcRating == 153 || srvcRating == 46) {
    srvcRating = '3';
  }
  
  if ( starRating == 231 || starRating == 174 || starRating == 157) {
    starRating = '1';
         rdisp();

  } else if ( starRating == 155 || starRating == 110 || starRating == 115) {
    starRating = '2';
        rdisp();

  } else if ( starRating == 230 || starRating == 153 || starRating == 46) {
    starRating = '3';
       rdisp();

  } else if ( starRating == 185 || starRating == 151 || starRating == 238) {
    starRating = '4';
              rdisp();

  } else if ( starRating == 149 || starRating == 174 || starRating == 229) {
    starRating = '5';
   rdisp();

  }

 //convert(foodRating,srvcRating,starRating);
 
 
 /*if(foodRating2 != foodRating && srvcRating2 != srvcRating && starRating2 != starRating) {
    PWMDTY0 = ATDDR0H;
    for (wait = 0;wait < 4000000; wait++) {}
    wait = 0;
    PWMDTY0 = 0;
 }*/
 
 foodRating2 = foodRating;
 srvcRating2 = srvcRating;
 starRating2 = starRating;
 dataRecvd = 0;
 PWMDTY0 = 0;
 
 
 /*foodRating2 = foodRating;
 srvcRating2 = srvcRating;
 starRating2 = starRating;
 rdisp();*/
    
    
  /*if (bci() == '@') {
    testch = '@';
  }*/
  //testbci = bci();
  /*while (bci() != ':') {}
  //testch = ':' ;
  foodRating = bci();
  srvcRating = bci();
  starRating = bci();
  rdisp();*/
  
  
  /*testch = bci();
  read1 = bci();
  read2 = bci();
  read3 = bci();*/
    
    
  
  
  

  
  

  
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
  //PWMDTY0 = ATDDR0H;
  

 

}

/*
***********************************************************************                       
  SCI interrupt service routine                                                   
***********************************************************************
*/

interrupt 20 void SCI_ISR(void)
{
    if ( SCISR1_RDRF == 1) {
    
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
    
  } 

}

unsigned char bci(){
  
  unsigned char x;
  while (tin == tout) {  SCICR2_RIE = 1;
}
  x = tbuf[tout];
  tout = (tout + 1) % TSIZE;
  
  //x = recv;

  return x;
}


void rdisp() {

  send_i(LCDCLR);
  pmsglcd("Food;");
  print_c(foodRating);
  pmsglcd("/3");
  
  pmsglcd("Srvc:");
  print_c(srvcRating);
  pmsglcd("/3");
  
  chgline(0xC0);
  pmsglcd("  Overall:");
  print_c(starRating);
  pmsglcd("/5");

  
}

void convert(unsigned char x,unsigned char y,unsigned char z) {
  if ( x == 231 || x == 174 || x == 157) {
    x = '1';
  } else if ( x == 155 || x == 110 || x == 115) {
    x = '2';
  } else if ( x == 230 || x == 153 || x == 46) {
    x = '3';
  } else if ( x == 185 || x == 151 || x == 238) {
    x = '4';
  } else if ( x == 149 || x == 174 || x == 229) {
    x = '5';
  }
  
  if ( y == 231 || y == 174 || y == 157) {
    y = '1';
  } else if ( y == 155 || y == 110 || y == 115) {
    y = '2';
  } else if ( y == 230 || y == 153 || y == 46) {
    y = '3';
  } else if ( y == 185 || y == 151 || y == 238) {
    y = '4';
  } else if ( y == 149 || y == 174 || y == 229) {
    y = '5';
  }
  
  if ( z == 231 || z == 174 || z == 157) {
    z = '1';
  } else if ( z == 155 || z == 110 || z == 115) {
    z = '2';
  } else if ( z == 230 || z == 153 || z == 46) {
    z = '3';
  } else if ( z == 185 || z == 151 || z == 238) {
    z = '4';
  } else if ( z == 149 || z == 174 || z == 229) {
    z = '5';
  }
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
     PTT_PTT4 = 0;
     lcdwait();
     
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