#NAME: Adam Cole, Jake Wallin
#EMAIL: ############@gmail.com, ############@gmail.com
#ID: #########, #########

SOURCE = lab3b.py
DEST = lab3b

.SILENT:

# creates symbolic link
default: lab3b.py
	rm -rf lab3b
	ln -s $(SOURCE) $(DEST)
	chmod +x $(DEST)

clean:
	rm -f lab3b *.tar.gz *~

dist: 
	tar -cvzf lab3b-#########.tar.gz lab3b.py Makefile README
