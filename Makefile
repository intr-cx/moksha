NAME = moksha
CC   = gcc

CFLAGS  = -Wall -Wextra -Wpedantic -Iinc
LDFLAGS = -L/usr/lib -lm -lncurses

SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)

.PHONY: clean test release debug all

all: release

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

build: $(OBJ)
	$(shell mkdir -p bin)
	$(CC) -o bin/$(NAME) $(OBJ) $(DBG) $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(OBJ)

debug: DBG = -O0 -g -ggdb -DDEBUG
debug: clean build

release: DBG = -O3
release: clean build
