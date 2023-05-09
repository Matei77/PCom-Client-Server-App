CC = g++
CFLAGS = -g -Wall -Wextra -std=c++17

.PHONY: build clean

build: server subscriber server_main.o server.o user.o subscriber_main.o \
	   subscriber.o utils.o


server_main.o: Server/main.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

server.o: Server/server.cpp Server/server.hpp
	$(CC) $(CFLAGS) -o $@ -c $<

user.o: Server/user.cpp Server/user.hpp
	$(CC) $(CFLAGS) -o $@ -c $<


subscriber_main.o: Subscriber/main.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

subscriber.o: Subscriber/subscriber.cpp Subscriber/subscriber.hpp
	$(CC) $(CFLAGS) -o $@ -c $<


utils.o: Utils/utils.cpp Utils/utils.hpp
	$(CC) $(CFLAGS) -o $@ -c $<


server: server_main.o server.o user.o utils.o
	$(CC) $(CFLAGS) -o $@ $^

subscriber: subscriber_main.o subscriber.o utils.o
	$(CC) $(CFLAGS) -o $@ $^


clean:
	-rm -f *.o server subscriber