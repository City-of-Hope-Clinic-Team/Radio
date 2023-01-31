/**
    Main: Contains main function for AES SPI communication with FPGA.
    Sends the plaintext and key over SPI to the FPGA and then receives back
    the cyphertext. The cyphertext is then compared against the solution
    listed in Appendix A of the AES FIPS 197 standard.
    @file lab7.c
    @author Josh Brake
    @version 1.0 7/13/2021
*/
#include <stdio.h>
#include <stm32l432xx.h>
#include "STM32L432KC.h"

////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////

#define MCK_FREQ 100000
// Complete define statements for the LED pin and delay duration

int main(void) {
 

  // Enable GPIOA clock
  RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN | RCC_AHB2ENR_GPIOCEN);

  // "clock divide" = master clock frequency / desired baud rate
  // the phase for the SPI clock is 1 and the polarity is 0
  initSPI(1, 0, 0); //clock divide = 1, phase and polarity is 0

  while(1){
    // should send a read 'status' register(register x07) command out of the system
    // 0000 0111 
    char check_status = 0xAA;
    char blank = 0x0F;
    char recievea,recieveb;

    recievea = spiSendReceive(check_status);
    recieveb = spiSendReceive(blank);


  }


}
