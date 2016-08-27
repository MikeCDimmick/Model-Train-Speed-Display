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
void getReadings();
int processSpeed();

#define DISPLAY_TIME 1000	//number proportional to a time that the speed shows up for
#define SEGMENT_PAUSE 500	//time between segment switches in us
#define NUM_READINGS 10		//number of samples to take from the sensor
#define READ_TIME 3000		//total time that program is pulling data from sensor (READ_TIME/NUM_READINGS is the sample period)

int readings[NUM_READINGS];	//array to hold the samples from the sensor, these values are in cm 
int speeds[NUM_READINGS - 2]; //array that holds the speed calculations

int main(void)
{
	DDRB = 0xFF; //PORTB as Output for 7-segment LEDs
	DDRD = 0x07; //PORTD as Output for control, PD0 is digit 1, PD1 is digit 2, PD2 is digit 3
	
	int speed = 0;
	while(1) //infinite loop, take sensor readings, calculate average speed, display 3 digit number
	{
		 getReadings();
		 speed = processSpeed();
		 if(speed > 10)
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

//pulls NUM_READINGS data points from the sensor, places values in an array 
void getReadings()
{
	for(int i = 0; i < NUM_READINGS; i++)
	{
		readings[i] = read_sonar();
		_delay_ms(READ_TIME/NUM_READINGS);
	}
}

//calculates speeds from the distance readings using the centered difference method (dx/dt = (x_n+1 - x_n-1)/2*deltaT)
int processSpeed()
{
	//calculate centered difference and put speed into an array
	for(int i = 0; i < NUM_READINGS-2; i++)
		speeds[i] = abs((readings[i+2]-readings[i])/(2*(READ_TIME/NUM_READINGS)/1000));

	//sum together the speeds
	int sum = 0;
	for(int i = 0; i < NUM_READINGS-2; i++)
			sum += speeds[i];
			
	float avgSpeed = sum/(NUM_READINGS-2);	//calculate average speed in cm/s
	avgSpeed = avgSpeed*0.02237;			//convert speed from cm/s to MPH
	avgSpeed = avgSpeed*87					//scale up from HO to real life
	return (int)avgSpeed;					
}
