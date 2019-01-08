#include <stdio.h>
#include <stdlib.h>
#include <pigpio.h>			//Used for GPIO programming

#define PIN 5
#define LED 28

int setup()
{
  if(gpioInitialise() < 0)
  {
    printf("piGpio initialisation failed\n");
		exit(2);
  }else
  {
    printf("piGpio initialisation succeeded\n");
  }

	if(gpioSetMode(PIN, PI_INPUT) == 0)		//setup RS485 enable pin as output
	{
		//setting gpio mode succeeded
	}else
	{
		printf("failed to set gpio mode\n");
		exit(3);
	}

  gpioSetPullUpDown(PIN, PI_PUD_UP);

  if(gpioSetMode(LED, PI_OUTPUT) == 0)		//setup RS485 enable pin as output
	{
		//setting gpio mode succeeded
	}else
	{
		printf("failed to set gpio mode\n");
		exit(3);
	}

  if(gpioWrite(LED, 0) == 0)						//set it to LOW to enable receiving
	{
		//setting pin to LOW succeeded
	}else
	{
		printf("failed to write to led\n");
		exit(4);
	}

  return(1);
}

int main(void)
{
  if(gpioRead(PIN) == 0)
  {
    gpioWrite(LED, 1);
  } else {
    gpioWrite(LED, 0);
  }
}
