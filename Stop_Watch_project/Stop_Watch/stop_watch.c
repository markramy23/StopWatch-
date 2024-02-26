/*
 * stop_watch.c
 *
 *  Created on: Sep 15, 2023
 *      Author: Mark
 */
#include <avr/io.h>
#include<avr/interrupt.h>
#include<avr/delay.h>
char seconds=0,minutes=0,hours=0; //golbal variables for time
char time[6]={0}; //array to store units and tenth of time

void timer1_init(){
	//initializing timer 1 with compare mode
	TCCR1A|=(1<<FOC1A)|(1<<FOC1B);
	TCCR1B|=(1<<CS11)|(1<<CS10)|(1<<WGM12); //using 64 prescaler
	TCNT1=0;//setting timer initial value
	OCR1A=15625; //setting compare value for 1 second interrupt
	TIMSK|=1<<OCIE1A; //activating interrupt on compare mode
}
void Int0_init(){ //initializing external interrupt 0
	DDRD&=~(1<<PD2); //initializing pin2 as input pin
	PORTD|=1<<PD2; //using internal pull up network
	MCUCR|=(1<<ISC01);//configure interrupt on falling egde
	GICR|=1<<INT0;//activating int0
}
void Int1_init(){//initializing interrupt 1
	DDRD&=~(1<<PD3); //initializing pin3 as input pin
	//initializing int1 for rising egde
	MCUCR|=(1<<ISC11);
	MCUCR|=(1<<ISC10);
	GICR|=1<<INT1;//activating int1
}
void Int2_init(){ //initializing interrupt 2
	DDRB&=~(1<<PB2); //initializing pin2 as input pin
	PORTB|=1<<PB2;//activating internal pull up
	MCUCSR&=~(1<<ISC2); //activating int2 on faliing edge
	GICR|=1<<INT2;//activating int2
}
void IO_init(){ //initializing all I/O pins
	DDRC|=0x0f; //portc output for displaying on 7-segments
	PORTC=0;
	DDRA|=0x3f; //porta output for enabling 7 segments
	PORTA=0x3f;
	DDRD|=1; //using pin0 in portD to enable the decimal point in the third and fifth segment
	PORTD=0;
}
void time_split(){
	//function to split time to single digits and adding them in the time array
	time[0]=seconds%10;
	time[1]=seconds/10;
	time[2]=minutes%10;
	time[3]=minutes/10;
	time[4]=hours%10;
	time[5]=hours/10;
}
ISR(TIMER1_COMPA_vect){ //timer interrupt handler
	seconds++; //increment seconds with every interrupt
	if(seconds==60){
		//after 60 seconds increment minutes and reset seconds to 0
		minutes++;
		seconds=0;
	}
	if(minutes==60){
		//after 60 minutes increment hours and reset minutes
		hours++;
		minutes=0;
	}

}
ISR(INT0_vect){//interrupt 0 handler to reset the stopwatch
	//reseting all time variables to 0
	seconds=0;
	minutes=0;
	hours=0;
	TCNT1=0; //reseting timer to 0 to begin counting from beginning
}
ISR(INT1_vect){ //interrupt 1 handler for pausing stopwatch
	TCCR1B&=~(1<<CS11)&~(1<<CS10); //clearing clock bits to stop timer from counting
}
ISR(INT2_vect){ //interrupt 2 handler for resuming stopwatch
	TCCR1B|=(1<<CS11)|(1<<CS10); //resetting clock bits to 1 to resume counting by timer
}
int main(void){
	//calling all initializing methods
	IO_init();
	timer1_init();
	Int1_init();
	Int0_init();
	Int2_init();
	SREG|=1<<7; //activating global interrupts

	while(1){
		time_split(); //calling time split before displaying
		PORTA=1; //activating the first display segment only
		//looping through all 6 segments and displaying on them
		for(char i=0;i<6;i++){
			//activating decimal point when displaying on third and fifth decimal
			//to make it more visually clear
			if(i==2||i==4)
				PORTD&=~(1<<PD0);
			else
				PORTD|=1<<PD0;
			PORTC=time[i]; //display on the current segment it's equivalent time digit
			_delay_ms(2); //delay for segment to respond
			PORTA=PORTA<<1;//enabling the next 7 segment to display on it

		}

	}
}
