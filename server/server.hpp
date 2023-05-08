// Copyright Ionescu Matei-Stefan - 323CAb - 2022-2023
#ifndef SERVER_HPP_
#define SERVER_HPP_

#include <unordered_map>
#include <vector>
#include <poll.h>
#include <string>
#include <arpa/inet.h>

#include "user.hpp"

#define MAX_LISTEN_QUEUE 32
#define MAX_CLIENT_ID_LEN 11
#define EXIT_COMMAND "exit"

using namespace std;

class Server {
	private:
		// map id to the user
		static unordered_map<string, User> users_database;

		// server sockets fd
		int tcp_socket_fd, udp_socket_fd;

		// server address
		sockaddr_in server_addr;

		// server port
		uint16_t server_port;

		// fd vector for poll
		vector<pollfd> poll_fds;

   public:
		// constructor
		Server::Server(uint16_t port);

		// start and run the server
		void RunServer();

	private:
		// initialize the server
		void InitServer();

		// process a new tcp connection
		void ProcessNewTcpConnection();

		// close the server
		void ExitServer();
};

// constructor for Server class
inline Server::Server(uint16_t port) {
	this->server_port = port;
}

#endif // SERVER_HPP_
