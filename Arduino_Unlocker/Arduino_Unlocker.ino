#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>
#include <util/delay.h>

#include "328p.h"
#include "Jap_fix.h"

// PsNee Jap_Unlocker
// Porting to Arduino atmega328p MCU based from Ch32v003 Jap_Unlocker by Carmax91 rev_1
// Original PsNee_AIO new JAP Fix code: https://github.com/Crx91/PsNee_Aio
// Base PsNee Injection code: https://github.com/kalymos/PsNee 
//
// Beware to use the PSX 3.5V / 3.3V power, *NOT* 5V! The installation pictures include an example.
//
// Changelog:
// Rev1 - First release! Hello world!
//
// PINOUT:
/*
 * Pinout
 * Arduino   | PSNee     |
 * ---------------------------------------------------
 * VCC       | VCC       |
 * GND       | GND       |
 * RST       | RESET     | 
 * D2        | Speed     | 
 * D3        | Bios_CE   | 
 * D4        | Bios_D2   | 
 * D5        | SWITCH    | Optional for disabling Bios patch
 * D6        | SQCK      |
 * D7        | SUBQ      |
 * D8        | DATA      |
 * D9        | WFCK      |
 * D13       | LED       | 
*/

//_______________________PREDIRECTIVES SETTINGS_________________________________________________________________________
// Speed set. Uncomment and comment the other speed. NEVER use both speeds!:
#define Speed_16MHZ  //Speed 16Mhz. Arduino standard. Recommended speed setting
//#define Speed_8MHZ  //Speed 8Mhz. Arduino micro @3.6v with 8Mhz OSC or Atmega328p with internal OSC fuse set!

// Region set:
#define X_BIT 'i'  // 'e' for PAL, 'a' for USA, 'i' for JAP region

// Those values should be never touched:
// Hysteresis set.
#define HYSTERESIS_MAX 17  //Value of hysteresys, bad drive need higher value max at 19
// Timings
#define delay_between_bits 4000      // 250 bits/s (microseconds)
#define delay_between_injections 90  // 72 in oldcrow. PU-22+ work best with 80 to 100 (milliseconds)
//______________________________________________________________________________________________________________________

volatile uint8_t count_isr = 0;
volatile uint32_t microsec = 0;
volatile uint16_t millisec = 0;

// Setup() detects which (of 2) injection methods this PSX board requires, then stores it in pu22mode.
uint8_t wfckmode;

/*----------------------------------------------------------------------
  Function: ISR on Timer0
-----------------------------------------------------------------------*/
ISR(TIMER0_COMPA_vect) {
  microsec += 10;
  count_isr++;
  if (count_isr == 100) {
    millisec++;
    count_isr = 0;
  }
}

void Timer_Start() {
  TIMER_TCNT_CLEAR;
  TIMER_TIFR_CLEAR;
  TIMER_INTERRUPT_ENABLE;
}

void Timer_Stop() {
  TIMER_INTERRUPT_DISABLE;
  TIMER_TCNT_CLEAR;
  count_isr = 0;
  microsec = 0;
  millisec = 0;
}

/*----------------------------------------------------------------------
  Function: board_detection

  This function distinguishes motherboard generations by detecting
  the nature of the WFCK signal:

  WFCK: __-----------------------  // CONTINUOUS (PU-7 .. PU-20)(GATE)

  WFCK: __-_-_-_-_-_-_-_-_-_-_-_-  // FREQUENCY  (PU-22 or newer)

  Traditionally, the WFCK signal was called GATE. This is because, on early models,
  modchips acted like a gate that would open to pull the signal down
   at the exact moment the region code was being passed (which is still the case today).

  During the initialization and region protection zone reading phases,
  the WFCK clock frequency is approximately 7.3 kHz.
  During normal data reading, the frequency shifts to 14.6 kHz.

-----------------------------------------------------------------------*/
void board_detection() {
  // Default to static signal (PU-7 to PU-20)
  wfckmode = 0;

  /* 
    INITIAL STABILIZATION DELAY (300ms)
    PU-7 to PU-20: Voltage climbs slowly (up to 54ms) then stays HIGH (static).
    PU-22+: Signal only starts oscillating (~7.3kHz) after approximately 297ms.
    Waiting 300ms bypasses all power-up transients and initial noise.
  */
  delay(300);

  // Sampling window to detect the oscillating signal
  uint16_t detectionWindow = 10000;

  while (--detectionWindow) {
    /* 
       On older boards (PU-7/20), the signal is now a solid HIGH.
       If we detect a LOW state, it's a potential oscillation from a newer board.
    */
    if (!WFCK_READ) {  //Wfck == 0
      // Small debounce delay to filter out micro-glitches or remaining noise
      uint8_t debounce = 100;
      while (--debounce)
        ;

      /* 
         VERIFICATION: If the signal is STILL low, it confirms a real 
         clock cycle (WFCK). Older boards will never reach this state 
         once stabilized at HIGH.
      */
      if (!WFCK_READ) {  // Wfck == 0
        wfckmode = 1;    // Target: PU-22 or newer
        return;
      }
    }
  }
}

// borrowed from AttyNee. Bitmagic to get to the SCEX strings stored in flash (because Harvard architecture)
bool readBit(int index, const unsigned char *ByteSet) {
  int byte_index = index >> 3;
  byte bits = pgm_read_byte(&(ByteSet[byte_index]));
  int bit_index = index & 0x7;  // same as (index - byte_index<<3) or (index%8)
  byte mask = 1 << bit_index;
  return (0 != (bits & mask));
}

void inject_SCEX(char region) {
  //SCEE: 1 00110101 00, 1 00111101 00, 1 01011101 00, 1 01011101 00
  //SCEA: 1 00110101 00, 1 00111101 00, 1 01011101 00, 1 01111101 00
  //SCEI: 1 00110101 00, 1 00111101 00, 1 01011101 00, 1 01101101 00
  //const boolean SCEEData[44] = {1,0,0,1,1,0,1,0,1,0,0,1,0,0,1,1,1,1,0,1,0,0,1,0,1,0,1,1,1,0,1,0,0,1,0,1,0,1,1,1,0,1,0,0};
  //const boolean SCEAData[44] = {1,0,0,1,1,0,1,0,1,0,0,1,0,0,1,1,1,1,0,1,0,0,1,0,1,0,1,1,1,0,1,0,0,1,0,1,0,1,1,1,0,1,0,0};
  //const boolean SCEIData[44] = {1,0,0,1,1,0,1,0,1,0,0,1,0,0,1,1,1,1,0,1,0,0,1,0,1,0,1,1,1,0,1,0,0,1,0,1,0,1,1,1,0,1,0,0};
  static const PROGMEM unsigned char SCEEData[] = { 0b01011001, 0b11001001, 0b01001011, 0b01011101, 0b11101010, 0b00000010 };
  static const PROGMEM unsigned char SCEAData[] = { 0b01011001, 0b11001001, 0b01001011, 0b01011101, 0b11111010, 0b00000010 };
  static const PROGMEM unsigned char SCEIData[] = { 0b01011001, 0b11001001, 0b01001011, 0b01011101, 0b11011010, 0b00000010 };

  // pinMode(data, OUTPUT) is used more than it has to be but that's fine.
  for (byte bit_counter = 0; bit_counter < 44; bit_counter++) {
    if (readBit(bit_counter, region == 'e' ? SCEEData : region == 'a' ? SCEAData
                                                                      : SCEIData)
        == 0) {
      DATA_OUTPUT;
      DATA_L;  // data low
      delayMicroseconds(delay_between_bits);
    } else {
      if (wfckmode) {
        /*  
      WFCK Modulation Loop: Syncs to 7.3kHz or 14.6kHz.
      Follows hardware edges to stay bit-perfect with the console.
       */
        DATA_OUTPUT;  //Data Output

        uint8_t count = 30;

        while (count--) {

          while (WFCK_READ)
            ;      //Wait for Falling Edge  //WfckRead
          DATA_L;  //Data Low

          while (!WFCK_READ)
            ;      //Wait for Rising Edge //!WfckRead
          DATA_H;  //Data High
        }
      } else {  // PU-18 or lower mode
        DATA_INPUT;
        delayMicroseconds(delay_between_bits);
      }
    }
  }

  DATA_OUTPUT;
  DATA_L;  // pull data low
  delay(delay_between_injections);
}

//--------------------------------------------------
//     Setup
//--------------------------------------------------

void setup() {
  DATA_INPUT;
  WFCK_INPUT;
  SUBQ_INPUT;
  SQCK_INPUT;
  LED_OUTPUT;
  Bios_D2_IN;
  Bios_CE_IN;
  SWITCH_INPUT;
  SWITCH_SET;
  Speed_ISR_RISING;

  #ifdef Speed_8MHZ
  Timer0_8_init();
  #endif

  #ifdef Speed_16MHZ
  Timer0_16_init();
  #endif
  
  LED_ON;  //Setup start

  if (SWITCH_READ) {
    Jfix = 1;
    Trigger = 0;
    Jready = 0;
  }

  // wait for console power on and stable signals
  while (!SQCK_READ)
    ;
  while (!WFCK_READ)
    ;

  board_detection();  //Start the board detection function

  LED_OFF;  //Setup complete
}

void loop() {
  static byte scbuf[12] = { 0 };  // We will be capturing PSX "SUBQ" packets, there are 12 bytes per valid read.
  static unsigned int timeout_clock_counter = 0;
  static byte bitbuf = 0;  // SUBQ bit storage
  static byte bitpos = 0;
  byte scpos = 0;  // scbuf position

  // start with a small delay, which can be necessary in cases where the MCU loops too quickly
  // and picks up the laster SUBQ trailing end
  delay(1);

  //-------- Japan import bios Fix----------
  if (Jfix && Jready) {
    JAP_fix();
  }
  //----------------------------------------

start:
  // Capture 8 bits for 12 runs > complete SUBQ transmission
  bitpos = 0;
  for (; bitpos < 8; bitpos++) {
    while (SQCK_READ != 0) {
      // wait for clock to go low..
      // a timeout resets the 12 byte stream in case the PSX sends malformatted clock pulses, as happens on bootup
      timeout_clock_counter++;
      if (timeout_clock_counter > 1000) {
        scpos = 0;  // reset SUBQ packet stream
        timeout_clock_counter = 0;
        bitbuf = 0;
        goto start;
      }
    }

    // wait for clock to go high..
    while (SQCK_READ == 0)
      ;

    if (SUBQ_READ)  // while clock pin high
    {
      bitbuf |= 1 << bitpos;  // Set the bit at position bitpos in the bitbuf to 1. Using OR combined with a bit shift
    }

    timeout_clock_counter = 0;  // no problem with this bit
  }

  // one byte done
  scbuf[scpos] = bitbuf;
  scpos++;
  bitbuf = 0;

  // repeat for all 12 bytes
  if (scpos < 12) {
    goto start;
  }

  // check if read head is in wobble area
  // We only want to unlock game discs (0x41) and only if the read head is in the outer TOC area.
  // We want to see a TOC sector repeatedly before injecting (helps with timing and marginal lasers).
  // All this logic is because we don't know if the HC-05 is actually processing a getSCEX() command.
  // Hysteresis is used because older drives exhibit more variation in read head positioning.
  // While the laser lens moves to correct for the error, they can pick up a few TOC sectors.
  static byte hysteresis = 0;
  boolean isDataSector = (((scbuf[0] & 0x40) == 0x40) && (((scbuf[0] & 0x10) == 0) && ((scbuf[0] & 0x80) == 0)));

  if (
    (isDataSector && scbuf[1] == 0x00 && scbuf[6] == 0x00) &&       // [0] = 41 means psx game disk. the other 2 checks are garbage protection
    (scbuf[2] == 0xA0 || scbuf[2] == 0xA1 || scbuf[2] == 0xA2 ||    // if [2] = A0, A1, A2 ..
     (scbuf[2] == 0x01 && (scbuf[3] >= 0x98 || scbuf[3] <= 0x02)))  // .. or = 01 but then [3] is either > 98 or < 02
  ) {
    hysteresis++;
  } else if (hysteresis > 0 && ((scbuf[0] == 0x01 || isDataSector) && (scbuf[1] == 0x00 /*|| scbuf[1] == 0x01*/) && scbuf[6] == 0x00)) {  // This CD has the wobble into CD-DA space. (started at 0x41, then went into 0x01)
    hysteresis++;
  } else if (hysteresis > 0) {
    hysteresis--;  // None of the above. Initial detection was noise. Decrease the counter.
  }

  // hysteresis value "optimized" using very worn but working drive on ATmega328 @ 16Mhz
  // should be fine on other MCUs and speeds, as the PSX dictates SUBQ rate
  if (hysteresis >= HYSTERESIS_MAX) {
    // If the read head is still here after injection, resending should be quick.
    // Hysteresis naturally goes to 0 otherwise (the read head moved).
    hysteresis = 11;

    LED_ON;  //Injecting!

    DATA_OUTPUT;
    DATA_L;  // Pull data low
    if (!wfckmode) {
      WFCK_OUTPUT;
      WFCK_L;
    }

    // HC-05 waits for a bit of silence (pin low) before it begins decoding.
    delay(delay_between_injections);
    // Inject symbols now. 2 x 3 runs seems optimal to cover all boards
    for (byte loop_counter = 0; loop_counter < 2; loop_counter++) {
      inject_SCEX(X_BIT);  // e = SCEE, a = SCEA, i = SCEI
      inject_SCEX(X_BIT);  // injects all 3 regions by default
      inject_SCEX(X_BIT);  // optimize boot time by sending only your console region letter (all 3 times per loop)
    }

    if (!wfckmode) {
      WFCK_INPUT;  // high-z the line, we're done
    }
    DATA_INPUT;  // high-z the line, we're done

    Jready = 1;

    LED_OFF;  // injection done!
  }
  // keep catching SUBQ packets forever
}
