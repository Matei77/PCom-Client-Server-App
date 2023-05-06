#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <poll.h>
#include <netinet/tcp.h>
#include <vector>
#include <unistd.h>
#include <unordered_map>
#include <unordered_set>

#include "utils.hpp"

#define MAX_LISTEN_QUEUE 32

#define EXIT_COMMAND "exit"

using namespace std;

struct udp_message_t {
	string topic;
	char data_type;
	string content;
};

struct topic_t {
	string topic;
	bool sf;

	bool operator==(const topic_t& t) const {
		return (this->topic == t.topic);
	}

	struct HashFunction {
		size_t operator()(const topic_t& t) const {
			return hash<string>()(t.topic);
		}
	}
};

class Database {
	unordered_set<User, User::HashFunction> users;

};

class User {
	private:
		string id;
		bool online;
		int fd;
		unordered_set<topic_t, topic_t::HashFunction> subbed_topics;
		vector <string> queued_messages;
	
	public:
		User (string id, bool online, int fd) {
			this->id = id;
			this->fd = fd;
			this->online = online;
		}

		bool operator==(const User& u) const {
			return (this->id == u.id);
		}

		struct HashFunction {
			size_t operator()(const User& u) const {
				return u.id;
			}
		}
};

class Server {
	private:
		int tcp_socket_fd, udp_socket_fd;
		sockaddr_in server_addr;
		uint16_t server_port;
		vector<pollfd> poll_fds;

	public:
		Server(uint16_t port) {
			this->server_port = port;
		}

		void RunServer() {
			int rc;

			InitServer();

			// add the tcp, udp and stdin fd to the poll
			poll_fds.push_back({tcp_socket_fd, POLLIN, 0});
			poll_fds.push_back({udp_socket_fd, POLLIN, 0});
			poll_fds.push_back({STDIN_FILENO, POLLIN, 0});

		
			while (1) {
				rc = poll(poll_fds.data(), poll_fds.size(), -1);
				DIE (rc < 0, "poll");

				for (auto poll_fd : poll_fds) {
					if (poll_fd.revents & POLLIN) {

						if (poll_fd.fd == tcp_socket_fd) {
							// accept new tcp connection
							int client_fd = accept(tcp_socket_fd, NULL, NULL);
							DIE (client_fd < 0, "accept");

							// add the new client to the poll
							poll_fds.push_back({client_fd, POLLIN, 0});
						
						} else if (poll_fd.fd == udp_socket_fd) {
							// receive data from udp socket

						} else if (poll_fd.fd == STDIN_FILENO) {
							// process the command given to the server by stdin
							string command;
							getline(cin, command, '\n');

							if (command == EXIT_COMMAND) {
								ExitServer();
								break;
							
							} else {
								cout << "Unrecognized command\n";
							}

						} else {
							// receive data from client

						}
					}
				}
			}
		}
	
	private:
		void InitServer() {
			int rc, enable;

			// get a TCP socket for receiving connections
			tcp_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
			DIE (tcp_socket_fd < 0, "tcp socket");

			// get a UDP socket for receiving connections
			udp_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
			DIE (udp_socket_fd < 0, "udp socket");

			// make the sockets reusable
			enable = 1;
			rc = setsockopt(tcp_socket_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
			DIE(rc < 0, "setsockopt(SO_REUSEADDR) tcp");

			enable = 1;
			rc = setsockopt(udp_socket_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
			DIE(rc < 0, "setsockopt(SO_REUSEADDR) udp");

			// disable Nagle algorithm
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
			DIE (rc < 0, "tcp bind");

			//bind udp socket to server address
			rc = bind(udp_socket_fd, (const struct sockaddr *)&server_addr, sizeof(server_addr));
			DIE (rc < 0, "udp bind");

			// listen on the tcp socket
			rc = listen(tcp_socket_fd, MAX_LISTEN_QUEUE);
			DIE (rc < 0, "listen");
		}

		void ExitServer() {

		}
};

int main(int argc, char *argv[]) {
	// disable buffering
	setvbuf(stdout, NULL, _IONBF, BUFSIZ);

	// check the number of arguments received
	if (argc != 2) {
		printf("\n Usage: %s <DESIRED_PORT>\n", argv[0]);
		return EXIT_FAILURE;
	}

	// get the port
	uint16_t port;
	int rc = sscanf(argv[1], "%hu", &port);
	DIE(rc != 1, "Given port is invalid");

	Server server(port);
	
	server.RunServer();
}