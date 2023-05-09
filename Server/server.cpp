// Copyright Ionescu Matei-Stefan - 323CAb - 2022-2023
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <queue>
#include <unordered_map>
#include <algorithm>
#include <vector>

#include "../Utils/utils.hpp"
#include "server.hpp"
#include "user.hpp"

using namespace std;

void Server::RunServer() {
	int rc, stop_server = 0;

	InitServer();

	// add the tcp, udp and stdin fd to the poll
	poll_fds.push_back({tcp_socket_fd, POLLIN, 0});
	poll_fds.push_back({udp_socket_fd, POLLIN, 0});
	poll_fds.push_back({STDIN_FILENO, POLLIN, 0});

	do {

		rc = poll(&poll_fds[0], poll_fds.size(), -1);
		DIE(rc < 0, "poll");

		for (auto poll_fd : poll_fds) {
			if (poll_fd.revents & POLLIN) {
				if (poll_fd.fd == tcp_socket_fd) {
					// process the new tcp connection
					ProcessNewTcpConnection();

				} else if (poll_fd.fd == udp_socket_fd) {
					// receive data from udp socket
					ProcessUdpData();

				} else if (poll_fd.fd == STDIN_FILENO) {
					// process the command given to the server by stdin
					stop_server = ProcessStdinCommand();
					if (stop_server) break;
				
				} else {
					// receive data from client
					ProcessClientRequest(poll_fd);
				}
			}
		}

		// erase closed file descriptors from the vector
		poll_fds.erase(remove_if(poll_fds.begin(), poll_fds.end(),
			[](pollfd p_fd) { return p_fd.fd == -1; }), poll_fds.end());

	} while (!stop_server);

	ExitServer();
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
	rc = recv_all(client_fd, &client_id[0]);

	// check if client_id already exists
	if (users_database.count(client_id)) {

		// ? ---->>>> might be copy not the actual element in map
		User user = users_database.find(client_id)->second;

		if (user.IsOnline()) {
			// client exists and is online
			printf("Client %s already connected.\n", client_id);
			
			return;

		} else {
			// client exists but is offline

			// update user's online status
			user.SetOnline(true);
			user.SetFd(client_fd);

			// reconnect user
			user.ReconnectUser();
		}

	} else {
		// is a new user
		User user(client_fd, true);
		users_database.insert({client_id, user});
	}

	// show message for new and reconnected users
	printf("New client %s connected from %s:%d.\n", client_id,
			inet_ntoa(client_addr.sin_addr), client_addr.sin_port);
}

void Server::ProcessUdpData() {
	int rc;
	sockaddr_in udp_client_addr;
	socklen_t udp_client_len = sizeof(udp_client_addr);

	string buffer(MAX_BUFF_SIZE, '\0');

	// receive data
	// ? ----->>>> might not receive data well
	rc = recvfrom(udp_socket_fd, &buffer, MAX_BUFF_SIZE, 0,
		(struct sockaddr *)&udp_client_addr, &udp_client_len);
	DIE(rc < 0, "recvfrom udp");

	buffer.resize(rc);

	printf("[DEBUG] packet received from udp: %s", buffer);

	// parse content received
	string topic;
	char data_type;
	string content;

	auto pos = buffer.substr(0, MAX_TOPIC_SIZE).find('\0');
	if (pos != string::npos) {
		topic = buffer.substr(0, pos);
		data_type = buffer.at(pos);
		content = buffer.substr(pos + 1, rc - pos - 1);
	} else {
		topic = buffer.substr(0, MAX_TOPIC_SIZE);
		data_type = buffer.at(MAX_TOPIC_SIZE);
		content = buffer.substr(MAX_TOPIC_SIZE + 1, rc - pos - 1);
	}

	// set the message sent to the users
	string message;
	message = *inet_ntoa(udp_client_addr.sin_addr) + ":"
			  + to_string(udp_client_addr.sin_port) + " - " + topic + " - ";
	
	if (data_type == 0) {
		message = message + "INT" + " - ";
		if (content[0] == 1) {
			message += "-";
		}

	} else if (data_type == 1) {
		message = message + "SHORT_REAL" + " - ";

	} else if (data_type == 2) {
		message = message + "FLOAT" + " - ";
		
	}else if (data_type == 3) {
		message = message + "STRING" + " - ";
		
	}
	message += content;

	printf("[DEBUG] message sent from udp: %s\n", message);


	// notify users
	for (auto user : users_database) {
		user.second.NotifyUser(topic, message);
	}
}

bool Server::ProcessStdinCommand() {
	string command;
	getline(cin, command, '\n');

	if (command == EXIT_COMMAND) {
		return 1;

	} else {
		printf("Unrecognized command\n");
	}

	return 0;
}

void Server::ProcessClientRequest(pollfd poll_fd) {
	int rc;
	string buffer(MAX_USER_COMMAND_SIZE, '\0');

	pair<string, User> user = FindUserByFd(poll_fd.fd);

	rc = recv_all(poll_fd.fd, &buffer[0]);
	DIE (rc < 0, "recv tcp client");

	buffer.resize(rc);

	if (rc == 0) {
		// user has disconnected
		user.second.SetOnline(false);
		user.second.SetFd(-1);

		// close the connection fd
		close(poll_fd.fd);
		poll_fd.fd = -1;

		printf("Client %s disconnected.\n", user.first);
		
		return;
	}

	if (buffer[0] == 's') {
		// subscribe user to topic
		string topic = buffer.substr(1, rc - 2);
		bool sf = atoi(&buffer[rc - 1]);

		// ? --------->>>>> needs Setter?
		user.second.GetSubbedTopic().insert({topic, sf});



	} else if (buffer[0] == 'u') {
		// unsubscribe user from topic
		string topic = buffer.substr(1, rc - 1);

		user.second.GetSubbedTopic().erase(topic);
	}
}

void Server::ExitServer() {
	// close connections
	for (auto poll_fd : poll_fds) {
		if (poll_fd.fd >= 0)
			close(poll_fd.fd);
	}
}

pair<string, User> Server::FindUserByFd(int fd) {
	// ? ----->>>> User* ?
	for (auto user : users_database) {
		if (user.second.GetFd() == fd) {
			return user;
		}
	}
	return {"NO_USER", User{-1, 0}};
}
