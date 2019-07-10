#NAME: Adam Cole
#EMAIL: adam.cole5621@gmail.com
#ID: 004912373

CC = gcc
CFLAGS = -Wall -Wextra -g
files = lab1a.c Makefile README

.SILENT:

all:
	$(CC) $(CFLAGS) -o lab1a lab1a.c
clean:
	rm -f *.o *.txt *.tar.gz lab1a
dist:
	tar -czvf lab1a-004912373.tar.gz $(files)
