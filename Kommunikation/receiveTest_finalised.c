/*
	Caliditas receive test program, destined to become the final program for the diploma project
	Written by: Mario Sharkhawy
	Compile with: gcc -Wall -pthread -o program program.c -lpigpio -lrt
*/

//libraries
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>     //Used for UART
#include <fcntl.h>      //Used for UART
#include <termios.h>      //Used for UART
#include <pigpio.h>     //Used for GPIO programming

//defines
#define INTERFACE "/dev/ttyAMA0"      //set your Interface Port here
#define MAXDATA 10
#define EN485 4     //set the pin that determines RX or TX functionality of the shield

//address variables
static const unsigned char addressSlave = 0x41;     //set address of slave here
static const unsigned char addressGroup = 0x40;     //set address of broadcast call here

//BUS setup
int uart0_filestream = -1;
struct termios options;     //initialise global struct for configuring our BUS

//functions for sending
int sendData(int fd, char tx_chr)
{
  // write next byte with SPACE parity (address bit is 0)
  tcgetattr(uart0_filestream, &options);      //get current options
  options.c_cflag &= ~PARODD;      //set SPACE parity
  tcsetattr(uart0_filestream, TCSADRAIN, &options);     //set changed attributes
  int count = write(fd, &tx_chr, 1);      //write one byte
  tcflush(fd, TCIOFLUSH);     //flush anything incoming or outgoing that hasn't been sent or received
  if (count != 1) {     //if count is not 1 then the write failed
//    printf("wrerr\n");      //let user know
    exit(-1);     //exit the program
  }
  return count;
}

int sendAddress(int fd, char tx_chr)
{
  // write first 4 bytes with MARK Parity (address bit is 1)
  tcgetattr(uart0_filestream, &options);      //get current options
  options.c_cflag |= PARODD;      //set MARK parity
  tcsetattr(uart0_filestream, TCSADRAIN, &options);     //set changed attributes
  int count = write(fd, &tx_chr, 1);      //write one byte
  tcflush(fd, TCIOFLUSH);      //flush anything incoming or outgoing that hasn't been sent or received
  if (count != 1) {     //if count is not 1 then the write failed
//    printf("wrerr\n");      //let user know
    exit(-1);     //exit the program
  }
  return count;
}

//functions for receiving
unsigned char readByteWithParity(int fd, int *status)
{
  //function to read an incoming byte and determine it's parity for later use
  unsigned char buf;
  // read 1st byte
  if (read(fd, &buf, 1) != 1)
  {
    *status = -1;	// error
    return 0;
  }
  else if (buf != 0xFF)
  {
    *status = 0;	// ok - normal data
    return buf;
  }
  // read next byte
  if (read(fd, &buf, 1) != 1)
  {
    *status = -1;	// error again
    return 0;
  }
  else if (buf == 0xFF)
  {
    // just 0xFF that was escaped
    *status = 0;
    return buf;
  }
  else if (buf == 0)
  {
    // parity "error" detected
    // read next byte	// get real data byte
    if (read(fd, &buf, 1) != 1)
    {
      *status = -1;	// error again
      return 0;
    }
    *status = 1;	// flag as address byte
    return buf;
  }
  return buf;
}

//funcions for evaluation

static const char* sevenOut[256] = {
  //array to save specific bit-patterns used for decoding some data
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
//  [0b01011111] = "G",
  [0b00110111] = "H",
  [0b10110111] = "H.",
  [0b00010111] = "h",
  [0b00000110] = "I",
  [0b00001110] = "L",
  [0b01110110] = "fancyU",
  [0b00011101] = "o",
  [0b00010101] = "n",
  [0b01100111] = "p",
//  [0b01011011] = "S",
  [0b00001111] = "t",
  [0b00000101] = "r",
  [0b10000101] = "r.",
  [0b00111110] = "U",
  [0b00011100] = "u",
  [0b00111011] = "y",
//  [0b01101101] = "Z",
  [0b01100011] = "°"
};

_Bool checkAddress(unsigned char addr)
{
  //function to check if received address byte matches one of the addresses
  return (addr == addressSlave || addr == addressGroup);
}

int readFrame(int fd, unsigned char* func, unsigned char* data)
{
  //function to read a frame according to documentation and save the frame and compare checkSum
	int status = -1;     //prerequisite for readByteWithParity
	unsigned char cSum = 0;      //initialise checkSum

  unsigned char dataLen = readByteWithParity(fd, &status);     //read length of data block
  if (status != 0)
  {
    return -1;      //If anything but data is received the frame is invalid
  }

  *func = readByteWithParity(fd, &status);      //read the function of the frame
  if (status != 0)
  {
    return -1;      //If anything but data is received the frame is invalid
  }
  cSum += *func;      //add the function to the checkSum

  if (readByteWithParity(fd, &status) != 0x02)      //see if next byte is STX
  {
    return -1;
  }
  if (status != 0)
  {
    return -1;      //If anything but data is received the frame is invalid
  }
  cSum += 0x02;     //add STX to the checkSum

  int i;
  for(i = 0; i < dataLen; i ++)     //read data block, based off of it's length saved earlier
  {
    data[i] = readByteWithParity(fd, &status);
    if (status != 0)
    {
      return -1;      //If anything but data is received the frame is invalid
    }
    cSum += data[i];      //add data block to checkSum
  }

  if (readByteWithParity(fd, &status) != 0x03)      //see if next byte is ETX
  {
//    printf("frame exit %d\n", status);
    return -1;
  }
  if (status != 0)
  {
    return -1;      //If anything but data is received the frame is invalid
  }
  cSum += 0x03;     //add ETX to checkSum

  if (readByteWithParity(fd, &status) != cSum)      //see if next byte is same as our checkSum
  {
    return -1;
  }
  if (status != 0)
  {
    return -1;      //If anything but data is received the frame is invalid
  }
  return i;
}

int processData(unsigned char* func, unsigned char* data, int len)
{
  //function to process the data received and act upon it
  switch(*func)
  {
    case 'I':
    {     //Initialisation
//      printf("Initialisation\n");
      int l = 7;      //set length of response
      char tx_buffer[] = {0x10, 0x01, 'I', 0x02, 0x00, 0x03, 0x4E};     //buffer for answer to be sent
      gpioWrite(EN485, 1);      //Enable RS485 Output
      int count = 0;      //count how many bytes were sent
      int i = 0;
      count = sendAddress(uart0_filestream, tx_buffer[i]);      //send first byte which is the address byte
      i++;
      for(; i < l; i++)     //send remaining buffer bytes as data bytes
      {
        count += sendData(uart0_filestream, tx_buffer[i]);
      }

//      printf("Written %d bytes...\n", count);     //print the bytes written
      if (count < 0)      //if count is smaller than 0 the write failed
      {
//        printf("UART TX error\n");      //let user know
        exit(11);     //exit the program
      }
      return 0;
    }
    case '7':
    {     //Seven Segment Display
//      printf("7 Segments\n");
      int i;
			unsigned char denom = '0';

      const char* digit1 = sevenOut[data[0]];
      const char* digit2 = sevenOut[data[1]];
      const char* digit3 = sevenOut[data[2]];
      const char* digit4 = sevenOut[data[3]];

      if(digit1 == "°")
      {
        denom = 'T';
        printf("%c%s%s\0\n", denom, /*digit4,*/ digit3, digit2);
      }
      if(digit1 == "r." || digit1 == "F.")
      {
        denom = 'H';
        printf("%c%s%s\0\n", denom, digit4, digit3);
      }

      return 0;
    }

    case 'W':
    {     //Slave Temperature
//      printf("Slave Temperature\n");
      int l = 7;
      char tx_buffer[] = {0x10, 0x01, 'W', 0x02, 0b00011001, 0x03, 0x75};
      gpioWrite(EN485, 1);      //Enable RS485 Output
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

    case 'T':
    {     //Slave Buttons
//      printf("Slave Buttons\n");
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
//		printf("Error - Unable to open UART.  Ensure it is not in use by another application\n");
		exit(1);
	}
	if(gpioInitialise() < 0)
  {
//    printf("piGpio initialisation failed\n");
		exit(2);
  }else
  {
//    printf("piGpio initialisation succeeded\n");
  }

	if(gpioSetMode(EN485, PI_OUTPUT) == 0)		//setup RS485 enable pin as output
	{
		//setting gpio mode succeeded
	}else
	{
//		printf("failed to set gpio mode\n");
		exit(3);
	}

	if(gpioWrite(EN485, 0) == 0)						//set it to LOW to enable receiving
	{
		//setting pin to LOW succeeded
	}else
	{
//		printf("failed to write to pin\n");
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
//		printf("error with init\n");
		exit(10);
	}
	else
	{
//		printf("init successful\n");
	}

	while (1)
	{
    if (uart0_filestream > 0)
		{
      int status = -1;
      gpioWrite(EN485, 0);
  		unsigned char in;
      in = readByteWithParity(uart0_filestream, &status);
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
  			default: /*printf("Error!\n");*/ continue;
  		}
  		unsigned char func;
  		unsigned char data[256];
  		int len = readFrame(uart0_filestream, &func, data);
  		if (len < 0)
  		{
//  			printf("Invalid Frame!\n");
  			continue;
  		}
/*  		printf("Function: %c\n Frame:", func);
  		for (int i = 0; i < len; i ++)
  		{
        printf(" %02x", data[i]);
  		}
  		printf("\n");

*/      if (processData(&func, data, len) < 0)
      {
//        printf("Process Data failed!\n");
      }
    }
  }

	//----- CLOSE THE UART -----
	close(uart0_filestream);
}
