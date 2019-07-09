#NAME: Adam Cole
#EMAIL: ###########@gmail.com
#ID: #########

#use gcc
CC = gcc
#compiling flags
CFLAGS = -g -Wall -Wextra
#include the files in the directory and tar requirements
files = lab0.c Makefile README backtrace.png breakpoint.png 

#will only print out echos
.SILENT:

all:
#compile the executable
	$(CC) $(CFLAGS) -o lab0 lab0.c

check: clean all tests

tests:
#Byte for Byte test
	echo "test data for byte to byte comparison" > bfb.txt; \
	./lab0 -i bfb.txt -o o.txt; \
	cmp bfb.txt o.txt; \
	if [[ $$? -ne 0 ]]; \
	then \
		echo "Byte for Byte test failed.\n" > fail.txt; \
	fi; \
	rm -f bfb.txt o.txt;
#Open test
	echo "this is a file to be deleted" > noread.txt; \
	chmod u-r noread.txt; \
	./lab0 -i noread.txt; \
	if [[ $$? -ne 2 ]]; \
	then \
		echo "Open test failed.\n" > fail.txt; \
	fi; \
	rm -f noread.txt;
#Write test
	echo "this is some input text" > i.txt; \
	touch nowrite.txt; \
	chmod u-w nowrite.txt; \
	./lab0 -i i.txt -o nowrite.txt; \
	if [[ $$? -ne 3 ]]; \
	then \
		echo "Write test failed.\n" > fail.txt; \
	fi; \
	rm -f nowrite.txt i.txt;
#Segmentation test
	./lab0 -s; \
	if [[ $$? -ne 139 ]]; \
	then \
		echo "Segmentation test failed.\n" > fail.txt; \
	fi;
#Catch test
	./lab0 -s -c; \
	if [[ $$? -ne 4 ]]; \
	then \
		echo "Catch test failed.\n" > fail.txt; \
	fi;
#Incorrect argument test
	./lab0 --chipotle; \
	if [[ $$? -ne 1 ]]; \
	then \
		echo "Incorrect argument test failed.\n" > fail.txt; \
	fi;
#See if any tests failed
	if [[ -s fail.txt ]]; \
	then \
		printf "\n\nTests Failed.\n\n"; \
		cat fail.txt; \
		rm -f fail.txt; \
	else \
		printf "\n\nAll tests passed.\n\n"; \
	fi

dist:
#build the distribution tarbell
	tar -cvzf lab0-#########.tar.gz $(files)

clean:
#remove all files created by Makefile
#return directory to untarred state
	rm -f *.o *.txt *.tar.gz lab0
