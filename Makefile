CC=gcc
CFLAGS= -Wall -g -pthread
NAME_C= client
NAME_S = server


all: 
	$(CC) $(NAME_C).c -o $(NAME_C) $(CFLAGS)
	$(CC) $(NAME_S).c -o $(NAME_S) $(CFLAGS)

clean:
	rm -rf *.o $(NAME_C) $(NAME_S)
 

