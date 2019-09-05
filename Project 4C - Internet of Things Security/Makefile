#NAME: Adam Cole
#EMAIL: ############@gmail.com
#ID: #########

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
	tar -czvf lab4c-#########.tar.gz $(files)
