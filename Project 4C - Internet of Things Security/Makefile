#NAME: Adam Cole
#EMAIL: adam.cole5621@gmail.com
#ID: 004912373

CC = gcc
CFLAGS = -Wall -Wextra -g -lmraa -lm
files = 4c_tcp.c 4c_tls.c README Makefile

.SILENT:

default:
	$(CC) $(CFLAGS) 4c_tcp.c -o lab4c_tcp
	$(CC) $(CFLAGS) -lssl -lcrypto 4c_tls.c -o lab4c_tls

clean:
	rm -f lab4c_tcp lab4c_tls *.tar.gz

dist:
	tar -czvf lab4c-004912373.tar.gz $(files)
