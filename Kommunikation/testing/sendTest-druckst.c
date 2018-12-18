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
struct termios options;

int setup()
{
	//-------------------------
	//----- SETUP UART 0 -----
	//-------------------------
	//At bootup, pins 8 and 10 are already set to UART0_TXD, UART0_RXD (ie the alt0 function) respectively

	//OPEN THE UART
	//The flags (defined in fcntl.h):
	//	Access modes (use 1 of these):
	//		O_RDONLY - Open for reading only.
	//		O_RDWR - Open for reading and writing.
	//		O_WRONLY - Open for writing only.
	//
	//		O_NDELAY / O_NONBLOCK (same function) - Enables nonblocking mode. When set read requests on the file can return immediately with a failure status
	//		if there is no input immediately available (instead of blocking). Likewise, write requests can also return
	//		immediately with a failure status if the output can't be written immediately.
	//
	//		O_NOCTTY - When set and path identifies a terminal device, open() shall not cause the terminal device to become the controlling terminal for the process.
	uart0_filestream = open(INTERFACE, O_RDWR | O_NOCTTY);		//Open in read/write mode
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

	pinMode(EN485, OUTPUT);			//setup RS485 enable pin as output
	digitalWrite(EN485, HIGH);			//set it to HIGH to enable sending

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
	tcgetattr(uart0_filestream, &options);
	options.c_cflag = B115200 | CS8	| CLOCAL | CREAD | PARENB | CMSPAR;	//<Set baud rate to 115k2; Use MARK parity with undocumented CMSPAR flag
//	options.c_cflag &= ~PARENB;
//	options.c_cflag &= ~PARODD;					//uncomment to use SPACE parity
//	options.c_iflag = PARMRK;					//mark parity errors on input to process further

	options.c_oflag = ONLRET;					//don't output CR
	options.c_lflag = 0;
	tcflush(uart0_filestream, TCIOFLUSH);			 	//flush the input and output queues
//	cfmakeraw(&options);						//set terminal to raw mode, read input character by character
	tcsetattr(uart0_filestream, TCSANOW, &options);

	return 1;
}

int main (void)
{
	_Bool setupDone = 1;							//Variable to determine wether the setup was a success

	if (setup() != 1)
	{
		printf ("error with init\n");
		setupDone = 0;
		exit(10);
	}

	while(setupDone)
	{
		digitalWrite(EN485, HIGH);
		//----- TX BYTES -----
		//char* tx_buffer = "Hello!\n";
		char* tx_buffer = "\01\02\03\04AB\n";	// first 4 bytes are pretended to be address bytes, last 3 data...

		if (uart0_filestream != -1)
		{
			digitalWrite(EN485, HIGH);		//Enable RS485 Output

			// write first 4 bytes with MARK Parity (address bit is 1)
			tcgetattr(uart0_filestream, &options);
			options.c_cflag |= PARODD;
			tcsetattr(uart0_filestream, TCSADRAIN, &options);
			int count = write(uart0_filestream, tx_buffer, 4);	//Filestream, bytes to write, number of bytes to write

			// write next 3 bytes with SPACE Parity (address bit is 0)
			tcgetattr(uart0_filestream, &options);
			options.c_cflag &= ~PARODD;
			tcsetattr(uart0_filestream, TCSADRAIN, &options);
			count += write(uart0_filestream, tx_buffer+4, 3);	

			printf("Written %d bytes...\n", count);
			if (count < 0)
			{
				printf("UART TX error\n");
				exit(11);
			}
		}
	}

	//----- CLOSE THE UART -----
	close(uart0_filestream);
}

