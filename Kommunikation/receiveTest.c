/*
	Caliditas receive test program, destined to become the final program for the diploma project
	Written by: Mario Sharkhawy
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>			//Used for UART
#include <fcntl.h>			//Used for UART
#include <termios.h>		//Used for UART
#include <wiringPi.h>		//Used for GPIO programming

#define INTERFACE "/dev/ttyAMA0"		//set your Interface Port here

#define EN485 7					//set the pin that determines RX or TX functionality of the shield

int uart0_filestream = -1;

int setup()
{
	//-------------------------
	//----- SETUP USART 0 -----
	//-------------------------
	//At bootup, pins 8 and 10 are already set to UART0_TXD, UART0_RXD (ie the alt0 function) respectively
//	int uart0_filestream = -1;

	//OPEN THE UART
	//The flags (defined in fcntl.h):
	//	Access modes (use 1 of these):
	//		O_RDONLY - Open for reading only.
	//		O_RDWR - Open for reading and writing.
	//		O_WRONLY - Open for writing only.
	//
	//		O_NDELAY / O_NONBLOCK (same function) - Enables nonblocking mode. When set read requests on the file can return immediately with a failure status
	//											if there is no input immediately available (instead of blocking). Likewise, write requests can also return
	//											immediately with a failure status if the output can't be written immediately.
	//
	//		O_NOCTTY - When set and path identifies a terminal device, open() shall not cause the terminal device to become the controlling terminal for the process.
	uart0_filestream = open(INTERFACE, O_RDWR | O_NOCTTY);		//Open in non blocking read/write mode
	if (uart0_filestream < 0)
	{
		//ERROR - CAN'T OPEN SERIAL PORT
		printf("Error - Unable to open UART.  Ensure it is not in use by another application\n");
		exit(1);
	}

	if(wiringPiSetupGpio() <0)
	{
		printf("set wiringPi lib failed!\n");
		exit(2);
	}else
	{
		printf("set wiringPi lib succeeded\n");
	}

	pinMode(EN485, OUTPUT);					//setup RS485 enable pin as output
	digitalWrite(EN485, LOW);				//set it to LOW to enable receiving

	//CONFIGURE THE UART
	//The flags (defined in /usr/include/termios.h - see http://pubs.opengroup.org/onlinepubs/007908799/xsh/termios.h.html):
	//	Baud rate:- B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800, B500000, B576000, B921600, B1000000, B1152000, B1500000, B2000000, B2500000, B3000000, B3500000, B4000000
	//	CSIZE:- CS5, CS6, CS7, CS8
	//	CLOCAL - Ignore modem status lines
	//	CREAD - Enable receiver
	//	IGNPAR = Ignore characters with parity errors
	//	ICRNL - Map CR to NL on input (Use for ASCII comms where you want to auto correct end of line characters - don't use for bianry comms!)
	//	PARENB - Parity enable
	//	PARODD - Odd parity (else even)
	struct termios options;
	tcgetattr(uart0_filestream, &options);
	options.c_cflag = B9600 | CS8	/*| CLOCAL*/ | CREAD | PARENB;		//<Set baud rate to 115k2; Use MARK parity with undocumented CMSPAR flag
//	options.c_cflag &= ~PARODD;												//uncomment to use SPACE parity
//	options.c_iflag = PARMRK;												//mark parity errors on input to process further
	options.c_oflag = ONLRET;												//don't output CR
	options.c_lflag = 0;
	tcflush(uart0_filestream, TCIOFLUSH);
//	cfmakeraw(&options);														//set terminal to raw mode, read input character by character
	tcsetattr(uart0_filestream, TCSANOW, &options);
	return 1;
}

int main (void)
{
	_Bool setupDone = 1;														//Variable to determine wether the setup was a success

	if (setup() != 1)
	{
		printf ("error with init\n");
		setupDone = 0;
		exit(10);
	}else
	{
		printf("init successful\n");
	}

	while(setupDone)
	{
		digitalWrite(EN485, LOW);
		if (uart0_filestream != -1)
		{
			// Read up to 255 characters from the port if they are there
			unsigned char rx_buffer[256];

			int rx_length = read(uart0_filestream, rx_buffer, 255);		//Filestream, buffer to store in, number of bytes to read (max)
			if (rx_length < 0)
			{
					//An error occured (will occur if there are no bytes)
					printf("An error occured (will occur if there are no bytes)\n");
			}
			else if (rx_length == 0)
			{
				//No data waiting
				printf("No data waiting\n");
			}
			else
			{
				//Bytes received
				rx_buffer[rx_length] = '\0';
				printf("%i bytes read : %s\n", rx_length, rx_buffer);
			}
		}
	}

	//----- CLOSE THE UART -----
	close(uart0_filestream);
}
