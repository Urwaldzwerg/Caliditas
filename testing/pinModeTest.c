#include <stdio.h>
#include <pigpio.h>

int main(void)
{
  if(gpioInitialise() < 0)
  {
    printf("piGpio initialisation failed\n");
  }else
  {
    printf("piGpio initialisation succeeded");
  }

  gpioSetMode(4, PI_OUTPUT);
  printf("pinMode changed\n");
  //to write to a pin use gpioWrite(PIN, HIGH/LOW);
}
