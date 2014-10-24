CFLAGS = -Wall

all: mastermind.c logging.c
	gcc $(CFLAGS) -o mastermind mastermind.c logging.c
