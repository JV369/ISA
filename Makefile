.PHONY: all feedreader clean test

all: feedreader

feedreader:
	gcc -std=gnu99 -Wall -pedantic `xml2-config --cflags` -o $(@) queue.c argChecker.c main.c feeder.c connect.c -lpthread -lcrypto -lssl -I/usr/include/libxml2 -lxml2 `xml2-config --libs`
tar:
	tar -cvf xvavra20.tar *.c *.h  Makefile manual.pdf README test.sh test/*

clean:
	rm -f feedreader

test: feedreader
	chmod +x test.sh
	./test.sh
