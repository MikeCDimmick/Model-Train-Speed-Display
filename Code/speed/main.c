/*
 * speed.c
 *
 * Created: 2016-05-09 12:14:49 AM
 * Author : Mike
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <sonar.h>
#include <stdlib.h>

void setDisplay(int displayNum, int number);
unsigned int count(unsigned int i);
void processThreeDigitNumber(int number);
int* getReadings();
int processSpeed();

//data to the displays are sent from PORT B
#define SEVEN_SEG_DISPLAY_PORT PORTB
#define SEVEN_SEG_DISPLAY_DDR  DDRB
#define SEVEN_SEG_DATA_PINS  0x7F

#define SEVEN_SEG_CONTROL_PORT PORTD
#define SEVEN_SEG_CONTROL_DDR  DDRD
#define SEVEN_SEG_CONTROL_PINS  0x07

#define MINIMUM_DISPLAY_SPEED 10

#define DISPLAY_TIME 1000	//number proportional to a time that the speed shows up for
#define SEGMENT_PAUSE 500	//time between segment switches in us
#define POSITION_SAMPLES 10		//number of samples to take from the sensor
#define SPEED_SAMPLES 8
#define READ_TIME 3000		//total time that program is pulling data from sensor (READ_TIME/POSITION_SAMPLES is the sample period)


int main(void)
{
	SEVEN_SEG_DISPLAY_DDR = SEVEN_SEG_DATA_PINS; //PORTB as Output for 7-segment LEDs
	SEVEN_SEG_CONTROL_DDR = SEVEN_SEG_CONTROL_PINS; //PORTD as Output for control, PD0 is digit 1, PD1 is digit 2, PD2 is digit 3
	
	int speed = 0;
	int *positions;
	while(1) //infinite loop, take sensor readings, calculate average speed, display 3 digit number
	{
		 positions = getReadings();
		 speed = processSpeed(positions);
		 if(speed > MINIMUM_DISPLAY_SPEED)
			 processThreeDigitNumber(speed);

	}
}

//count the number of digits in a number
unsigned int count(unsigned int i)
{
	unsigned int ret=1;
	while (i/=10) ret++;
	return ret;
}

//takes in a three digit number and develops the data to be sent to the displays
void processThreeDigitNumber(int number)
{
	//break the three digit number into its three digits and place them in an array
	unsigned int dig=count(number);
	unsigned int compare = dig;
	char arr[dig];
	while (dig--) 
	{
		arr[dig]=number%10;
		number/=10;
	}
	
	//arr has the following contents: [digit 3, digit 2, digit 1]
	//if there are three digits then we can just send out the array
	if(compare == 3)
	{
		//set the displays for a desired period of time
		for(int i = 0; i < DISPLAY_TIME; i++)
		{
			setDisplay(3,arr[0]);
			_delay_us(SEGMENT_PAUSE);
			setDisplay(2,arr[1]);
			_delay_us(SEGMENT_PAUSE);
			setDisplay(1,arr[2]);
			_delay_us(SEGMENT_PAUSE);
		}
	}
	//if there are only two digits just make digit 3 a 0, and use the two digits to construct the number on digits 1 and 2
	else if(compare == 2)
	{
		//set the displays for a desired period of time
		for(int i = 0; i < DISPLAY_TIME; i++)
		{
			setDisplay(3,0);
			_delay_us(SEGMENT_PAUSE);
			setDisplay(2,arr[0]);
			_delay_us(SEGMENT_PAUSE);
			setDisplay(1,arr[1]);
			_delay_us(SEGMENT_PAUSE);
		}
	}
	//if there is only one digit just make digit 3 and 2 a 0, and use the single digit to construct the number on digit 1
	else if(compare == 1)
	{
		//set the displays for a desired period of time
		for(int i = 0; i < DISPLAY_TIME; i++)
		{
			setDisplay(3,0);
			_delay_us(SEGMENT_PAUSE);
			setDisplay(2,0);
			_delay_us(SEGMENT_PAUSE);
			setDisplay(1,arr[0]);
			_delay_us(SEGMENT_PAUSE);

		}
	}
	//turn off display
	PORTB = 0x00;
	PORTD = 0x00;
}

//turns on the desired 7seg (either 1,2, or 3) and displays the desired number (0-9)
void setDisplay(int displayNum, int number)
{
	//turn on correct control
	if(displayNum == 1)
		PORTD = 0x01;
	else if (displayNum == 2)
		PORTD = 0x02;
	else if (displayNum == 3)
		PORTD = 0x04;
	
	//turn on correct LEDs
	if(number == 0)
		PORTB = 0xC0;
	else if(number == 1)
		PORTB = 0xF9;
	else if(number == 2)
		PORTB = 0xA4;
	else if(number == 3)
		PORTB = 0xB0;
	else if(number == 4)
		PORTB = 0x99;
	else if(number == 5)
		PORTB = 0x92;
	else if(number == 6)
		PORTB = 0x82;
	else if(number == 7)
		PORTB = 0xF8;
	else if(number == 8)
		PORTB = 0x80;
	else if(number == 9)
		PORTB = 0x98;
}

//pulls POSITION_SAMPLES data points from the sensor, places values in an array 
int* getReadings()
{
	static int positions[POSITION_SAMPLES];	//array to hold the samples from the sensor, these values are in cm
	
	for(int i = 0; i < POSITION_SAMPLES; i++)
	{
		positions[i] = read_sonar();
		_delay_ms(READ_TIME/POSITION_SAMPLES);
	}
	return positions;
}

//calculates speeds from the distance readings using the centered difference method (dx/dt = (x_n+1 - x_n-1)/2*deltaT)
int processSpeed(int* positions)
{
	float speeds[SPEED_SAMPLES];
	float sum = 0;

	//calculate centered difference and put speed into an array
	for(int i = 0; i < SPEED_SAMPLES; i++)
	{
		speeds[i] = abs((positions[i+2]-positions[i])/(2*(READ_TIME/POSITION_SAMPLES)/1000.0));
		sum += speeds[i];
	}
			
	float avgSpeed = sum/(SPEED_SAMPLES);	//calculate average speed in cm/s
	avgSpeed = avgSpeed*0.02237;			//convert speed from cm/s to MPH
	avgSpeed = avgSpeed*87;					//scale up from HO to real life
	return (int)avgSpeed;					
}
