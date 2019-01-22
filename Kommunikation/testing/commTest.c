//Program to test communicaion between picture generation and data extraction

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main (void)
{
  while(1)
  {
    srand(time(0));
    int i;
    unsigned char denom;
    char value[5];
    for(i = 0; i < 4; i++)
    {
      //take saved data and send it through the decoder
      value[i] = '1';
      //printf("%s\n", sevenOut[d]);
    }
    value[i] = '\0';
    unsigned char leds = '2';
    if(leds == '1')
    {
      denom = 'T';
    }
    if(leds == '2')
    {
      denom = 'H';
    }

    printf("%c%s\n", denom, value);
  }
}
