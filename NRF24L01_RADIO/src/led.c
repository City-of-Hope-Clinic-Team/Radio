// 02_read_button.c
// Blink LED with struct
// Josh Brake
// jbrake@hmc.edu
// 9/22/21

// Include integer types with specified widths like uint32_t
#include <stdint.h>

// Pointer to AHB2 enable register for turning on GPIOB
volatile unsigned long * RCC_AHB2ENR  = (unsigned long *) (0x4002104CUL);

// Base addresses for GPIO ports
#define GPIOB_BASE (0x48000400UL)

// GPIO register structs here
typedef struct {
  volatile uint32_t MODER;   // GPIO Offset 0x00 GPIO port mode register
  volatile uint32_t OTYPER;  // GPIO Offset 0x04
  volatile uint32_t OSPEEDR; // GPIO Offset 0x08
  volatile uint32_t PURPDR;  // GPIO Offset 0x0C
  volatile uint32_t IDR;     // GPIO Offset 0x10
  volatile uint32_t ODR;     // GPIO Offset 0x14
  volatile uint32_t BSRR;    // GPIO Offset 0x18
  volatile uint32_t LCKR;    // GPIO Offset 0x1C
  volatile uint32_t AFRL;    // GPIO Offset 0x20
  volatile uint32_t AFRH;    // GPIO Offset 0x24
} GPIO;

#include "led.h"
#include "stm32l432xx.h"
#define LED_PIN           3

void led_init()
	{
	 *RCC_AHB2ENR |= (1 << 1);;
   GPIOB->MODER |= (1 << 2*LED_PIN);
   GPIOB->MODER &= ~(1 << (2*LED_PIN+1));
	}

void led_on()
	{
	GPIOB->BSRR=GPIO_BSRR_BS4;
	}

void led_off()
	{
	GPIOB->BSRR=GPIO_BSRR_BR4;
	}

void toggle_led()
	{
	GPIOB->ODR^=GPIO_ODR_OD4;
	}
