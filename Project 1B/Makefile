#NAME: Adam Cole
#EMAIL: adam.cole5621@gmail.com
#ID: 004912373

CC = gcc
CFLAGS = -Wall -Wextra -g -lmcrypt
files = lab1b-client.c lab1b-server.c my.key Makefile README

.SILENT:

all:
	$(CC) $(CFLAGS) -o lab1b-client lab1b-client.c 
	$(CC) $(CFLAGS) -o lab1b-server lab1b-server.c
client:
	$(CC) $(CFLAGS) -o lab1b-client lab1b-client.c
server:
	$(CC) $(CFLAGS) -o lab1b-server lab1b-server.c
clean:
	rm -f *.o *.txt *.tar.gz lab1b-client lab1b-server
dist:
	tar -czvf lab1b-004912373.tar.gz $(files)
