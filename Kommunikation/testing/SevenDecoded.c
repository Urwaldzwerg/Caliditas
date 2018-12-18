#include <stdio.h>
#include <stdlib.h>

char evaluateData(unsigned char d, unsigned char *out){
	switch(d){
		case 0b10000000: {
			*out = ".";
		}
		case 0b01111110: {
			*out = "0";
		}
		case 0b00110000: {
			*out = "1";
		}
		case 0b01101101: {
			*out = "2";
		}
		case 0b01111001: {
			*out = "3";
		}
		case 0b00110011: {
			*out = "4";
		}
		case 0b01011011: {
			*out = "5";
		}
		case 0b01011111: {
			*out = "6";
		}
		case 0b01110000: {
			*out = "7";
		}
		case 0b01111111: {
			*out = "8";
		}
		case 0b01111011: {
			*out = "9";
		}
		case 0b00000001: {
			*out = "-";
		}
		case 0b00001001: {
			*out = "=";
		}
		case 0b01110111: {
			*out = "A";
		}
		case 0b00011111: {
			*out = "b";
		}
		case 0b01001110: {
			*out = "C";
		}
		case 0b00111101: {
			*out = "d";
		}
		case 0b01001111: {
			*out = "E";
		}
		case 0b01000111: {
			*out = "F";
		}
		case 0b11000111: {
			*out = "F.";
		}
		case 0b00110111: {
			*out = "H";
		}
		case 0b10110111: {
			*out = "H.";
		}
		case 0b00010111: {
			*out = "h";
		}
		case 0b00000110: {
			*out = "I";
		}
		case 0b00001110: {
			*out = "L";
		}
		case 0b01110110: {
			*out = "fancyU";
		}
		case 0b00011101: {
			*out = "o";
		}
		case 0b00010101: {
			*out = "n";
		}
		case 0b01100111: {
			*out = "p";
		}
		case 0b00001111: {
			*out = "t";
		}
		case 0b00000101: {
			*out = "r";
		}
		case 0b10000101: {
			*out = "r.";
		}
		case 0b00111110: {
			*out = "U";
		}
		case 0b00011100: {
			*out = "u";
		}
		case 0b00111011: {
			*out = 'y';
		}
		case 0b01100011: {
			*out = '�';
		}
	}
}

int main(int argc, char *argv[]){
	FILE *filep;
	printf("Reading\n");
	if (argc != 2) {
		fprintf (stderr, "\aBitte Dateinamen beim Programmaufruf angeben!\n");
		return -1;
	} else if ((filep = fopen(argv[1], "r")) == NULL) {
		fprintf (stderr, "\aDatei \"%s\" nicht lesbar!\n", argv[1]);
		return -2;
	}
	printf("Start\n");
	unsigned char data[100000];
	unsigned char out;
	fscanf (filep, "%c", &data);

	for(int i = 0;i < 100000; i++){

		//scanf("%c", &data);
		evaluateData(data[i], &out);

		printf("Output: %c", out);
	}
}
