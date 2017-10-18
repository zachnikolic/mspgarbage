//Sean Flaherty
//Program is pretty simple. Just sends a 10 uS pulse to output to ultrasonic sensor.
//
//when button is pressed, start timer countup and enable output pin.
//when timer reaches countup, 
#include <msp430g2553.h>

int main(void){
	WDTCTL = WDTPW + WDTHOLD; //stop wdt

	P1DIR = BIT2; //P1.2 for trigger output
	P1SEL |= BIT2; //put P1.2 in auxiliary mode
	P1REN |= BIT3; //Pushbutton input P1.3
	P1OUT |= BIT3;//enable P1.3 pullup resistor
	P1IES |= BIT3; //enable interrupt for P1.3
	P1IFG &= ~BIT3; //clear P1.3 interrupt flag

	return 0;
}

//port 1 ISR
#pragma vector=PORT1_VECTOR
__interrupt void P1_ISR(void){
	//
	P1IFG &= ~BIT3; //clear P1.3 interrupt flag
	TA0CTL = TASSEL_2 + MC_1;
	TA0CCR1 = 10; //we want to count to roughly 10mS before turning off the timer
	TA0CCTL1 = OUTMOD_1 + CCIE; //set timer to set mode, enable capture/compare interrupt
	P1OUT |= BIT2; //turn on P1.2 until timer is done
	P1IFG &= ~BIT3; //clear P1.3 interrupt flag
}

#pragma vector=TIMERA0_VECTOR
__interrupt void TA0_ISR(void){
	//this ISR is intended to happen 10 uS after the timer is activated
	P1OUT &= ~BIT2; //turn off P1.2
	TA0CTL = MC_0; //turn off timer
	TA0CCTL1 &= ~CCIFG; //clear interrupt flag
}
