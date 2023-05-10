**Name: Ionescu Matei-Ștefan**  
**Group: 323CAb**

# Pcom Homework #2 - Client-Server-App

The application consists of a server and a tcp client. The server can receive
data from both tcp and udp clients.

The tcp clients can subscribe/unsubscribe to/from topics and, when the server
receives a message from a udp client with a topic that the tcp client is
subscribed to, the server will send the message to the tcp client.

## Server
The `Server` class contains the users database, the port the server is opened
on, the file descriptor for the tcp and udp sockets and the vector of poll file
descriptors used for the poll system call.

The most important method in the `Server` class is `RunServer()`. This method
runs the server, monitoring the tcp connection socket, udp connection socket
and stdin fd using the poll system call.


## User
The `User` class represents a tcp client from the server's perspective. It
contains the tcp connection file descriptor, the online status, an unordered
map storing the subscribed topics and their store-and-forward value and a queue
of messages that will be sent when the user reconnects to the server. It also
contanis the following methods:

The `NotifyUser()` method will either send a message to the tcp client or store
the message if the user is subscribed to the topic of the message.

The `ReconnectUser()` method will send all the messages that are are in the
queue.


## Subscriber
The `Subscriber` class contains the id of the client, the server's ip and port,
the tcp socket file descriptor and the vector of poll file descriptors used for
the poll system call.

The most important method in the `Subscriber` class is `RunClient()`. This
method runs the client, monitoring the tcp connection socket and stdin fd using
the poll system call.


## Server-Client Data Transfer Protocol Over TCP
In order to be efficient, messages of variable length will be sent and received.
Both the server and the client will firstly send the length of the data
transmitted, then the content. When receiving, they will get the length of the
data and then the data. This method also prevents concatenating messages that
are sent rapidly one after another.

Sending an empty message will notify the receiving end that the connection
should close.

To save bandwith the tcp client will send subscribe and unsubscribe commands to
the server as strings that have the first byte 'u' for unsubscribe or 's' for
subscribe, then the topic string, then for the subscribe command, the last byte
will be either 1 or 0, representing the store-and-forward status.

For example the client command "subscribe topic_name 1" will be sent to the
server as "stopic_name1".

## Mentions
Buffering is disabled using `setvbuf(stdout, NULL, _IONBF, BUFSIZ);`.

Nagle algorithm is turned off for the server's and client's tcp sockets.

The implementation passed all the tests on the checker.

## References

1. https://pcom.pages.upb.ro/labs/lab7/server.html
2. https://gitlab.cs.pub.ro/pcom/pcom-laboratoare-public/-/blob/master/lab7
3. https://www.ibm.com/docs/en/i/7.4?topic=designs-using-poll-instead-select