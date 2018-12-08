/*
	Caliditas receive test program, destined to become the final program for the diploma project
	Written by: Mario Sharkhawy

	Compile with: gcc -Wall -pthread -o program program.c -lpigpio -lrt

*/
#include <stdio.h>
#include <stdlib.h>

int randomValue(){
  int v;
  v = rand() %39 + 1;
  return v;
}

unsigned char determineData(int v){
  switch(v){
    case 1: {
      return 0b10000000;
    }
    case 2: {
      return 0b01111110;
    }
    case 3: {
      return 0b00110000;
    }
    case 4: {
      return 0b01101101;
    }
    case 5: {
      return 0b01111001;
    }
    case 6: {
      return 0b00110011;
    }
    case 7: {
      return 0b01011011;
    }
    case 8: {
      return 0b01011111;
    }
    case 9: {
      return 0b01110000;
    }
    case 10: {
      return 0b01111111;
    }
    case 11: {
      return 0b01111011;
    }
    case 12: {
      return 0b00000001;
    }
    case 13: {
      return 0b00001001;
    }
    case 14: {
      return 0b01110111;
    }
    case 15: {
      return 0b00011111;
    }
    case 16: {
      return 0b01001110;
    }
    case 17: {
      return 0b00111101;
    }
    case 18: {
      return 0b01001111;
    }
    case 19: {
      return 0b01000111;
    }
    case 20: {
      return 0b11000111;
    }
    case 21: {
      return 0b01011111;
    }
    case 22: {
      return 0b00110111;
    }
    case 23: {
      return 0b10110111;
    }
    case 24: {
      return 0b00010111;
    }
    case 25: {
      return 0b00000110;
    }
    case 26: {
      return 0b00001110;
    }
    case 27: {
      return 0b01110110;
    }
    case 28: {
      return 0b00011101;
    }
    case 29: {
      return 0b00010101;
    }
    case 30: {
      return 0b01100111;
    }
    case 31: {
      return 0b01011011;
    }
    case 32: {
      return 0b00001111;
    }
    case 33: {
      return 0b00000101;
    }
    case 34: {
      return 0b10000101;
    }
    case 35: {
      return 0b00111110;
    }
    case 36: {
      return 0b00011100;
    }
    case 37: {
      return 0b00111011;
    }
    case 38: {
      return 0b01101101;
    }
    case 39: {
      return 0b01100011;
    }
    return -1;
  }
}

int main (void)
{
  while(1){
  	int value = randomValue();
    unsigned char data = determineData(value);
    printf("%c\n", data);
  }
}
