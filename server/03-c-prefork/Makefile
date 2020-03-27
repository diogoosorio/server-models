CC      = gcc
CFLAGS  = -Wall -Wextra -ansi -pedantic -std=c99

all: server

server: server.c
	$(CC) $(CFLAGS) -o $@.o server.c

clean:
	@rm -rf *.o
