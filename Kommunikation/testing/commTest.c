//Program to test communicaion between picture generation and data extraction

#include <stdio.h>
int main (void)
{
  srand(time(0));
  int temp = rand() % 80;
  int hum = rand() % 100;
  printf("T%dH%dK%dE", temp, hum, (temp + hum));
}
