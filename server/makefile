CFLAGS= -std=c++11 
all: bigboss

bigboss: main.o server.o desk.o utils.o datamanager.o
	g++ -L/lib/arm-linux-gnueabihf -lwiringPi -lpigpio -lrt -lboost_system -lpthread -o bigboss main.o server.o desk.o utils.o datamanager.o

main.o: main.cpp server.hpp
	g++ -c $(CFLAGS) main.cpp -lpthread -lboost_system -g 

server.o: server.hpp server.cpp datamanager.hpp utils.hpp
	g++ -c $(CFLAGS) server.cpp -lpthread -lboost_system -g 

datamanager.o: datamanager.hpp datamanager.cpp desk.hpp defs.h
	g++ -c $(CFLAGS) datamanager.cpp -lpthread -lboost_system -lpigpio -lrt -g -lwiringPi

desk.o: desk.hpp desk.hpp
	g++ -c $(CFLAGS) desk.cpp -lpthread -lboost_system -g

utils.o: utils.cpp utils.hpp
	g++ -c $(CFLAGS) utils.cpp

clean:
	rm -f *.o &&\
	rm -f bigboss
