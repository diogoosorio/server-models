CC      = gcc
CFLAGS  = -Wall -Wextra -ansi -pedantic -std=c99

all: server

server: server.c $(wildcard ../lib/*.c)
	$(CC) $(CFLAGS) -o server $^

clean:
	@rm -rf *.o
