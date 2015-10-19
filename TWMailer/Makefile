#Makefile
all: server client clean_o

server: main.o fileOperations.o
	g++ -std=c++11 main.o fileOperations.o -o server

main.o: main.cpp fileOperations.h
	g++ -Wall -g -std=c++11 -c main.cpp -o main.o

fileOperations.o: fileOperations.cpp fileOperations.h
	g++ -Wall -g -std=c++11 -c fileOperations.cpp -o fileOperaions.h


client: client.o
	g++ -Wall -g -std=c++11 client.o -o client

client.o: client.cpp
	g++ -Wall -g -std=c++11 -c client.cpp -o client.o

clean_o:
	rm -f *.o

clean:
	rm -f *.o server client
