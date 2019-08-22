#NAME: Adam Cole, Jake Wallin
#EMAIL: adam.cole5621@gmail.com, jakewallin82@gmail.com
#ID: 004912373, 404979154

CC = gcc
CLFAGS = -g -Wall -Wextra
files = Makefile README lab3a.c ext2_fs.h

.SILENT:

default:
	$(CC) $(CFLAGS) lab3a.c -o lab3a

clean:
	rm -f lab3a *.tar.gz

dist:	clean default
	tar -cvzf lab3a-004912373.tar.gz $(files)
