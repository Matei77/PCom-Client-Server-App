CC = g++
CFLAGS = -Wall -Wextra -std=c++17

.PHONY: build clean

build: server subscriber


server_main.o: src/server/main.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

server.o: src/server/server.cpp src/server/server.hpp
	$(CC) $(CFLAGS) -o $@ -c $<

user.o: src/server/user.cpp src/server/user.hpp
	$(CC) $(CFLAGS) -o $@ -c $<


subscriber_main.o: src/subscriber/main.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

subscriber.o: src/subscriber/subscriber.cpp src/subscriber/subscriber.hpp
	$(CC) $(CFLAGS) -o $@ -c $<


utils.o: src/utils/utils.cpp src/utils/utils.hpp
	$(CC) $(CFLAGS) -o $@ -c $<


server: server_main.o server.o user.o utils.o
	$(CC) $(CFLAGS) -o $@ $^

subscriber: subscriber_main.o subscriber.o utils.o
	$(CC) $(CFLAGS) -o $@ $^


clean:
	-rm -f *.o server subscriber