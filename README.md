# MSP430 Pomodoro

This is a simple [Pomodoro](https://en.wikipedia.org/wiki/Pomodoro_Technique) implementation for [T.I. MSP430 Launchpad](http://www.ti.com/tool/MSP-EXP430G2).

Detailed description in [this french article](https://www.blaess.fr/christophe/2017/04/10/msp-omodoro/).

Six steps:
- Flahing red led: It' time to work! Press the push-button when ready.
- Red led on: Working phase (25').
- Red led intermittent: Break time is coming (last 2').
- Flashing green led: It's time to take a break. Press the button once for a short break (5') and twice for a long break (15').
- Green led on: Pause.
- Intermittent green led: work phase is coming (last 2').

Author: Christophe Blaess 2017.
