// Copyright Ionescu Matei-Stefan - 323CAb - 2022-2023
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <queue>
#include <unordered_map>
#include <vector>

#include "../utils/utils.hpp"
#include "server.hpp"
#include "user.hpp"

using namespace std;

void Server::RunServer() {
	int rc;

	InitServer();

	// add the tcp, udp and stdin fd to the poll
	poll_fds.push_back({tcp_socket_fd, POLLIN, 0});
	poll_fds.push_back({udp_socket_fd, POLLIN, 0});
	poll_fds.push_back({STDIN_FILENO, POLLIN, 0});

	while (1) {
		rc = poll(poll_fds.data(), poll_fds.size(), -1);
		DIE(rc < 0, "poll");

		for (auto poll_fd : poll_fds) {
			if (poll_fd.revents & POLLIN) {
				if (poll_fd.fd == tcp_socket_fd) {
					// process the new tcp connection
					ProcessNewTcpConnection();


				} else if (poll_fd.fd == udp_socket_fd) {
					// receive data from udp socket

					// TODO: reveive data

					// TODO: set message

					// TODO: notify users

				} else if (poll_fd.fd == STDIN_FILENO) {
					// process the command given to the server by stdin
					string command;
					getline(cin, command, '\n');

					if (command == EXIT_COMMAND) {
						ExitServer();
						break;

					} else {
						printf("Unrecognized command\n");
					}

				} else {
					// receive data from client

					// TODO: receive data from client

					// TODO: check if connection was closed

					// TODO: find user in db based on fd

					// TODO: subscribe/unsubscribe user
				}
			}
		}
	}
}


void Server::InitServer() {
	int rc, enable;

	// get a TCP socket for receiving connections
	tcp_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(tcp_socket_fd < 0, "tcp socket");

	// get a UDP socket for receiving connections
	udp_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	DIE(udp_socket_fd < 0, "udp socket");

	// make the sockets reusable
	enable = 1;
	rc = setsockopt(tcp_socket_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
	DIE(rc < 0, "setsockopt(SO_REUSEADDR) tcp");

	enable = 1;
	rc = setsockopt(udp_socket_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
	DIE(rc < 0, "setsockopt(SO_REUSEADDR) udp");

	// disable Nagle algorithm for the tcp socket
	enable = 1;
	rc = setsockopt(tcp_socket_fd, SOL_TCP, TCP_NODELAY, &enable, sizeof(int));
	DIE(rc < 0, "setsockopt(SO_REUSEADDR) tcp");

	// TODO: ioctl() -> make sockets nonblocking?

	// fill server_addr data
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(server_port);
	server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	// bind tcp socket to server address
	rc = bind(tcp_socket_fd, (const struct sockaddr *)&server_addr, sizeof(server_addr));
	DIE(rc < 0, "tcp bind");

	// bind udp socket to server address
	rc = bind(udp_socket_fd, (const struct sockaddr *)&server_addr, sizeof(server_addr));
	DIE(rc < 0, "udp bind");

	// listen on the tcp socket
	rc = listen(tcp_socket_fd, MAX_LISTEN_QUEUE);
	DIE(rc < 0, "listen");
}


void Server::ProcessNewTcpConnection() {
	int rc;
	sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	// initialize the client_addr to 0
	memset(&client_addr, 0, sizeof(client_addr));

	// accept new tcp connection
	int client_fd =
		accept(tcp_socket_fd, (struct sockaddr *)&client_addr, &client_len);
	DIE(client_fd < 0, "accept");

	// add the new client to the poll
	poll_fds.push_back({client_fd, POLLIN, 0});

	// receive client id
	string client_id;
	rc = recv(client_fd, &client_id, MAX_CLIENT_ID_LEN, 0);

	// check if client_id already exists
	if (users_database.count(client_id)) {
		User user = users_database.find(client_id)->second;

		if (user.IsOnline()) {
			// client is online
			printf("Client %s already connected.", client_id);

		} else {
			// client is offline
			printf("New client %s connected from %s:%d", client_id,
					inet_ntoa(client_addr.sin_addr), client_addr.sin_port);
			
			// update user's online status
			user.SetOnline(true);
		}

	} else {
		// is a new user
		User user(client_fd, true);
		users_database.insert({client_id, user});
	}
}

void Server::ExitServer() {
	// TODO: close connections
}
