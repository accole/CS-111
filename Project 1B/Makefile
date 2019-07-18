#NAME: Adam Cole
#EMAIL: ###########@gmail.com
#ID: #########

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
	tar -czvf lab1b-#########.tar.gz $(files)
