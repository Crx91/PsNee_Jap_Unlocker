#pragma once

//---------------------------------------------------------------
// Japanese BIOS patching lib v1.1
//
// - The fix will auto-disable if not triggered before Timeot var
//   after the first region code patch injection
//---------------------------------------------------------------

#define Timeout 10000  // Timeout var.

volatile uint8_t Trigger = 0;
uint8_t Jready = 0;
uint8_t Jfix = 0;

extern volatile uint8_t count_isr;
extern volatile uint32_t microsec;
extern volatile uint16_t millisec;

void Timer_Start(void);
void Timer_Stop(void);

void Jfix_Injection() {
  // Begin Bios Jap patching
  cli();  // Disable all the active ISR now

  LED_ON;  // Patching

  delayMicroseconds(80);

  while (Bios_CE_READ)
    ;  // While Bios_CE is High wait and do nothing

  if (!(Bios_CE_READ))  // If Bios_CE is Low
  {
    // Bios_D2 Output
    Bios_D2_OUT;
    // Bios_D2 Low;
    Bios_D2_L;
  }

  delayMicroseconds(900);

  // Release D2
  Bios_D2_IN;
  Jfix = 0;  // Patch done
  LED_OFF;
}

void JAP_fix() {
  switch (Trigger) {
    case 0:
      // Speed_ISR ENABLE
      Speed_ISR_ENABLE;
      // Start Timer with timeout of 10sec.
      Timer_Start();
      break;
    case 1:
      if (millisec > Timeout) {
        // Too much time, disable Jfix
        Speed_ISR_DISABLE;
        Jfix = 0;
        Timer_Stop();
        } else {
        Jfix_Injection();
      }
      break;
  }
}


// Speed ISR (Interrupt Service Routine)
ISR(INT0_vect) {
  Trigger = 1;
  JAP_fix();
  Speed_ISR_DISABLE;
}