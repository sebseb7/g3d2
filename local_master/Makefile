SRC = $(wildcard *.c)
SRC+= pixelfonts/5x3/font.c
SRC+= $(wildcard libs/*.c)

FLAGS = -Wall -O2 --std=gnu99

all:
	gcc -o tetris $(SRC) $(FLAGS)

clean:
	rm -f tetris

