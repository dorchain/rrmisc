/* ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <info@gerhard-bertelsmann.de> wrote this file. As long as you retain this
 * notice you can do whatever you want with this stuff. If we meet some day,
 * and you think this stuff is worth it, you can buy me a beer in return
 * Gerhard Bertelsmann
 * ----------------------------------------------------------------------------
 */


/* S88 Test Connections
   RA2 Data Output
   RC0 RESET -> ISR
   RC1 LOAD  -> ISR
   RC2 CLOCK -> ISR
 */

#include "main.h"

static __code uint16_t __at (_CONFIG1) configword1= _FOSC_INTOSC & _WDTE_OFF & _PLLEN_OFF & _MCLRE_ON;
static __code uint16_t __at (_CONFIG2) configword2= _LVP_ON & _CLKOUTEN_OFF;

struct serial_buffer_t tx_fifo, rx_fifo;
volatile uint8_t counter=0;

#define BUFFER_SIZE_BANK	64

#if 1
__data uint8_t __at(0x120) s88_data1[];
__data uint8_t __at(0x1A0) s88_data2[];
__data uint8_t __at(0x220) s88_data3[];
__data uint8_t __at(0x2A0) s88_data4[];
#else
volatile uint8_t s88_data1[BUFFER_SIZE_BANK];
volatile uint8_t s88_data2[BUFFER_SIZE_BANK];
volatile uint8_t s88_data3[BUFFER_SIZE_BANK];
volatile uint8_t s88_data4[BUFFER_SIZE_BANK];
#endif

void isr (void) __interrupt (1){
  if (IOCIF) {
    IOCCF = 0;
  }
  // S88-CLOCK RC2
  if ( IOCCF2 ) {
    IOCCF2 = 0;
    if ( RA2 )
      LATA2 = 0;
    else
      LATA2 = 1;
  }
}

void pps_init(void) {
  PPSLOCK = 0x55;
  PPSLOCK = 0xaa;
  PPSLOCK = 0;		// unlock PPS
  // set USART : RX on RA0 , TX on RA1 / 40001729B.pdf page 141
  RXPPS  = 0b00000;	// input  EUSART RX -> RA0
  RA1PPS = 0b10100;	// RA1 output TX/CK

  PPSLOCK = 0x55;
  PPSLOCK = 0xaa;
  PPSLOCK = 1;		// lock PPS
}

void system_init() {
  // switch off analog
  OSCCON = 0b11110000; // Configure oscillator
           //1------- use PLL to get 4x8 Mhz (system clock)
           //-1110--- 8 MHz internal oscillator (instruction clock)
           //------00 oscillator selected with INTOSC
  ANSELA  = 0;
  ANSELC  = 0;
  ADCON0  = 0;
  ADCON1  = 0;
  ADCON2  = 0;
  CM1CON0 = 0;
  CM1CON1 = 0;
  CM2CON0 = 0;
  CM2CON1 = 0;
  TRISA5 = 0;
  TRISA4 = 0;
  // TRISC5 = 0;  // CCP1
  // setup interrupt events
  //clear all relevant interrupt flags
  PIR1 = 0;
  INTCON = 3;
}

void uart_init (void) {
  TX9  = 1;		// 8-bit transmission
  TX9D = 1;		//  one extra stop bit
  TXEN = 1;		// transmit enabled
  SYNC = 0;		// asynchronous mode
  BRGH = 1;		// high speed
  SPEN = 1;		// enable serial port (configures RX/DT and TX/CK pins as serial port pins)
  RX9  = 0;		// 8-bit reception
  CREN = 1;		// enable receiver
  BRG16 = USE_BRG16;	// 8-bit baud rate generator

  SPBRG = SBRG_VAL;	// calculated by defines

  TRISA0 = 1;		// make the TX pin a digital output
  TRISA1 = 0;		// make the RX pin a digital input

  // RCIF = 0;
  PIR1 = 0;
}

void pio_init(void) {
  TRISA2 = 0;
  TRISC0 = 1;
  TRISC1 = 1;
  TRISC2 = 1;

  IOCAF = 0;
  IOCCF = 0;
  // enbale "Interrupt On Change" for S88-Clock P&N
  IOCCN2 = 1;
  IOCCP2 = 1;
  IOCIE = 1;
}

void timer0_init(void) {
  TMR0CS = 0;	// FOSC / 4
  PSA = 0;	// use prescaler
  OPTION_REG = 0b00000010;	// prescaler 1:8
  //TMR0 = TIMER0_VAL;
  TMR0 = 240;
  T0IE = 1;
}

void data_init(void) {
  uint8_t i;
  for (i=BUFFER_SIZE_BANK; i!=0; i--)
    s88_data1[i]=0;
    s88_data2[i]=0;
    s88_data3[i]=0;
    s88_data4[i]=0;
}

void main() {
  unsigned short counter=0;

  pps_init();
  system_init();
  uart_init();
  data_init();
  pio_init();

  /* empty circular buffers */
  tx_fifo.head=0;
  tx_fifo.tail=0;
  rx_fifo.head=0;
  rx_fifo.tail=0;

  GIE = 1;
  while(1) {
    if ( counter == 0 )
      putchar_wait(0x55);
    counter++;
  }
}
