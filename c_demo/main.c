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
 
        tty_fd=open("/dev/ttyACM1", O_RDWR | O_NONBLOCK);      
        cfsetospeed(&tio,B500000);            // 115200 baud
        cfsetispeed(&tio,B500000);            // 115200 baud
 
        tcsetattr(tty_fd,TCSANOW,&tio);

		char cmd1[] = {92,54,32,92,48}; 
		write(tty_fd,&cmd1,5);


		char cmd2[] = {92,52,85,195,210,40,195,92,48}; 
		write(tty_fd,&cmd2,9);

		char cmd3[] = {92,51,82,69,77,48,84,92,48}; 
		write(tty_fd,&cmd3,9);

		char cmd4[] = {92,53,81,92,48}; 
		write(tty_fd,&cmd4,5);


		while(1)
		{


			char cmd5[] = {92,49,32,71,65,0,0,0,0,1,0,0,0,85,195,210,40,195,83,195,210,0,6,16,116,101,116,114,105,86,73,201,233,92,48}; 
			write(tty_fd,&cmd5,36);
			usleep(1000000);
		}
}