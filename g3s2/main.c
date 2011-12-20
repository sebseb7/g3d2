#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <SDL/SDL.h>
#include "SDL/SDL_thread.h"
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#if defined(MAC_OS_X_VERSION_10_4) && (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_4)
#include <sys/ioctl.h>
#include <IOKit/serial/ioss.h>
#include <errno.h>
#endif

#include "main.h"
#include "sdl_draw/SDL_draw.h"

static int usbThread(void *nothing);
static SDL_mutex *eventLock = NULL;
static SDL_mutex *displayLock = NULL;


enum { ZOOM = 14 };

static unsigned char display[DISPLAY_HEIGHT][DISPLAY_WIDTH];
static unsigned char display2[DISPLAY_HEIGHT][DISPLAY_WIDTH];

 
int main(int argc,char** argv)
{
	srand(SDL_GetTicks());
	SDL_Surface* screen = SDL_SetVideoMode(
		DISPLAY_WIDTH * ZOOM,
		DISPLAY_HEIGHT * ZOOM,
		32, SDL_SWSURFACE | SDL_DOUBLEBUF);

	const unsigned int COLORS[] = {
		SDL_MapRGB(screen->format, 0x00,0x10,0x00),
		SDL_MapRGB(screen->format, 0x00,0x20,0x00),
		SDL_MapRGB(screen->format, 0x00,0x30,0x00),
		SDL_MapRGB(screen->format, 0x00,0x40,0x00),
		SDL_MapRGB(screen->format, 0x00,0x50,0x00),
		SDL_MapRGB(screen->format, 0x00,0x60,0x00),
		SDL_MapRGB(screen->format, 0x00,0x70,0x00),
		SDL_MapRGB(screen->format, 0x00,0x80,0x00),
		SDL_MapRGB(screen->format, 0x00,0x90,0x00),
		SDL_MapRGB(screen->format, 0x00,0xa0,0x00),
		SDL_MapRGB(screen->format, 0x00,0xb0,0x00),
		SDL_MapRGB(screen->format, 0x00,0xc0,0x00),
		SDL_MapRGB(screen->format, 0x00,0xd0,0x00),
		SDL_MapRGB(screen->format, 0x00,0xe0,0x00),
		SDL_MapRGB(screen->format, 0x00,0xf0,0x00),
		SDL_MapRGB(screen->format, 0x00,0xff,0x00)
	};

	SDL_EnableKeyRepeat(100, 30);	// FIXME: must be deleted
	SDL_Flip(screen);

    SDL_Event ev;


	eventLock = SDL_CreateMutex();
	displayLock = SDL_CreateMutex();
	SDL_Thread *thread = NULL;
  	thread = SDL_CreateThread(usbThread, NULL);
                                    


	int running = 1;

	int rerender = 0;

	while(running) {


		SDL_mutexP(eventLock);
		while(SDL_PollEvent(&ev)) {
                
			switch(ev.type) {
			case SDL_USEREVENT:
				rerender = 1;
				break;
			case SDL_QUIT:
				running = 0;
				break;
            case SDL_KEYUP:
			case SDL_KEYDOWN:
                        
				switch(ev.key.keysym.sym) {
				case SDLK_ESCAPE:
					running = 0;
					break;
				default:
					break;
				}
				break;
			default:
				break;
			}


		}
		SDL_mutexV(eventLock);
                                                                        

		if(rerender)
		{
			rerender=0;
			SDL_mutexP(displayLock);
			for(int x = 0; x < DISPLAY_WIDTH; x++)
				for(int y = 0; y < DISPLAY_HEIGHT; y++)
					if(display[y][x] != display2[y][x]){
						display2[y][x]=display[y][x];
						Draw_FillCircle(screen, ZOOM*x + ZOOM/2, ZOOM*y + ZOOM/2, ZOOM*0.45, COLORS[display[y][x]]);
					}

			SDL_mutexV(displayLock);
			SDL_Flip(screen);
		}
		
	}


	
	SDL_Quit();
	return 0;

}

static int usbThread(void *nothing)
{

	int tty_fd;

        struct termios tio;
 
 
        memset(&tio,0,sizeof(tio));
        tio.c_iflag=0;
        tio.c_oflag=0;
        tio.c_cflag=CS8|CREAD|CLOCAL;           // 8n1, see termios.h for more information
        tio.c_lflag=0;
        tio.c_cc[VMIN]=1;
        tio.c_cc[VTIME]=5;
 
#if defined(MAC_OS_X_VERSION_10_4) && (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_4)
        tty_fd=open("/dev/cu.usbserial-A600dJ57", O_RDWR | O_NONBLOCK);      
		speed_t speed = 500000;
		if ( ioctl( tty_fd,	 IOSSIOSPEED, &speed ) == -1 )
		{
			printf( "Error %d calling ioctl( ..., IOSSIOSPEED, ... )\n", errno );
		}
#else
 //       tty_fd=open("/dev/ttyUSB1", O_RDWR );
        tty_fd=open("/dev/ttyUSB1", O_RDWR | O_NONBLOCK);      
        cfsetospeed(&tio,B500000);
		cfsetispeed(&tio,B500000);
#endif 
        tcsetattr(tty_fd,TCSANOW,&tio);
        
	unsigned char c;

	int state = 0;
	int escape = 0;
            
	int idx = 0;
	int pixel_mod = 0;
	int pixel_x = 0;
	int pixel_y = 0;
                            
	int pixel_x_state = 0;
	int pixel_y_state = 0;
	int mod_state = 0;
	
	
	int rerender = 0;
	int rerender_a = 0;

  	SDL_Event ev;
  
	ev.type = SDL_USEREVENT;
	ev.user.code = 0;
	ev.user.data1 = 0;
	ev.user.data2 = 0;

	while(1)
	{
	
       	while(read(tty_fd,&c,1)>0)
       	{
			switch(c)
			{
				case 0x68:
					// single pixel
					state = 1;
					idx = 0;
					continue;
				case 0x67:
					// full frame
					state = 2;

					mod_state = 0;
					pixel_x_state = 0;
					pixel_y_state = 0;
					continue;
				case  0x65:
					escape = 1;
					continue;
				case 0x66:
					state = 3;
					continue;
			}
			
			if(escape == 1)
			{
				escape = 0;
				
				switch(c)
				{
					case 0x01:
						c = 0x67;
						break;
					case 0x02:
						c = 0x68;
						break;
					case 0x03:
						c = 0x65;
						break;
					case 0x04:
						c = 0x66;
						break;
				}
			}
			
		
			if(state == 1)
			{
				// wait for our pixel
				switch(idx)
				{
					case 0:
						pixel_mod = c;
						break;
					case 1:
						pixel_x = c;
						break;
					case 2:
						pixel_y = c;
						break;
					case 3:
						if((pixel_y < 32)&&(pixel_x<72)&&(c < 16))
						{
							int y1 = 31- (pixel_y+((pixel_mod - (pixel_mod%9))/9*8));
							int x1 = pixel_x+((pixel_mod%9)*8);
							SDL_mutexP(displayLock);
							if(display[y1][x1] != c)
							{
								display[y1][x1]=c;
								rerender=1;
							}
			       			SDL_mutexV(displayLock);
						}
				}
				idx++;
				
			}
			else if(state == 2)
			{


				int y1 = pixel_y_state+((mod_state - (mod_state%9))/9*8);
				int x1 = pixel_x_state+((mod_state%9)*8);

			//	printf("%i:%i %i    %i:%i   %i\n",pixel_y_state,pixel_x_state,mod_state,y1,x1,c);

				SDL_mutexP(displayLock);
				display[y1][x1] = (c & 0x0f);
				display[y1+1][x1] = ((c & 0xf0)>>4);
       			SDL_mutexV(displayLock);

				pixel_y_state+=2;

				if(pixel_y_state == 8)
				{
					pixel_y_state = 0;
					pixel_x_state++;
					if(pixel_x_state == 8)
					{
						pixel_x_state=0;
						mod_state++;
						if(mod_state == 36)
						{
							rerender = 1;
						}
					}
				}
			}
		}

       	if(rerender==1)
       	{
       		int val = 0;
   	
			SDL_mutexP(eventLock);
			val = SDL_PushEvent(&ev);
			if(val != -1)
			{
//				rerender=0;
			}
				rerender=0;
			SDL_mutexV(eventLock);
  	
	    }
	}

	close(tty_fd);

	return 0;
}

