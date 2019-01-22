#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

float return_val(char * value) {
  int i;
  for(i = 0; i < 80; i++) {
    if(value[i] == '\0') {
      i--;
      break;
    }
  }

  char str[20];

  strncpy(str, value + 1, i );
  return atof(str);
}

int main( void ) {
  while(1) {
    float currentTemp = 0;
    float currentHum = 0;

    char value[80];

    scanf( "%s", &value );

    switch( value[0] ) {
      case 'T':
        currentTemp = return_val(value);
        printf( "Temp: %.f \n", currentTemp );
        break;

      case 'H':
        currentHum = return_val(value);
        printf( "Hum: %.f \n", currentHum );
        break;
    }
  }
}
