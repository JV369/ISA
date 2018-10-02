ALL: feedreader

feedreader:
	gcc -g -std=gnu99 -Wall -pedantic -o $(@) queue.c argChecker.c main.c feeder.c -lpthread -lcrypto -lssl

zip:
	zip xvavra20.zip main.c queue.h queue.c Makefile

clean:
	rm -f feedreader

