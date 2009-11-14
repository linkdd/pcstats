SRC = main.c ram.c cpu.c disk.c battery.c net.c top.c print.c

CC = gcc
CFLAGS = -Wall -g -D _BSD_SOURCE -lncurses 

all: pcstats

pcstats: $(SRC) local.h config.h
	$(CC) $(CFLAGS) $(SRC) -o $@
