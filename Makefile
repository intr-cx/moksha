NAME = moksha
CC   = gcc

CFLAGS  = -DDEBUG -g -ggdb -Wall -Wextra -Wpedantic -Iinc
LDFLAGS = -L/usr/lib -lm -lncurses

SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)

.PHONY: clean test all

all: build clean

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

build: $(OBJ)
	$(shell mkdir -p bin)
	$(CC) -o bin/$(NAME) $(OBJ) $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(OBJ)
