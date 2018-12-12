/*
	Caliditas receive test program, destined to become the final program for the diploma project
	Written by: Mario Sharkhawy

	Compile with: gcc -Wall -pthread -o program program.c -lpigpio -lrt

*/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>			//Used for UART
#include <fcntl.h>			//Used for UART
#include <termios.h>		//Used for UART
#include <pigpio.h>			//Used for GPIO programming

#define INTERFACE "/dev/ttyAMA0"		//set your Interface Port here
#define MAXDATA 10
#define EN485 4					//set the pin that determines RX or TX functionality of the shield

static const unsigned char addressSlave = 0x41;		//set adress of slave here; Sting A should represent 0x41
static const unsigned char addressGroup = 0x40;

int uart0_filestream = -1;
struct termios options;

int sendData(int fd, char tx_chr)
{
  // write next byte with SPACE Parity (address bit is 0)
  tcgetattr(uart0_filestream, &options);
  options.c_cflag &= ~PARODD;
  tcsetattr(uart0_filestream, TCSADRAIN, &options);
  int count = write(fd, &tx_chr, 1);
	tcflush(fd, TCIOFLUSH);
  if (count != 1) {
    printf("wrerr\n");
    exit(-1);
  }
  return count;
}

int sendAddress(int fd, char tx_chr)
{
  // write first 4 bytes with MARK Parity (address bit is 1)
  tcgetattr(uart0_filestream, &options);
  options.c_cflag |= PARODD;
  tcsetattr(uart0_filestream, TCSADRAIN, &options);
  int count = write(fd, &tx_chr, 1);
	tcflush(fd, TCIOFLUSH);
  if (count != 1) {
    printf("wrerr\n");
    exit(-1);
  }
  printf("count level: %d\n", count);
  return count;
}

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
	return buf;
}

static const char* sevenOut[256] = {
  [0b10000000] = ".",
  [0b01111110] = "0",
  [0b00110000] = "1",
  [0b01101101] = "2",
  [0b01111001] = "3",
  [0b00110011] = "4",
  [0b01011011] = "5",
  [0b01011111] = "6",
  [0b01110000] = "7",
  [0b01111111] = "8",
  [0b01111011] = "9",
  [0b00000001] = "-",
  [0b00001001] = "=",
  [0b01110111] = "A",
  [0b00011111] = "b",
  [0b01001110] = "C",
  [0b00111101] = "d",
  [0b01001111] = "E",
  [0b01000111] = "F",
  [0b11000111] = "F.",
  [0b01011111] = "G",
  [0b00110111] = "H",
  [0b10110111] = "H.",
  [0b00010111] = "h",
  [0b00000110] = "I",
  [0b00001110] = "L",
  [0b01110110] = "fancyU",
  [0b00011101] = "o",
  [0b00010101] = "n",
  [0b01100111] = "p",
  [0b01011011] = "S",
  [0b00001111] = "t",
  [0b00000101] = "r",
  [0b10000101] = "r.",
  [0b00111110] = "U",
  [0b00011100] = "u",
  [0b00111011] = "y",
  [0b01101101] = "Z",
  [0b01100011] = "°"
};

_Bool checkAddress(unsigned char addr)
{
	return (addr == addressSlave || addr == addressGroup);
}

int readFrame(int fd, unsigned char* func, unsigned char* data)
{
	int status = -1;
	unsigned char cSum = 0;

	unsigned char dataLen = readByteWithParity(fd, &status);
	if (status != 0)
	{
		return -1;		//If anything but data is received the frame is invalid
	}

	*func = readByteWithParity(fd, &status);
	if (status != 0)
	{
		return -1;		//If anything but data is received the frame is invalid
	}
	cSum += *func;

	if (readByteWithParity(fd, &status) != 0x02)
	{
		return -1;
	}
	if (status != 0)
	{
		return -1;		//If anything but data is received the frame is invalid
	}
	cSum += 0x02;

	int i;
	for(i = 0; i < dataLen; i ++)
	{
 		data[i] = readByteWithParity(fd, &status);
		if (status != 0)
		{
			return -1;		//If anything but data is received the frame is invalid
		}
		cSum += data[i];
	}

	if (readByteWithParity(fd, &status) != 0x03)
	{
		printf("frame exit %d\n", status);
		return -1;
	}
	if (status != 0)
	{
		return -1;		//If anything but data is received the frame is invalid
	}
	cSum += 0x03;

	if (readByteWithParity(fd, &status) != cSum)
	{
		return -1;
	}
	if (status != 0)
	{
		return -1;		//If anything but data is received the frame is invalid
	}
	return i;
}

int processData(unsigned char* func, unsigned char* data, int len)
{
  switch(*func) {
    case 'I': {   //Initialisation
      printf("Initialisation\n");
			int l = 7;
			char tx_buffer[] = {0x10, 0x01, 'I', 0x02, 0x00, 0x03, 0x4E};
			gpioWrite(EN485, 1);		//Enable RS485 Output
			int count = 0;
			int i = 0;
      count = sendAddress(uart0_filestream, tx_buffer[i]);
			i++;
      for(; i < l; i++)
			{
				count += sendData(uart0_filestream, tx_buffer[i]);
			}

			printf("Written %d bytes...\n", count);
			if (count < 0)
			{
				printf("UART TX error\n");
				exit(11);
			}
      return 0;
    }
    case '7': {   //Seven Segment Display
			int i;
      for(i = 0; i < len; i++)
      {
        unsigned char d = data[i];
	printf("%s\n", sevenOut[d]);
      }

      return 0;
    }
    case 'W': {   //Slave Temperature
      printf("Slave Temperature\n");
      int l = 7;
      char tx_buffer[] = {0x10, 0x01, 'W', 0x02, 0b00011001, 0x03, 0x75};
      gpioWrite(EN485, 1);		//Enable RS485 Output
			int count = 0;
			int i = 0;
      count = sendAddress(uart0_filestream, tx_buffer[i]);
			i++;
      for(; i < l; i++)
			{
				count += sendData(uart0_filestream, tx_buffer[i]);
			}
      return 0;
    }
    case 'T': {   //Slave Buttons
      printf("Slave Buttons\n");
      return 0;
    }
    default:
			return -1;
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
	cfmakeraw(&options);
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
	if(gpioInitialise() < 0)
  {
    printf("piGpio initialisation failed\n");
		exit(2);
  }else
  {
    printf("piGpio initialisation succeeded\n");
  }

	if(gpioSetMode(EN485, PI_OUTPUT) == 0)		//setup RS485 enable pin as output
	{
		//setting gpio mode succeeded
	}else
	{
		printf("failed to set gpio mode\n");
		exit(3);
	}

	if(gpioWrite(EN485, 0) == 0)						//set it to LOW to enable receiving
	{
		//setting pin to LOW succeeded
	}else
	{
		printf("failed to write to pin\n");
		exit(4);
	}

	//CONFIGURE THE UART
	setupSerial(uart0_filestream);
	return 1;
}

int main (void)
{
	if (setup() != 1)
	{
		printf("error with init\n");
		exit(10);
	}
	else
	{
		printf("init successful\n");
	}

	while (1)
	{
    if (uart0_filestream > 0)
		{
      int status = -1;
      gpioWrite(EN485, 0);
      printf("here\n");
  		unsigned char in;
      in = readByteWithParity(uart0_filestream, &status);
      printf("Status: %d, chr: %c, byte: %02x\n", status, in, in);
  		switch (status)
  		{
  			case 1: {
  				if (checkAddress(in))
  				{
  					break;
  				}
  			}
  			case 0: continue;
  			case -1:
  			default: printf("Error!\n"); continue;
  		}
  		unsigned char func;
  		unsigned char data[256];
  		int len = readFrame(uart0_filestream, &func, data);
  		if (len < 0)
  		{
  			printf("Invalid Frame!\n");
  			continue;
  		}
  		printf("Function: %c\n Frame:", func);
  		for (int i = 0; i < len; i ++)
  		{
  			printf(" %02x", data[i]);
  		}
  		printf("\n");
      if (processData(&func, data, len) < 0)
      {
        printf("Process Data failed!\n");
      }
    }
  }

	//----- CLOSE THE UART -----
	close(uart0_filestream);
}
