#NAME: Adam Cole, Jake Wallin
#EMAIL: ############@gmail.com, ##########@gmail.com
#ID: #########, #########

CC = gcc
CLFAGS = -g -Wall -Wextra
files = Makefile README lab3a.c ext2_fs.h

.SILENT:

default:
	$(CC) $(CFLAGS) lab3a.c -o lab3a

clean:
	rm -f lab3a *.tar.gz

dist:	clean default
	tar -cvzf lab3a-#########.tar.gz $(files)
