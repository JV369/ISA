ALL: feedreader

feedreader:
	gcc -g -std=gnu99 -Wall -pedantic `xml2-config --cflags` -o $(@) queue.c argChecker.c main.c feeder.c -lpthread -lcrypto -lssl -I/usr/include/libxml2 -lxml2 `xml2-config --libs`
zip:
	zip xvavra20.zip main.c queue.h queue.c Makefile

clean:
	rm -f feedreader

