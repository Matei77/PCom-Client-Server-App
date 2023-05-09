// Copyright Ionescu Matei-Stefan - 323CAb - 2022-2023
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include <sys/poll.h>
#include <iostream>

#include "subscriber.hpp"
#include "../utils/utils.hpp"
#include "../utils/types.hpp"

void Subscriber::ConnectToServer() {
	int rc;

	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(socket_fd < 0, "socket");

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
	send_all(socket_fd, &id, id.size());
}

void Subscriber::RunClient() {
	int rc;
	bool stop_client = false;

	ConnectToServer();

	poll_fds.push_back({socket_fd, POLLIN, 0});
	poll_fds.push_back({STDIN_FILENO, POLLIN, 0});

	do {
		rc = poll(poll_fds.data(), poll_fds.size(), -1);
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

void Subscriber::ProcessServerMessage() {
	string message;

	// receive message from server
	int rc = recv_all(socket_fd, &message[0]);

	// print message
	printf("%s\n", message);
}

bool Subscriber::ProcessStdinCommand() {
	string token;
	int rc;
	getline(cin, token, ' ');

	if (token == SUBSCRIBE_COMMAND) {
		string server_command;

		server_command = "s";

		(cin, token, ' ');
		if (token.size() == 0) {
			printf("Unrecognized command.\n");
			return;
		}

		server_command = server_command + token;

		getline(cin, token);
		if (token.size() == 0) {
			printf("Unrecognized command.\n");
			return;
		}

		server_command = server_command + token;

		send_all(socket_fd, &server_command, server_command.size());

		printf("Subscribed to topic.\n");

	} else if (token == UNSUBSCRIBE_COMMAND) {
		string server_command;

		server_command = "u";

		getline(cin, token);
		if (token.size() == 0) {
			printf("Unrecognized command.\n");
			return;
		}

		server_command = server_command + token;

		send_all(socket_fd, &server_command, server_command.size());

		printf("Unsubscribed from topic.\n");

	} else if (token == EXIT_COMMAND) {
		return true;

	} else {
		printf("Unrecognized command.\n");
	}

	return false;
}