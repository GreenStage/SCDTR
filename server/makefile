CFLAGS= -std=c++11 
all: bigboss

bigboss: main.o server.o desk.o utils.o datamanager.o
	g++ -o bigboss main.o server.o desk.o utils.o datamanager.o

main.o: main.cpp server.hpp
	g++ -c $(CFLAGS) main.cpp -lpthread -lboost_system

server.o: server.hpp server.cpp datamanager.hpp utils.hpp
	g++ -c $(CFLAGS) server.cpp -lpthread -lboost_system

datamanager.o: datamanager.hpp datamanager.cpp desk.hpp defs.h
	g++ -c $(CFLAGS) datamanager.cpp

desk.o: desk.hpp desk.hpp
	g++ -c $(CFLAGS) desk.cpp

utils.o: utils.cpp utils.hpp
	g++ -c $(CFLAGS) utils.cpp
