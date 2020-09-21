all: exec

CC = g++
INCLUDE = .
CFLAGS = -g -Wall -ansi
MEMORY_SIZE = 400M
DATA_PATH = NDK_0.txt
 
exec: main.o bpt.o pcm.o
	$(CC) -o exec main.o bpt.o pcm.o

main.o: main.cpp bpt.h pcm.h
	$(CC) -I$(INCLUDE) $(CFLAGS) -c main.cpp

bpt.o: bpt.cpp bpt.h 
	$(CC) -I$(INCLUDE) $(CFLAGS) -c bpt.cpp

pcm.o: pcm.cpp pcm.h
	$(CC) -I$(INCLUDE) $(CFLAGS) -c pcm.cpp

clean:
	rm -rf exec main.o bpt.o pcm.o

run: ./exec
	./exec $(MEMORY_SIZE) $(DATA_PATH)

