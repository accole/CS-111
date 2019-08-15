#NAME: Adam Cole
#EMAIL: adam.cole5621@gmail.com
#ID: 004912373

CC = gcc
CFLAGS = -Wall -Wextra -g -lmraa -lm
files = lab4b.c README Makefile

.SILENT:
default:
	 $(CC) $(CFLAGS) lab4b.c -o lab4b

check:
	echo | ./lab4b --lmao &> /dev/null; \
	if [[ $? -ne 1 ]]; \
	then \
		echo "Smoke Check failed\n"; \
	else \
		echo "Smoke Check passed\n"; \
	fi

clean:
	rm -f lab4b lab4b-004912373.tar.gz

dist:
	tar -czvf lab4b-004912373.tar.gz $(files)
