#Makefile
all: server client clean_o

server: server.o fileOperations.o ldap.o
	g++ -std=c++11 server.o fileOperations.o ldap.o -o server -lpthread -DLDAP_DEPRECATED -lldap -llber

server.o: server.cpp fileOperations.h
	g++ -Wall -g -std=c++11 -c server.cpp -o server.o -lpthread -DLDAP_DEPRECATED -lldap -llber

fileOperations.o: fileOperations.cpp fileOperations.h
	g++ -Wall -g -std=c++11 -c fileOperations.cpp -o fileOperations.o 

ldap.o: ldap.cpp ldap.h
	g++ -Wall -g -std=c++11 -c ldap.cpp -o ldap.o -DLDAP_DEPRECATED -lldap -llber


client: client.o
	g++ -Wall -g -std=c++11 client.o -o client

client.o: client.cpp
	g++ -Wall -g -std=c++11 -c client.cpp -o client.o

clean_o:
	rm -f *.o

clean:
	rm -f *.o server client
