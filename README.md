# PsNee and JAP Bios Unlocker for Arduino with Atmega328p MCU
The ultimate mod based on ![PsNee](https://github.com/kalymos/psnee) and my latest ![Jap_Bios_Unlocker](https://github.com/Crx91/Ps1_Jap_Bios_Unlocker).                   

This is a port of my PsNee_Aio for Arduino boards with Atmega328p MCU.                                                          
Fully compatible and stealth with all PS1 JAP models and unlocks all JAP BIOSes too!                                                                                         

This mod uses a new method to patch the ps1 JAP bios protection, allowing playing PAL and USA region games.
My method uses a different approach instead of the one made by postal v8 PsNee (and kalymos upgrades).

Postal fix aims to patch a few bits at "ns" precision time at console boot, but have different disadvantages:
- Each bios needs a different time patching and approach;
- On some BIOSes, the fix corrupt the bios "CD player" or "MC manager", causing console crash if you try to enter in those menus;
- Wire lengths are very strict (due to the timing involved), a few mm longer wire (propagation delay) and the fix will fail;
- Combinations of different Atmega328p and crystals can cause ISR jitter and speed patching issues. BTW kudos to kalymnos who is working hard to make the code more stable, you're great!
- You need to upload the code with an ICSP programmer and set the correct fuses based on the code settings.

My method instead, patches on the fly the bios line only when the game is recognized, so we have:
- Universal patching method for all BIOSes;
- Wire lengths aren't an issue any more;
- No more combinations of different Atmega328p and crystals problems;
- The *ino sketch can be uploaded to the Arduino without any need for an ICSP programmer and fuses settings (my code works even with bootloader startup delay!)

I'm just a hobbyist programmer, so please forgive any coding mistakes, syntax errors, or other issues. Code is written using Arduino IDE and port manipulation for better speed!
The main project based on the ch32v003 can be found ![HERE](https://github.com/Crx91/PsNee_Aio)

## Supported consoles:
- ALL Japanese consoles with JAP BIOS! (SCPH-1000,3000,5000,7000,7500,9000 or SCPH-100)

## Supported MCU:
- All the arduino equipped with Atmega328p MCUs (uno, nano, pro-mini, etc...)

## Prerequisites:
- Only Arduino IDE
- Any Arduino board with Atmega328p MCU

## HowTo:
-	Download this repository.
-	Follow the [Wiki](https://github.com/Crx91/PsNee_Jap_Unlocker/wiki) instructions.

## Thanks to:
ramapcsx2, kalymos, SpenceKonde, oldcrow, mayumi, arduino community, ch32fun community, Infrid and lots of people that can't remember now.

