CC=gcc
CFLAGS=-I.
DEPS = lab3.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

main: main.o
	$(CC) -o lab3 main.o

clean: 
	rm *.o lab3