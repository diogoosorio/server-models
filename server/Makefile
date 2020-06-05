CC      = gcc
CFLAGS  = -Wall -Wextra -ansi -pedantic -std=c99 -g
SERVERS := $(wildcard src/*.c)

all: $(SERVERS)

$(SERVERS):
	-$(CC) $(CFLAGS) -o ./bin/$(basename $(notdir $@)) $@ $(wildcard ./src/lib/*.c)

clean:
	@rm -rf bin/*

.PHONY: all $(SERVERS)
