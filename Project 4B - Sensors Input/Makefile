#NAME: Adam Cole
#EMAIL: ############@gmail.com
#ID: #########

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
	rm -f lab4b *.tar.gz

dist:
	tar -czvf lab4b-#########.tar.gz $(files)
