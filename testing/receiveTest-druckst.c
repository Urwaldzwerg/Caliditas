/*
	Caliditas receive test program, destined to become the final program for the diploma project
	Written by: Mario Sharkhawy
*/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>			//Used for UART
#include <fcntl.h>			//Used for UART
#include <termios.h>		//Used for UART
#include <wiringPi.h>		//Used for GPIO programming

#define INTERFACE "/dev/ttyAMA0"		//set your Interface Port here

#define EN485 7					//set the pin that determines RX or TX functionality of the shield

int uart0_filestream = -1;


unsigned char readByteWithParity(int fd, int *status) {
	
	unsigned char buf;

	// read 1st byte
	if (read(fd, &buf, 1) != 1) {
		*status = -1;	// error
		return 0;
	}
	else if (buf != 0xFF) {
		*status = 0;	// ok - normal data
		return buf;
	} 
	// read next byte 
	if (read(fd, &buf, 1) != 1) {
		*status = -1;	// error again
		return 0;
	}
	else if (buf == 0xFF) {	// just 0xFF that was escaped
		*status = 0;
		return buf;	
	}
	else if (buf == 0) {	// parity "error" detected
		// read next byte	// get real data byte 
		if (read(fd, &buf, 1) != 1) {
			*status = -1;	// error again
			return 0;
		}
		*status = 1;	// flag as address byte	
		return buf;
	} 
} 

void setupSerial(int fd) {

	//CONFIGURE THE UART
	//The flags (defined in /usr/include/termios.h - see http://pubs.opengroup.org/onlinepubs/007908799/xsh/termios.h.html):
	//	Baud rate:- B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800, B500000, B576000, B921600, B1000000, B1152000, B1500000, B2000000, B2500000, B3000000, B3500000, B4000000
	//	CSIZE:- CS5, CS6, CS7, CS8
	//	CLOCAL - Ignore modem status lines
	//	CREAD - Enable receiver
	//	IGNPAR = Ignore characters with parity errors
	struct termios options;
	tcgetattr(fd, &options);
	options.c_cflag &= ~CSIZE;
	options.c_cflag =  B115200 | CS8 | CLOCAL | CREAD | PARENB | CMSPAR;
 	options.c_cflag &= ~PARODD;	// SPACE parity (should be 0, 1 == MARK will be detected as parity error);

	options.c_iflag =  INPCK | PARMRK;	// flag parity errors using 3 byte escapes

	tcflush(fd, TCIOFLUSH);
	tcsetattr(fd, TCSANOW, &options);
}

int setup()
{
	//----- SETUP UART 0 -----
        //-------------------------
        //At bootup, pins 8 and 10 are already set to UART0_TXD, UART0_RXD (ie the alt0 function) respectively

        //OPEN THE UART
        //The flags (defined in fcntl.h):
        //      Access modes (use 1 of these):
        //              O_RDONLY - Open for reading only.
        //              O_RDWR - Open for reading and writing.
        //              O_WRONLY - Open for writing only.
        //
        //              O_NDELAY / O_NONBLOCK (same function) - Enables nonblocking mode. When set read requests on the file can return immediately with a failure status
        //              if there is no input immediately available (instead of blocking). Likewise, write requests can also return
        //              immediately with a failure status if the output can't be written immediately.
        //
        //              O_NOCTTY - When set and path identifies a terminal device, open() shall not cause the terminal device to become the controlling terminal for the process.


	uart0_filestream = open(INTERFACE, O_RDWR | O_NOCTTY);		//Open in non blocking read/write mode (dru: ?)
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

	pinMode(EN485, OUTPUT);				//setup RS485 enable pin as output
	digitalWrite(EN485, LOW);				//set it to LOW to enable receiving

	//CONFIGURE THE UART
	setupSerial(uart0_filestream);
	return 1;
}

int main (void)
{
	_Bool setupDone = 1;					//Variable to determine wether the setup was a success

	if (setup() != 1)
	{
		printf ("error with init\n");
		setupDone = 0;
		exit(10);
	}
	else
	{
		printf("init successful\n");
	}

	while(setupDone)
	{
		digitalWrite(EN485, LOW);
		if (uart0_filestream > 0)
		{
			int status = -1;
			unsigned char c;
			c = readByteWithParity(uart0_filestream, &status);
			if (status >= 0)
				printf("0x%03x(%c)\n", (int)(((int)c)+(status<<8)), isprint(c) ? c : ' ');
			else if (status == -1) {
				printf("READ ERROR!");
				exit(1);
			}
			else {
				printf("Invalid read status %d\n", status);
				exit(1);
			}
		}
	}

	//----- CLOSE THE UART -----
	close(uart0_filestream);
}

