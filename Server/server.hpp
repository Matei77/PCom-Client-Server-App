// Copyright Ionescu Matei-Stefan - 323CAb - 2022-2023
#ifndef SERVER_HPP_
#define SERVER_HPP_

#include <unordered_map>
#include <vector>
#include <sys/poll.h>
#include <string>
#include <arpa/inet.h>

#include "user.hpp"

#define MAX_LISTEN_QUEUE 32
#define MAX_CLIENT_ID_LEN 10
#define MAX_BUFF_SIZE 1551
#define MAX_TOPIC_SIZE 50
#define MAX_USER_COMMAND_SIZE 65
#define EXIT_COMMAND "exit"

using namespace std;

class Server {
	private:
		// map id to the user
		unordered_map<string, User> users_database;

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
		Server(uint16_t port);

		// start and run the server
		void RunServer();

	private:
		// initialize the server
		void InitServer();

		// process a new tcp connection
		void ProcessNewTcpConnection();

		// process the data received from a udp connection
		void ProcessUdpData();

		// process commands given to the server by stdin.
		// returns 1 if the server should stop
		bool ProcessStdinCommand();

		// process the data sent by the tcp client
		void ProcessClientRequest(pollfd &poll_fd);

		// close the server
		void ExitServer();
		
		// find user by its file descriptor
		pair<string, User&> FindUserByFd(int fd);

		// generate the message that the tcp clients will receive if they are subscribed to the topic
		string GenerateMessage(string topic, uint8_t data_type,	string content, sockaddr_in udp_client_addr);
};

// constructor for Server class
inline Server::Server(uint16_t port) {
	this->server_port = port;
}

#endif // SERVER_HPP_
