CC=gcc
CFLAGS= -Wall -g -pthread
NAME_C= client
NAME_S = server
RES= game.c net.c ui.c
HEADER = header.h

all: $(HEADER) $(RES) $(NAME_C).c $(NAME_S).c
	$(CC) $(NAME_C).c $(RES) -o $(NAME_C) $(CFLAGS)
	$(CC) $(NAME_S).c -o $(NAME_S) $(CFLAGS)

clean:
	rm -rf *.o $(NAME_C) $(NAME_S)
 

