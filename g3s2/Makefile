UNAME := $(shell uname)

SRC = $(wildcard *.c)
SRC+= sdl_draw/SDL_draw.c

FLAGS = -Wall -O2 --std=gnu99

ifeq ($(UNAME), Darwin)
	FLAGS +=  -I/Library/Frameworks/SDL.framework/Headers SDLmain.m -framework SDL -framework Cocoa
endif

ifeq ($(UNAME), Linux)
	FLAGS +=  -lSDL
endif
    

all:
	gcc -o g3s2 $(SRC) $(FLAGS)

clean:
	rm -f g3s2