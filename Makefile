#NAME: Adam Cole
#EMAIL: adam.cole5621@gmail.com
#ID: 004912373

CC = gcc
CFLAGS = -g -Wall -Wextra -pthread

sourcefiles = lab2_add.c lab2_list.c SortedList.h SortedList.c

pictures = lab2_add-1.png lab2_add-2.png lab2_add-3.png lab2_add-4.png lab2_add-5.png lab2_list-1.png lab2_list-2.png lab2_list-3.png lab2_list-4.png lab2_list.gp lab2_add.gp

files = $(sourcefiles) $(pictures) lab2_add.csv lab2_list.csv Makefile README

.SILENT:

build:
	$(CC) $(CFLAGS) lab2_add.c -o lab2_add
	$(CC) $(CFLAGS) SortedList.c lab2_list.c -o lab2_list

graphs: tests
	gnuplot lab2_add.gp
	gnuplot lab2_list.gp

tests: clean build test1 test2

test1:
#test lab2_add and store results in lab2_add.csv
	./lab2_add --threads=2  --iterations=100    >> lab2_add.csv
	./lab2_add --threads=2  --iterations=1000   >> lab2_add.csv
	./lab2_add --threads=2  --iterations=10000  >> lab2_add.csv
	./lab2_add --threads=2  --iterations=100000 >> lab2_add.csv
	./lab2_add --threads=4  --iterations=100    >> lab2_add.csv
	./lab2_add --threads=4  --iterations=1000   >> lab2_add.csv
	./lab2_add --threads=4  --iterations=10000  >> lab2_add.csv
	./lab2_add --threads=4  --iterations=100000 >> lab2_add.csv
	./lab2_add --threads=8  --iterations=100    >> lab2_add.csv
	./lab2_add --threads=8  --iterations=1000   >> lab2_add.csv
	./lab2_add --threads=8  --iterations=10000  >> lab2_add.csv
	./lab2_add --threads=8  --iterations=100000 >> lab2_add.csv
	./lab2_add --threads=12 --iterations=100    >> lab2_add.csv
	./lab2_add --threads=12 --iterations=1000   >> lab2_add.csv
	./lab2_add --threads=12 --iterations=10000  >> lab2_add.csv
	./lab2_add --threads=12 --iterations=100000  >> lab2_add.csv

	./lab2_add --threads=2  --iterations=10     --yield >> lab2_add.csv
	./lab2_add --threads=4  --iterations=10     --yield >> lab2_add.csv
	./lab2_add --threads=8  --iterations=10     --yield >> lab2_add.csv
	./lab2_add --threads=12 --iterations=10     --yield >> lab2_add.csv
	./lab2_add --threads=2  --iterations=20     --yield >> lab2_add.csv
	./lab2_add --threads=4  --iterations=20     --yield >> lab2_add.csv
	./lab2_add --threads=8  --iterations=20     --yield >> lab2_add.csv
	./lab2_add --threads=12 --iterations=20     --yield >> lab2_add.csv
	./lab2_add --threads=2  --iterations=40     --yield >> lab2_add.csv
	./lab2_add --threads=4  --iterations=40     --yield >> lab2_add.csv
	./lab2_add --threads=8  --iterations=40     --yield >> lab2_add.csv
	./lab2_add --threads=12 --iterations=40     --yield >> lab2_add.csv
	./lab2_add --threads=2  --iterations=80     --yield >> lab2_add.csv
	./lab2_add --threads=4  --iterations=80     --yield >> lab2_add.csv
	./lab2_add --threads=8  --iterations=80     --yield >> lab2_add.csv
	./lab2_add --threads=12 --iterations=80     --yield >> lab2_add.csv
	./lab2_add --threads=2  --iterations=100    --yield >> lab2_add.csv
	./lab2_add --threads=4  --iterations=100    --yield >> lab2_add.csv
	./lab2_add --threads=8  --iterations=100    --yield >> lab2_add.csv
	./lab2_add --threads=12 --iterations=100    --yield >> lab2_add.csv
	./lab2_add --threads=2  --iterations=1000   --yield >> lab2_add.csv
	./lab2_add --threads=4  --iterations=1000   --yield >> lab2_add.csv
	./lab2_add --threads=8  --iterations=1000   --yield >> lab2_add.csv
	./lab2_add --threads=12 --iterations=1000   --yield >> lab2_add.csv
	./lab2_add --threads=2  --iterations=10000  --yield >> lab2_add.csv
	./lab2_add --threads=4  --iterations=10000  --yield >> lab2_add.csv
	./lab2_add --threads=8  --iterations=10000  --yield >> lab2_add.csv
	./lab2_add --threads=12 --iterations=10000  --yield >> lab2_add.csv
	./lab2_add --threads=2  --iterations=100000 --yield >> lab2_add.csv
	./lab2_add --threads=4  --iterations=100000 --yield >> lab2_add.csv
	./lab2_add --threads=8  --iterations=100000 --yield >> lab2_add.csv
	./lab2_add --threads=12 --iterations=100000 --yield >> lab2_add.csv

	./lab2_add --threads=1  --iterations=100         >> lab2_add.csv
	./lab2_add --threads=1  --iterations=1000        >> lab2_add.csv
	./lab2_add --threads=1  --iterations=10000       >> lab2_add.csv
	./lab2_add --threads=1  --iterations=100000      >> lab2_add.csv
	./lab2_add --threads=1  --iterations=1000000     >> lab2_add.csv
	./lab2_add --threads=2  --iterations=100         >> lab2_add.csv
	./lab2_add --threads=2  --iterations=1000        >> lab2_add.csv
	./lab2_add --threads=2  --iterations=10000       >> lab2_add.csv
	./lab2_add --threads=2  --iterations=100000      >> lab2_add.csv
	./lab2_add --threads=2  --iterations=1000000     >> lab2_add.csv
	./lab2_add --threads=4  --iterations=100         >> lab2_add.csv
	./lab2_add --threads=4  --iterations=1000        >> lab2_add.csv
	./lab2_add --threads=4  --iterations=10000       >> lab2_add.csv
	./lab2_add --threads=4  --iterations=100000      >> lab2_add.csv
	./lab2_add --threads=4  --iterations=1000000     >> lab2_add.csv
	./lab2_add --threads=1  --iterations=100     --yield >> lab2_add.csv
	./lab2_add --threads=1  --iterations=1000    --yield >> lab2_add.csv
	./lab2_add --threads=1  --iterations=10000   --yield >> lab2_add.csv
	./lab2_add --threads=1  --iterations=100000  --yield >> lab2_add.csv
	./lab2_add --threads=1  --iterations=1000000 --yield >> lab2_add.csv
	./lab2_add --threads=2  --iterations=100     --yield >> lab2_add.csv
	./lab2_add --threads=2  --iterations=1000    --yield >> lab2_add.csv
	./lab2_add --threads=2  --iterations=10000   --yield >> lab2_add.csv
	./lab2_add --threads=2  --iterations=100000  --yield >> lab2_add.csv
	./lab2_add --threads=2  --iterations=1000000 --yield >> lab2_add.csv
	./lab2_add --threads=4  --iterations=100     --yield >> lab2_add.csv
	./lab2_add --threads=4  --iterations=1000    --yield >> lab2_add.csv
	./lab2_add --threads=4  --iterations=10000   --yield >> lab2_add.csv
	./lab2_add --threads=4  --iterations=100000  --yield >> lab2_add.csv
	./lab2_add --threads=4  --iterations=1000000 --yield >> lab2_add.csv

	./lab2_add --threads=2  --iterations=10000 --yield --sync=m >> lab2_add.csv
	./lab2_add --threads=4  --iterations=10000 --yield --sync=m >> lab2_add.csv
	./lab2_add --threads=8  --iterations=10000 --yield --sync=m >> lab2_add.csv
	./lab2_add --threads=12 --iterations=10000 --yield --sync=m >> lab2_add.csv
	./lab2_add --threads=2  --iterations=10000 --yield --sync=c >> lab2_add.csv
	./lab2_add --threads=4  --iterations=10000 --yield --sync=c >> lab2_add.csv
	./lab2_add --threads=8  --iterations=10000 --yield --sync=c >> lab2_add.csv
	./lab2_add --threads=12 --iterations=10000 --yield --sync=c >> lab2_add.csv
	./lab2_add --threads=2  --iterations=10000 --yield --sync=s >> lab2_add.csv
	./lab2_add --threads=4  --iterations=10000 --yield --sync=s >> lab2_add.csv
	./lab2_add --threads=8  --iterations=1000  --yield --sync=s >> lab2_add.csv
	./lab2_add --threads=12 --iterations=1000  --yield --sync=s >> lab2_add.csv

	./lab2_add --threads=1  --iterations=10000 --sync=m >> lab2_add.csv
	./lab2_add --threads=2  --iterations=10000 --sync=m >> lab2_add.csv
	./lab2_add --threads=4  --iterations=10000 --sync=m >> lab2_add.csv
	./lab2_add --threads=8  --iterations=10000 --sync=m >> lab2_add.csv
	./lab2_add --threads=12 --iterations=10000 --sync=m >> lab2_add.csv
	./lab2_add --threads=1  --iterations=10000 --sync=c >> lab2_add.csv
	./lab2_add --threads=2  --iterations=10000 --sync=c >> lab2_add.csv
	./lab2_add --threads=4  --iterations=10000 --sync=c >> lab2_add.csv
	./lab2_add --threads=8  --iterations=10000 --sync=c >> lab2_add.csv
	./lab2_add --threads=12 --iterations=10000 --sync=c >> lab2_add.csv
	./lab2_add --threads=1  --iterations=10000 --sync=s >> lab2_add.csv
	./lab2_add --threads=2  --iterations=10000 --sync=s >> lab2_add.csv
	./lab2_add --threads=4  --iterations=10000 --sync=s >> lab2_add.csv
	./lab2_add --threads=8  --iterations=10000 --sync=s >> lab2_add.csv
	./lab2_add --threads=12 --iterations=10000 --sync=s >> lab2_add.csv

test2:
#test lab2_list and store results in lab2_list.csv
	./lab2_list --threads=1  --iterations=10	      >> lab2_list.csv
	./lab2_list --threads=1  --iterations=100	      >> lab2_list.csv
	./lab2_list --threads=1  --iterations=1000	      >> lab2_list.csv
	./lab2_list --threads=1  --iterations=10000	      >> lab2_list.csv
	./lab2_list --threads=1  --iterations=20000	      >> lab2_list.csv

	-@./lab2_list --threads=2  --iterations=1              >> lab2_list.csv
	-@./lab2_list --threads=2  --iterations=10             >> lab2_list.csv
	-@./lab2_list --threads=2  --iterations=100            >> lab2_list.csv
	-@./lab2_list --threads=2  --iterations=1000           >> lab2_list.csv
	-@./lab2_list --threads=4  --iterations=1              >> lab2_list.csv
	-@./lab2_list --threads=4  --iterations=10             >> lab2_list.csv
	-@./lab2_list --threads=4  --iterations=100            >> lab2_list.csv
	-@./lab2_list --threads=4  --iterations=1000           >> lab2_list.csv
	-@./lab2_list --threads=8  --iterations=1              >> lab2_list.csv
	-@./lab2_list --threads=8  --iterations=10             >> lab2_list.csv
	-@./lab2_list --threads=8  --iterations=100            >> lab2_list.csv
	-@./lab2_list --threads=8  --iterations=1000           >> lab2_list.csv
	-@./lab2_list --threads=12 --iterations=1              >> lab2_list.csv
	-@./lab2_list --threads=12 --iterations=10             >> lab2_list.csv
	-@./lab2_list --threads=12 --iterations=100            >> lab2_list.csv
	-@./lab2_list --threads=12 --iterations=1000           >> lab2_list.csv

	-@./lab2_list --threads=2  --iterations=1   --yield=i  >> lab2_list.csv
	-@./lab2_list --threads=2  --iterations=2   --yield=i  >> lab2_list.csv
	-@./lab2_list --threads=2  --iterations=4   --yield=i  >> lab2_list.csv
	-@./lab2_list --threads=2  --iterations=8   --yield=i  >> lab2_list.csv
	-@./lab2_list --threads=2  --iterations=16  --yield=i  >> lab2_list.csv
	-@./lab2_list --threads=2  --iterations=32  --yield=i  >> lab2_list.csv
	-@./lab2_list --threads=4  --iterations=1   --yield=i  >> lab2_list.csv
	-@./lab2_list --threads=4  --iterations=2   --yield=i  >> lab2_list.csv
	-@./lab2_list --threads=4  --iterations=4   --yield=i  >> lab2_list.csv
	-@./lab2_list --threads=4  --iterations=8   --yield=i  >> lab2_list.csv
	-@./lab2_list --threads=4  --iterations=16  --yield=i  >> lab2_list.csv
	-@./lab2_list --threads=8  --iterations=1   --yield=i  >> lab2_list.csv
	-@./lab2_list --threads=8  --iterations=2   --yield=i  >> lab2_list.csv
	-@./lab2_list --threads=8  --iterations=4   --yield=i  >> lab2_list.csv
	-@./lab2_list --threads=8  --iterations=8   --yield=i  >> lab2_list.csv
	-@./lab2_list --threads=8  --iterations=16  --yield=i  >> lab2_list.csv
	-@./lab2_list --threads=12 --iterations=1   --yield=i  >> lab2_list.csv
	-@./lab2_list --threads=12 --iterations=2   --yield=i  >> lab2_list.csv
	-@./lab2_list --threads=12 --iterations=4   --yield=i  >> lab2_list.csv
	-@./lab2_list --threads=12 --iterations=8   --yield=i  >> lab2_list.csv
	-@./lab2_list --threads=12 --iterations=16  --yield=i  >> lab2_list.csv

	-@./lab2_list --threads=2  --iterations=1   --yield=d  >> lab2_list.csv
	-@./lab2_list --threads=2  --iterations=2   --yield=d  >> lab2_list.csv
	-@./lab2_list --threads=2  --iterations=4   --yield=d  >> lab2_list.csv
	-@./lab2_list --threads=2  --iterations=8   --yield=d  >> lab2_list.csv
	-@./lab2_list --threads=2  --iterations=16  --yield=d  >> lab2_list.csv
	-@./lab2_list --threads=2  --iterations=32  --yield=d  >> lab2_list.csv
	-@./lab2_list --threads=4  --iterations=1   --yield=d  >> lab2_list.csv
	-@./lab2_list --threads=4  --iterations=2   --yield=d  >> lab2_list.csv
	-@./lab2_list --threads=4  --iterations=4   --yield=d  >> lab2_list.csv
	-@./lab2_list --threads=4  --iterations=8   --yield=d  >> lab2_list.csv
	-@./lab2_list --threads=4  --iterations=16  --yield=d  >> lab2_list.csv
	-@./lab2_list --threads=8  --iterations=1   --yield=d  >> lab2_list.csv
	-@./lab2_list --threads=8  --iterations=2   --yield=d  >> lab2_list.csv
	-@./lab2_list --threads=8  --iterations=4   --yield=d  >> lab2_list.csv
	-@./lab2_list --threads=8  --iterations=8   --yield=d  >> lab2_list.csv
	-@./lab2_list --threads=8  --iterations=16  --yield=d  >> lab2_list.csv
	-@./lab2_list --threads=12 --iterations=1   --yield=d  >> lab2_list.csv
	-@./lab2_list --threads=12 --iterations=2   --yield=d  >> lab2_list.csv
	-@./lab2_list --threads=12 --iterations=4   --yield=d  >> lab2_list.csv
	-@./lab2_list --threads=12 --iterations=8   --yield=d  >> lab2_list.csv
	-@./lab2_list --threads=12 --iterations=16  --yield=d  >> lab2_list.csv

	-@./lab2_list --threads=2  --iterations=1   --yield=il >> lab2_list.csv
	-@./lab2_list --threads=2  --iterations=2   --yield=il >> lab2_list.csv
	-@./lab2_list --threads=2  --iterations=4   --yield=il >> lab2_list.csv
	-@./lab2_list --threads=2  --iterations=8   --yield=il >> lab2_list.csv
	-@./lab2_list --threads=2  --iterations=16  --yield=il >> lab2_list.csv
	-@./lab2_list --threads=2  --iterations=32  --yield=il >> lab2_list.csv
	-@./lab2_list --threads=4  --iterations=1   --yield=il >> lab2_list.csv
	-@./lab2_list --threads=4  --iterations=2   --yield=il >> lab2_list.csv
	-@./lab2_list --threads=4  --iterations=4   --yield=il >> lab2_list.csv
	-@./lab2_list --threads=4  --iterations=8   --yield=il >> lab2_list.csv
	-@./lab2_list --threads=4  --iterations=16  --yield=il >> lab2_list.csv
	-@./lab2_list --threads=8  --iterations=1   --yield=il >> lab2_list.csv
	-@./lab2_list --threads=8  --iterations=2   --yield=il >> lab2_list.csv
	-@./lab2_list --threads=8  --iterations=4   --yield=il >> lab2_list.csv
	-@./lab2_list --threads=8  --iterations=8   --yield=il >> lab2_list.csv
	-@./lab2_list --threads=8  --iterations=16  --yield=il >> lab2_list.csv
	-@./lab2_list --threads=12 --iterations=1   --yield=il >> lab2_list.csv
	-@./lab2_list --threads=12 --iterations=2   --yield=il >> lab2_list.csv
	-@./lab2_list --threads=12 --iterations=4   --yield=il >> lab2_list.csv
	-@./lab2_list --threads=12 --iterations=8   --yield=il >> lab2_list.csv
	-@./lab2_list --threads=12 --iterations=16  --yield=il >> lab2_list.csv

	-@./lab2_list --threads=2  --iterations=1   --yield=dl >> lab2_list.csv
	-@./lab2_list --threads=2  --iterations=2   --yield=dl >> lab2_list.csv
	-@./lab2_list --threads=2  --iterations=4   --yield=dl >> lab2_list.csv
	-@./lab2_list --threads=2  --iterations=8   --yield=dl >> lab2_list.csv
	-@./lab2_list --threads=2  --iterations=16  --yield=dl >> lab2_list.csv
	-@./lab2_list --threads=2  --iterations=32  --yield=dl >> lab2_list.csv
	-@./lab2_list --threads=4  --iterations=1   --yield=dl >> lab2_list.csv
	-@./lab2_list --threads=4  --iterations=2   --yield=dl >> lab2_list.csv
	-@./lab2_list --threads=4  --iterations=4   --yield=dl >> lab2_list.csv
	-@./lab2_list --threads=4  --iterations=8   --yield=dl >> lab2_list.csv
	-@./lab2_list --threads=4  --iterations=16  --yield=dl >> lab2_list.csv
	-@./lab2_list --threads=8  --iterations=1   --yield=dl >> lab2_list.csv
	-@./lab2_list --threads=8  --iterations=2   --yield=dl >> lab2_list.csv
	-@./lab2_list --threads=8  --iterations=4   --yield=dl >> lab2_list.csv
	-@./lab2_list --threads=8  --iterations=8   --yield=dl >> lab2_list.csv
	-@./lab2_list --threads=8  --iterations=16  --yield=dl >> lab2_list.csv
	-@./lab2_list --threads=12 --iterations=1   --yield=dl >> lab2_list.csv
	-@./lab2_list --threads=12 --iterations=2   --yield=dl >> lab2_list.csv
	-@./lab2_list --threads=12 --iterations=4   --yield=dl >> lab2_list.csv
	-@./lab2_list --threads=12 --iterations=8   --yield=dl >> lab2_list.csv
	-@./lab2_list --threads=12 --iterations=16  --yield=dl >> lab2_list.csv

	./lab2_list --threads=12 --iterations=32 --yield=i  --sync=m >> lab2_list.csv
	./lab2_list --threads=12 --iterations=32 --yield=d  --sync=m >> lab2_list.csv
	./lab2_list --threads=12 --iterations=32 --yield=il --sync=m >> lab2_list.csv
	./lab2_list --threads=12 --iterations=32 --yield=dl --sync=m >> lab2_list.csv
	./lab2_list --threads=12 --iterations=32 --yield=i  --sync=s >> lab2_list.csv
	./lab2_list --threads=12 --iterations=32 --yield=d  --sync=s >> lab2_list.csv
	./lab2_list --threads=12 --iterations=32 --yield=il --sync=s >> lab2_list.csv
	./lab2_list --threads=12 --iterations=32 --yield=dl --sync=s >> lab2_list.csv

	./lab2_list --threads=1  --iterations=1000          >> lab2_list.csv
	./lab2_list --threads=1  --iterations=1000 --sync=m >> lab2_list.csv
	./lab2_list --threads=2  --iterations=1000 --sync=m >> lab2_list.csv
	./lab2_list --threads=4  --iterations=1000 --sync=m >> lab2_list.csv
	./lab2_list --threads=8  --iterations=1000 --sync=m >> lab2_list.csv
	./lab2_list --threads=12 --iterations=1000 --sync=m >> lab2_list.csv
	./lab2_list --threads=16 --iterations=1000 --sync=m >> lab2_list.csv
	./lab2_list --threads=24 --iterations=1000 --sync=m >> lab2_list.csv
	./lab2_list --threads=1  --iterations=1000 --sync=s >> lab2_list.csv
	./lab2_list --threads=2  --iterations=1000 --sync=s >> lab2_list.csv
	./lab2_list --threads=4  --iterations=1000 --sync=s >> lab2_list.csv
	./lab2_list --threads=8  --iterations=1000 --sync=s >> lab2_list.csv
	./lab2_list --threads=12 --iterations=1000 --sync=s >> lab2_list.csv
	./lab2_list --threads=16 --iterations=1000 --sync=s >> lab2_list.csv
	./lab2_list --threads=24 --iterations=1000 --sync=s >> lab2_list.csv

dist:	graphs
	tar -cvzf lab2a-004912373.tar.gz $(files)

clean:
	rm -f *.o *.txt *.tar.gz lab2_add lab2_list
