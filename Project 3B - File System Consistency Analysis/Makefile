#NAME: Adam Cole, Jake Wallin
#EMAIL: adam.cole5621@gmail.com, jakewallin82@gmail.com
#ID: 004912373, 404979154

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
	tar -cvzf lab3b-004912373.tar.gz lab3b.py Makefile README
