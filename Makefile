all: glplay

glplay: main.c
	$(CC) -o glplay main.c -ggdb --std=gnu99 -Werror -Wall -lm -lSDL2 -lSDL2_image -lGL -lepoxy -I/usr/include/GL -I/usr/include/SDL2 -D_REENTRANT

clean:
	rm -f glplay *.o *~

.PHONY: all clean
