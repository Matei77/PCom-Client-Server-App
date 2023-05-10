// Copyright Ionescu Matei-Stefan - 323CAb - 2022-2023
#include "subscriber.hpp"

#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

#include "../utils/utils.hpp"

// This method runs the client, monitoring the tcp connection socket and stdin fd using the poll
// system call.
void Subscriber::RunClient() {
	int rc;
	bool stop_client = false;

	ConnectToServer();

	// add the connection socket and stdin fd to the poll
	poll_fds.push_back({socket_fd, POLLIN, 0});
	poll_fds.push_back({STDIN_FILENO, POLLIN, 0});

	do {
		rc = poll(&poll_fds[0], poll_fds.size(), -1);
		DIE(rc < 0, "poll");

		for (auto poll_fd : poll_fds) {
			if (poll_fd.revents & POLLIN) {
				if (poll_fd.fd == socket_fd) {
					// the client received a message from the server
					ProcessServerMessage();

				} else {
					// the client received a command from the user
					stop_client = ProcessStdinCommand();
					if (stop_client) break;
				}
			}
		}
	} while (!stop_client);

	close(socket_fd);
}

// Create tcp socket, send the user's id to the server and get the response from the server.
void Subscriber::ConnectToServer() {
	int rc;

	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(socket_fd < 0, "socket");

	int enable = 1;
	rc = setsockopt(socket_fd, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(int));
	DIE(rc < 0, "setsockopt(TCP_NODELAY) tcp");

	struct sockaddr_in serv_addr;
	socklen_t socket_len = sizeof(struct sockaddr_in);

	memset(&serv_addr, 0, socket_len);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(server_port);
	rc = inet_pton(AF_INET, &server_ip[0], &serv_addr.sin_addr.s_addr);
	DIE(rc <= 0, "inet_pton");

	rc = connect(socket_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	DIE(rc < 0, "connect");

	// send client id to server
	send_all(socket_fd, &id[0], id.size() + 1);

	// receive the response from the server
	bool server_answer;
	rc = recv_all(socket_fd, &server_answer, sizeof(bool));
	if (server_answer == 0) {
		exit(0);
	}
}

// Show the message received from the server.
void Subscriber::ProcessServerMessage() {
	string message(MAX_MESSAGE_SIZE, '\0');

	// receive message from server
	int rc = recv_all(socket_fd, &message[0], MAX_MESSAGE_SIZE);

	// check for disconnect message
	if (rc == 0) {
		exit(0);
	}

	message.resize(rc);

	// print message
	printf("%s\n", message.c_str());
}

// Process the commands given by the user via stdin.
bool Subscriber::ProcessStdinCommand() {
	string command;
	string token;

	getline(cin, command);
	istringstream iss(command);

	// get the frst argument from the command
	getline(iss, token, ' ');

	if (token == SUBSCRIBE_COMMAND) {
		// the user sent the subscribe command
		string server_command = "s";

		// get second argument from the command
		token.clear();
		getline(iss, token, ' ');

		// check if it exits
		if (token.size() == 0) {
			printf("Unrecognized command.\n");
			return false;
		}

		server_command = server_command + token;

		// get third argument from the command
		token.clear();
		getline(iss, token);

		// check if it exits and is 0 or 1
		if (token.size() == 0 || (token != "0" && token != "1")) {
			printf("Unrecognized command.\n");
			return false;
		}

		server_command = server_command + token;

		send_all(socket_fd, &server_command[0], server_command.size() + 1);

		printf("Subscribed to topic.\n");

	} else if (token == UNSUBSCRIBE_COMMAND) {
		// the user sent the unsubscribe command
		string server_command = "u";

		// get second argument from the command
		token.clear();
		getline(iss, token);

		// check if it exits
		if (token.size() == 0) {
			printf("Unrecognized command.\n");
			return false;
		}

		server_command = server_command + token;

		send_all(socket_fd, &server_command[0], server_command.size() + 1);

		printf("Unsubscribed from topic.\n");

	} else if (token == EXIT_COMMAND) {
		send_all(socket_fd, NULL, 0);

		return true;

	} else {
		printf("Unrecognized command.\n");
	}

	return false;
}