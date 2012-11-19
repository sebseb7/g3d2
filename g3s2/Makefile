UNAME := $(shell uname)

SRC = $(wildcard *.c)
SRC+= sdl_draw/SDL_draw.c

FLAGS= -I. -I../firmware --std=gnu99 -Wall  -funsigned-char -Wundef -Wsign-compare -Wunreachable-code  -Wstrict-prototypes  -fsingle-precision-constant 
LDFLAGS=

ifeq ($(UNAME), Darwin)
	FLAGS   += -Qunused-arguments
	FLAGS   += `sdl-config --cflags` 
	LDFLAGS += `sdl-config --libs`  
endif

ifeq ($(UNAME), Linux)
	FLAGS   += `sdl-config --cflags`
	LDFLAGS +=  -lSDL  
endif

ifeq ($(UNAME), MINGW32_NT-6.1)
	FLAGS   += -I/MinGW/msys/1.0/include/ 
	LDFLAGS += -L/MinGW/msys/1.0/lib -lmingw32 -lSDLmain -lSDL
endif

all:
	gcc -o g3s2 $(SRC) $(FLAGS) $(LDFLAGS)

clean:
	rm -f g3s2
