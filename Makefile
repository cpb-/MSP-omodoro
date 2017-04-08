#######################################################################
## \file Makefile for the MSP-omodoro project.
##
## \author Christophe Blaess 2017 <christophe.blaess@logilin.fr>
##
## \license GPL.
##

# Project name
PROJECT = msp-omodoro

# Cross-compiler (standard on Debian/Ubuntu).
GCC = msp430-gcc

# Microcontroler version.
MCU = msp430g2553

# Programmer (T.I. Launchpad on USB).
PROGRAMER = rf2500

# Flasher program (standard on Debian/Ubuntu).
FLASHER = mspdebug

CFLAGS = -Wall

.phony: all

all: $(PROJECT).elf

$(PROJECT).elf: $(PROJECT).c
	$(GCC) $(CFLAGS) -mmcu=$(MCU) $(PROJECT).c -o $(PROJECT).elf

.phony: clean

clean:
	rm -f *.elf *~ *.o

.phony: flash

flash: $(PROJECT).elf
	$(FLASHER) $(PROGRAMER) erase
	$(FLASHER) $(PROGRAMER) "prog $(PROJECT).elf"

