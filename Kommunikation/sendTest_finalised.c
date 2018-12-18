/*aliditas receive test program, destined to become the final program for the diploma project
	Written by: Mario Sharkhawy

	Compile with: gcc -Wall -pthread -o program program.c -lpigpio -lrt

*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>			//Used for UART
#include <fcntl.h>			//Used for UART
#include <termios.h>		//Used for UART
#include <pigpio.h>			//Used for GPIO programming

#define INTERFACE "/dev/ttyAMA0"		//set your Interface Port here

#define EN485 4					//set the pin that determines RX or TX functionality of the shield

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
	options.c_cflag =  B115200 | CS8 | CLOCAL | PARENB | CMSPAR;
 	options.c_cflag &= ~PARODD;	// SPACE parity (should be 0, 1 == MARK will be detected as parity error);

	options.c_iflag =  INPCK | PARMRK;	// flag parity errors using 3 byte escapes

	tcflush(fd, TCIOFLUSH);
	tcsetattr(fd, TCSANOW, &options);
}

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
	uart0_filestream = open(INTERFACE, O_WRONLY | O_NOCTTY);		//Open in read/write mode
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
    printf("piGpio initialisation succeeded");
  }

	if(gpioSetMode(EN485, PI_OUTPUT) == 0)		//setup RS485 enable pin as output
	{
		//setting gpio mode succeeded
	}else
	{
		printf("failed to set gpio mode\n");
		exit(3);
	}

	if(gpioWrite(EN485, 1) == 0)						//set it to HIGH to enable sending
	{
		//setting pin to HIGH succeeded
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
	_Bool setupDone = 1;							//Variable to determine wether the setup was a success

	if (setup() != 1)
	{
		printf ("error with init\n");
		setupDone = 0;
		exit(10);
	}

	while(setupDone)
	{
		gpioWrite(EN485, 1);
		//----- TX BYTES -----
		//char* tx_buffer = "Hello!\n";
    int l = 10;
		char tx_buffer[] = {65, 0x04, '7', 0x02, 0b01000111, 0b01110111, 0b01000111, 0b01000111, 0x03, 0x88};
		if (uart0_filestream != -1)
		{
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
		}
	}

	//----- CLOSE THE UART -----
	close(uart0_filestream);
}

