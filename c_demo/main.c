#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>

#include "main.h"


int tty_fd;

 
int main(int argc,char** argv)
{
        struct termios tio;
 
 
        memset(&tio,0,sizeof(tio));
        tio.c_iflag=0;
        tio.c_oflag=0;
        tio.c_cflag=CS8|CREAD|CLOCAL;           // 8n1, see termios.h for more information
        tio.c_lflag=0;
        tio.c_cc[VMIN]=1;
        tio.c_cc[VTIME]=5;
 
        tty_fd=open("/dev/ttyUSB0", O_RDWR | O_NONBLOCK);      
        cfsetospeed(&tio,B500000);            // 115200 baud
        cfsetispeed(&tio,B500000);            // 115200 baud
 
        tcsetattr(tty_fd,TCSANOW,&tio);
//                if (read(tty_fd,&c,1)>0)        write(STDOUT_FILENO,&c,1);              // if new data is available on the serial port, print it out
//                if (read(STDIN_FILENO,&c,1)>0)  write(tty_fd,&c,1);                     // if new data is available on the console, send it to the serial port


    for(int y = 0 ; y < 32; y++)
	{
		for(int x = 0; x < 72;x++)
		{
			pixel(x,y,0);
		}
	}


	while(1)
	{
		for(int x = 0; x < 72;x++)
		{
	    	for(int y = 0 ; y < 32; y++)
			{
				pixel(x,y,15);
				usleep(8000);
				pixel(x,y,0);
			}
		}

	    for(int y = 0 ; y < 32; y++)
		{
			for(int x = 0; x < 72;x++)
			{
				pixel(x,y,15);
				usleep(8000);
				pixel(x,y,0);
			}
		}
	}

                                                            

 
	close(tty_fd);
}


void pixel(int x, int y, unsigned char color) 
{

			y = 31-y;

			unsigned char c=104;
			write(tty_fd,&c,1);


//			UART0_Sendchar(104);
			
			int x2 = x % 8;
			int y2 = y % 8;
			int mod = (x-x2)/8 + ((y-y2)/8*9);
			
//			UART0_Sendchar(mod);
			c=mod;
			write(tty_fd,&c,1);
//			UART0_Sendchar(x2);
			c=x2;
			write(tty_fd,&c,1);
//			UART0_Sendchar(y2);
			c=y2;
			write(tty_fd,&c,1);
//			UART0_Sendchar(color);
			c=color;
			write(tty_fd,&c,1);
			usleep(200);

}
        
// found it in the internet...
static unsigned int my_rand(void) {
    static unsigned int z1 = 12345, z2 = 12345, z3 = 12345, z4 = 12345;
    unsigned int b;
    b  = ((z1 << 6) ^ z1) >> 13;
    z1 = ((z1 & 4294967294U) << 18) ^ b;
    b  = ((z2 << 2) ^ z2) >> 27;
    z2 = ((z2 & 4294967288U) << 2) ^ b;
    b  = ((z3 << 13) ^ z3) >> 21;
    z3 = ((z3 & 4294967280U) << 7) ^ b;
    b  = ((z4 << 3) ^ z4) >> 12;
    z4 = ((z4 & 4294967168U) << 13) ^ b;
    return (z1 ^ z2 ^ z3 ^ z4);
}

unsigned int rand_int(unsigned int limit) {
    return my_rand() % limit;
}
