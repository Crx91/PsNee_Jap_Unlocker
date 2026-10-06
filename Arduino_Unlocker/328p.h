#pragma once
// Main Atmega328p pin definitions

#pragma once

#ifdef Speed_16MHZ
  #define F_CPU 16000000L
#endif

#ifdef Speed_8MHZ
  #define F_CPU 8000000L
#endif

// Define the main pins as inputs
#define DATA_INPUT DDRB &= ~(1 << DDB0)
#define WFCK_INPUT DDRB &= ~(1 << DDB1)
#define SQCK_INPUT DDRD &= ~(1 << DDD6)
#define SUBQ_INPUT DDRD &= ~(1 << DDD7)

// Configure lines as outputs (for injection/override)
#define DATA_OUTPUT DDRB |= (1 << DDB0)
#define WFCK_OUTPUT DDRB |= (1 << DDB1)

// Bus line state control (Set High / Low)
#define DATA_H PORTB |= (1 << PB0)
#define DATA_L PORTB &= ~(1 << PB0)
#define WFCK_L PORTB &= ~(1 << PB1)

// Direct Register Reading (High-speed polling)
#define SQCK_READ (!!(PIND & (1 << PIND6))) // Check if the value of PIND6 is high (1)
#define SUBQ_READ (!!(PIND & (1 << PIND7))) // Check if the value of PIND7 is high (1)
#define WFCK_READ (!!(PINB & (1 << PINB1))) // Check if the value of PINB1 is high (1)

// Bios_D2 aka digital pin D4
#define Bios_D2_IN DDRD &= ~(1 << DDD4)
#define Bios_D2_OUT DDRD |= (1 << DDD4)
#define Bios_D2_H PORTD |= (1 << PD4)
#define Bios_D2_L PORTD &= ~(1 << PD4)

// Bios_CE aka digital pin D3
#define Bios_CE_IN DDRD &= ~(1 << DDD3)
#define Bios_CE_READ (!!(PIND & (1 << PIND3)))  // Check if the value of PIND3 is high
#define Bios_CE_H PORTD |= (1 << PD3)
#define Bios_CE_L PORTD &= ~(1 << PD3)

// Speed_ISR0 on digital pin D2
#define Speed_ISR_ENABLE EIMSK = (1 << INT0)
#define Speed_ISR_DISABLE EIMSK &= ~(1 << INT0)
#define Speed_ISR_RISING EICRA = (1 << ISC01) | (1 << ISC00)

// Switch (digital pin D5)
#define SWITCH_INPUT DDRD &= ~(1 << DDD5)  // Configure PIND5 as input for switch
#define SWITCH_SET PORTD |= (1 << PD5)    // Set PIND5 high (enable pull-up)
#define SWITCH_READ (!!(PIND & (1 << PIND5)))   // Read the state of PIND5 (switch input)

// Injection Status LED
#define LED_OUTPUT DDRB |= (1 << 5)  // Configure PINB5 as output (for LED)
#define LED_ON PORTB |= (1 << 5)     // Set PINB5 high (turn on LED)
#define LED_OFF PORTB &= ~(1 << 5)   // Set PINB5 low (turn off LED)

/*
// Bios Patching Status LED
#define BLED_OUTPUT DDRB |= (1 << 5)  // Configure PINB5 as output (for LED)
#define BLED_ON PORTB |= (1 << 5)     // Set PINB5 high (turn on LED)
#define BLED_OFF PORTB &= ~(1 << 5)   // Set PINB5 low (turn off LED)
*/

// Timer
#define TIMER_INTERRUPT_ENABLE TIMSK0 |= (1 << OCIE0A)
#define TIMER_INTERRUPT_DISABLE TIMSK0 &= ~(1 << OCIE0A)
#define TIMER_TCNT_CLEAR TCNT0 = 0x00
#define TIMER_TIFR_CLEAR TIFR0 |= (1 << OCF0A)

void Timer0_8_init()  // Timer @8Mhz
  {
  TCNT0 = 0x00;
  OCR0A = 79;
  TCCR0A |= (1 << WGM01);
  TCCR0B |= (1 << CS00);
  }


void Timer0_16_init() // Timer @16Mhz
  {
  TCNT0 = 0x00;
  OCR0A = 159;
  TCCR0A |= (1 << WGM01);
  TCCR0B |= (1 << CS00);
  }