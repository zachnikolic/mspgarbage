//Sean Flaherty
//Ultrasonic Sensor module
//On a signal from P1.3 (push button or external), sends a ~10uS pulse to P1.1 (trigger) on the ultrasonic sensor
//The MCU then waits for a rising edge on the echo input (P1.4, P1.2) and sets a timer capture/compare block to capture on falling edge.
//The falling edge on P1.2 will cause the capture interrupt.
//Distance of an object from the sensor can be calculated using the pulse width recorded by the capture.

#include <msp430g2553.h>

int main(void){
	WDTCTL = WDTPW + WDTHOLD; //stop wdt

	P1DIR = BIT1 + BIT6; //P1.1 for TRIG, 1.6 for LED2
	P1REN |= BIT3 + BIT4; //P1.3 for BTN, P1.4 for ECHO
	P1OUT = BIT3;//enable P1.3 pullup resistor
	P1IE |= BIT3; //enable interrupt for P1.3, P1.4
  	P1IES |= BIT3; //interrupt on falling edge
	P1IES &= ~BIT4; //make sure this interrupts on rising edge
	P1IFG &= ~(BIT2 + BIT3 + BIT4); //clear P1.2, P1.3, P1.4 interrupt flag
	P1SEL = BIT2;
	P1SEL2 &= ~BIT2;
	
	//Note: Enabling P1SEL for P1.2 disables interrupts on that pin.
	TA0CTL = MC_0; //disable timer
	TA0CTL &= ~TAIFG;
	TA0CCTL1 &= ~CCIFG;
	_BIS_SR(GIE);
	volatile unsigned int i;
  	for(;;);

	return 0;
 
}
void overflow(void){
	P1OUT &= ~BIT1; //turn off P1.2
	//P1OUT ^= BIT6;
	TA0CTL = MC_0; //turn off timer
}

void record(void){
	while(TA0CCTL1 & SCCI){
		echo = TA0CCR1;
	}
	if(TA0CCTL1 & COV){
		P1OUT |= BIT6;
	}
	if(echo > 1){
		P1OUT |= BIT6;
	}
	TA0CTL = MC_0; //turn off timer

}
//port 1 ISR
#pragma vector=PORT1_VECTOR
__interrupt void P1_ISR(void){
	if(P1IFG & BIT3){ //interrupt from input to set off trigger pulse
		P1OUT |= BIT1; //turn on P1.2 until timer is done
		TA0CTL = TASSEL_2 + MC_1 + TAIE;
  		TA0CCR0 = 10;
  		TA0CCTL0 = CCIE;
		P1IFG &= ~BIT3;
		P1IE &= ~BIT3;
		P1IE |= BIT4;
	}
	else if(P1IFG & BIT4){ //interrupt from echo input
   		TA0CTL = TASSEL_2 + TACLR + MC_2; //continous mode to prevent overflow from TA0CCR0
		TA0CCR1 = 0x0FFFF;
		TA0CCR0 = 0x0FFFF; //again, to prevent overflow
		TA0CCTL1 = OUTMOD_0 + CM_2 + CCIS_0 + SCCI + CAP + CCIE; //capture from CCI1A: synchronous, falling edge
		P1IFG &= ~BIT4;
		P1IE &= ~BIT4;
		P1IE |= BIT3;
	}
	P1IFG &= ~(BIT3 + BIT4);
}
//Timer A0 ISR
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TA0_ISR(void){
	if(TA0IV &  0x0a){
    		overflow(); //hooray, this works!
		TA0CTL &= ~TAIFG;
	}
	//if(TA0IV & 0x02){ //TA0CCR1 CCIFG: this should occur if the code actually does as intended
	//	TA0CCTL1 &= ~CCIFG;
	//	P1OUT |= BIT6; //so far this still doesn't turn on
	//	record();
	//}
	TA0CTL &= ~TAIFG;
		//make sure to exit the interrupt if we get here and flags aren't pending
}
//An interrupt occurs at this vector for capture/compare block 1
#pragma vector=TIMER0_A1_VECTOR
__interrupt void capture(void){
	echo = TA0CCR1;
	if(echo<250){
		P1OUT |= BIT6;
	}
	else{
		P1OUT &= ~BIT6;
	}
	TA0CTL = MC_0;
	TA0CCTL1 &= ~CCIFG;
	TA0CCTL1 &= ~COV;
}
