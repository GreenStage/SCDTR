all: bigboss

bigboss: main.o server.o desk.o utils.o datamanager.o
	g++ -o bigboss main.o server.o desk.o utils.o datamanager.o

main.o: main.cpp main.hpp server.hpp
	g++ -o main.o main.cpp

server.o: server.hpp server.cpp datamanager.hpp
	g++ -o server.o server.hpp

datamanager.o: datamanager.hpp datamanager.cpp desk.hpp defs.h
	g++ -o datamanager.o datamanager.cpp

desk.o: desk.hpp desk.hpp
	g++ -o desk.o desk.cpp

