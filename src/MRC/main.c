/*
 * MRC.c
 *
 * Created: 08/02/2021 16:59:11
 * Author : Matthew Nickson
 */ 

#define F_CPU 8000000UL //Set clock speed 8MHz

/*Include required header files*/
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

/*Map function from arduino*/
long map(long x, long in_min, long in_max, long out_min, long out_max) {
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/*Main loop*/
int main(void)
{
	/*Define variables*/
	uint8_t adc_lobyte;
	uint16_t raw_adc;
	int rotDirection = 0;
	int pressed = 0;
	int pwmOutput;
	/*Set IO*/
	DDRB |= (1 << PB2); //Pin PB2 as output
	DDRB |= (1 << PB1); //Pin PB1 as output
	DDRB |= (1 << PB0); //Pin PB0 as output
	DDRB &= ~(1 << PINB3); //Pin PB3 as input
	/*Settings for ADC*/
	ADMUX =
		(0 << ADLAR) |     // do not left shift result (for 10-bit values)
		(0 << REFS2) |     // Sets ref. voltage to VCC, bit 2
		(0 << REFS1) |     // Sets ref. voltage to VCC, bit 1
		(0 << REFS0) |     // Sets ref. voltage to VCC, bit 0
		(0 << MUX3)  |     // use ADC2 for input (PB4), MUX bit 3
		(0 << MUX2)  |     // use ADC2 for input (PB4), MUX bit 2
		(1 << MUX1)  |     // use ADC2 for input (PB4), MUX bit 1
		(0 << MUX0);       // use ADC2 for input (PB4), MUX bit 0
	ADCSRA =
		(1 << ADEN)  |     // Enable ADC
		(1 << ADPS2) |     // set prescaler to 64, bit 2
		(1 << ADPS1) |     // set prescaler to 64, bit 1
		(1 << ADPS0);      // set prescaler to 64, bit 0
	/*Settings for PWM*/
	OCR0A = 0; //Set PWM 0% duty cycle
	TCCR0A |= (1 << COM0A1) | (1 << COM0A0); //Set inverting mode
	TCCR0A |= (1 << WGM01) | (1 << WGM00); //Set fast PWM mode
	TCCR0B |= (1 << CS01); //Set prescaler to 8 and starts PWM
	/*
	* Initial rotation direction is not set
	* to facilitate reset being used as 
	* emergency stop.
	*/
    while (1) 
    {
		/*Read potentiometer value*/
		ADCSRA |= (1 << ADSC);
		adc_lobyte = ADCL;
		raw_adc = ADCH<<8 | adc_lobyte;
		pwmOutput = map(raw_adc, 0, 1023, 0, 255); //Map potentiometer value from 0 to 255
		OCR0A = pwmOutput; //Send PWM signal to L298N enable pin
		/*Read button - debounce*/
		if ((PINB & (1 << PINB3)) == (1 << PINB3)) 
		{
			pressed = !pressed;
		}
		while ((PINB & (1 << PINB3)) == (1 << PINB3));
		_delay_ms(20);
		/*If button is pressed change rotation direction*/
		if ((pressed == 1) & (rotDirection == 0))
		{
			PORTB |= (1 << PB1);
			PORTB &= ~(1 << PB2);
			rotDirection = 1;
			_delay_ms(20);			
		}
		if ((pressed == 0) & (rotDirection == 1))
		{
			PORTB &= ~(1 << PB1);
			PORTB |= (1 << PB2);
			rotDirection = 0;
			_delay_ms(20);
		}
    }
}