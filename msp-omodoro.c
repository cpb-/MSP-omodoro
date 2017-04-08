// ---------------------------------------------------------------------
/// \file msp-omodoro.c - A simple Pomodoro implementation for MSP430.
//
/// \author Christophe Blaess 2017 - <christophe.blaess@logilin.fr>
//
/// \licence GPL.
//

// ----- System headers ------------------------------------------------

	#include <msp430g2553.h>

// ----- Project headers -----------------------------------------------

// ----- Private macros ------------------------------------------------

	// Nodes of the Finite-state Machine:
	#define STATE_OFF            0
	#define STATE_TIME_TO_WORK   1
	#define STATE_WORK           2
	#define STATE_PREPARE_BREAK  3
	#define STATE_TIME_TO_BREAK  4
	#define STATE_SHORT_BREAK    5
	#define STATE_LONG_BREAK     6
	#define STATE_PREPARE_WORK   7


	// Delays for transistions (in seconds):
	#define DELAY_BEFORE_OFF  (5*60)
	#define WORK_TIME         (25*60)
	#define PREPARE_TIME      (2*60)
	#define SHORT_BREAK_TIME  (5*60)
	#define LONG_BREAK_TIME   (15*60)

	// Port 1 GPIOs:
	#define RED_LED     0x01
	#define GREEN_LED   0x40
	#define BUTTON      0x08


// ----- Private variables ---------------------------------------------


	// The state of the machine:
	static int current_state = STATE_OFF;

	// Occurence of a button press:
	static int button_press = 0;



// ----- Public functions ----------------------------------------------

/// \brief This function does not loop, it initializes the I/O and
///        interrupts then put the MSP 430 into sleep mode. All the
///        work is done by interrupt handlers.
///
/// \returns (never) 0.
///
int main(void)
{
	// Stop the watchdog:
	WDTCTL = WDTPW | WDTHOLD;

	// Configure the DCO to 1MHz:
	DCOCTL  = CALDCO_1MHZ;
	BCSCTL1 = CALBC1_1MHZ;

	// Configure the timer to trigger an IRQ at 10Hz.
	TA0CTL |= TASSEL_2+ID_3+MC_1+TACLR;  // SMCLK divided by 8.
	TACCR0  = 12499;                     // 1000000 / 8 / 10 = 12500.
	TACCTL0 =  CCIE;                     // Enable interrupt for timer.

	// Configure the leds:
	P1DIR |=   GREEN_LED | RED_LED;
	P1OUT &= ~(GREEN_LED | RED_LED);

	// Configure the button:
	P1IES |=  BUTTON;   // High to low transition triggers an IRQ.
	P1REN |=  BUTTON;   // Pull resistor enabled.
	P1OUT |=  BUTTON;   // Pull-up mode.
	P1IFG &= ~BUTTON;  // Avoid immediate IRQ.
	P1IE  |=  BUTTON;   // Enable interrupts for port 1.3.

	// Enable the handlers.
	__enable_interrupt();

	// And now... sleep!
	__bis_SR_register(LPM1_bits + GIE);

    return 0;
}



/// \brief Port 1 interrupt handler (triggered by a button press).
///        Note the occurrence in the global variable button_press.
///
#pragma vector = PORT1_VECTOR
__interrupt void P1_ISR(void)
{
	button_press = 1;
	P1IFG &= ~BUTTON;
}



/// \brief Timer interrupt handler (triggered at 10Hz).
///        Implementation of the Finite State Machine.
///
#pragma vector = TIMER0_A0_VECTOR
__interrupt void TIMER_A(void)
{
	static int ticks   = 0;  // 1/10th seconds.
	static int seconds = 0;

	// Update the seconds counter.
	ticks ++;
	if (ticks == 10) {
		ticks = 0;
		seconds++;
	}


	// Handle the FSM transitions.
	switch (current_state) {
		case STATE_OFF:
			// Start-up (or Reset): inconditionally wait to work.
			current_state = STATE_TIME_TO_WORK;
			break;
		case STATE_TIME_TO_WORK:
			if (button_press) {
				button_press = 0;
				current_state = STATE_WORK;
			}
			if (seconds > DELAY_BEFORE_OFF) {
				seconds = 0;
				current_state = STATE_OFF;
			}
			break;
		case STATE_WORK:
			if (button_press) {
				// Nothing. Keep on working!
				button_press = 0;
			}
			if (seconds > (WORK_TIME-PREPARE_TIME)) {
				seconds = 0;
				current_state = STATE_PREPARE_BREAK;
			}
			break;
		case STATE_PREPARE_BREAK:
			if (button_press) {
				button_press = 0;
				current_state = STATE_SHORT_BREAK;
			}
			if (seconds > PREPARE_TIME) {
				seconds = 0;
				current_state = STATE_TIME_TO_BREAK;
			}
			break;
		case STATE_TIME_TO_BREAK:
			if (button_press) {
				button_press = 0;
				current_state = STATE_SHORT_BREAK;
			}
			if (seconds > DELAY_BEFORE_OFF) {
				seconds = 0;
				current_state = STATE_OFF;
			}
			break;
		case STATE_SHORT_BREAK:
			if (button_press) {
				button_press = 0;
				current_state = STATE_LONG_BREAK;
			}
			if (seconds > (SHORT_BREAK_TIME - PREPARE_TIME)) {
				seconds = 0;
				current_state = STATE_PREPARE_WORK;
			}
			break;
		case STATE_LONG_BREAK:
			if (button_press) {
				// Ignore button press during break time.
				button_press = 0;
			}
			if (seconds > (LONG_BREAK_TIME - PREPARE_TIME)) {
				seconds = 0;
				current_state = STATE_PREPARE_WORK;
			}
			break;
		case STATE_PREPARE_WORK:
			if (button_press) {
				button_press = 0;
				current_state = STATE_WORK;
			}
			if (seconds > PREPARE_TIME) {
				seconds = 0;
				current_state = STATE_TIME_TO_WORK;
			}
			break;
	}

	// Now handle the actions for current state.
	switch (current_state) {
		case STATE_OFF:
			// Halt and wait for reset.
			P1OUT = 0;
			__bis_SR_register(LPM4_bits + GIE);
			break;
		case STATE_TIME_TO_WORK:
			// Flashing red led.
			P1OUT &= ~GREEN_LED;
			if ((ticks % 2) == 0)
				P1OUT &= ~RED_LED;
			else
				P1OUT |= RED_LED;
			break;
		case STATE_WORK:
			// Red led on.
			P1OUT &= ~GREEN_LED;
			P1OUT |= RED_LED;
			break;
		case STATE_PREPARE_BREAK:
			// Red led off every 4 seconds.
			if ((seconds % 4) == 0)
				P1OUT &= ~RED_LED;
			else
				P1OUT |=  RED_LED;
			break;
		case STATE_TIME_TO_BREAK:
			// Flashing green led.
			P1OUT &= ~RED_LED;
			if ((ticks % 2) == 0)
				P1OUT &= ~GREEN_LED;
			else
				P1OUT |= GREEN_LED;
			break;
		case STATE_SHORT_BREAK:
		case STATE_LONG_BREAK:
			// Green led on.
			P1OUT |= GREEN_LED;
			P1OUT &= ~RED_LED;
			break;
		case STATE_PREPARE_WORK:
			// Green led off every 4 seconds.
			if ((seconds % 4) == 0)
				P1OUT &= ~GREEN_LED;
			else
				P1OUT |=  GREEN_LED;
			break;
	}
}

