all: glplay

glplay: main.c vector_ops.o matrix_ops.o gl_ops.o
	$(CC) -o glplay main.c vector_ops.o matrix_ops.o gl_ops.o -ggdb --std=gnu99 -Werror -Wall -lm -lSDL2 -lSDL2_image -lGL -lepoxy -I/usr/include/GL -I/usr/include/SDL2 -D_REENTRANT

vector_ops.o: vector_ops.c vector_ops.h
	$(CC) -o vector_ops.o vector_ops.c -c -ggdb --std=gnu99 -Werror -Wall

matrix_ops.o: matrix_ops.c matrix_ops.h
	$(CC) -o matrix_ops.o matrix_ops.c -c -ggdb --std=gnu99 -Werror -Wall

gl_ops.o: gl_ops.c gl_ops.h
	$(CC) -o gl_ops.o gl_ops.c -c -ggdb --std=gnu99 -Werror -Wall -I/usr/include/SDL2 -D_REENTRANT

clean:
	rm -f glplay *.o *~

.PHONY: all clean
