CC=gcc
CFLAGS=-I.

encrypt: assignment2/encrypt.c
	$(CC) -o encrypt assignment2/encrypt.c -I.