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
#include <cmath>
#include <sstream>
#include <sys/ioctl.h>


#include <iomanip>

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

		for (auto & poll_fd : poll_fds) {
			//cout << "---------> checking poll_fd: " << poll_fd.fd << endl;
			if (poll_fd.revents & POLLIN) {
				if (poll_fd.fd == tcp_socket_fd) {
					// process the new tcp connection
					ProcessNewTcpConnection();
					//cout << "processed new tcp connection()\n";

				} else if (poll_fd.fd == udp_socket_fd) {
					// receive data from udp socket
					ProcessUdpData();
					//cout << "processed new udp connection()\n";

				} else if (poll_fd.fd == STDIN_FILENO) {
					// process the command given to the server by stdin
					stop_server = ProcessStdinCommand();
					if (stop_server) break;
					//cout << "processed new stdin command\n";
				
				} else {
					// receive data from client
					ProcessClientRequest(poll_fd);
					//cout << "processed new client request\n";
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
	rc = setsockopt(tcp_socket_fd, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(int));
	DIE(rc < 0, "setsockopt(TCP_NODELAY) tcp");

	// TODO: ioctl() -> make sockets nonblocking?
	//rc = ioctl(tcp_socket_fd, FIONBIO, (char *)&enable);

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

	// receive client id
	string client_id(MAX_CLIENT_ID_LEN, '\0');
	rc = recv_all(client_fd, &client_id[0], MAX_CLIENT_ID_LEN);
	// cout << "received client id" << endl;

	client_id.resize(rc);

	// cout << "[DEBUG] client id = " << client_id << endl;

	// check if client_id already exists
	if (users_database.count(client_id)) {

		// ? ---->>>> might be copy not the actual element in map
		User& user = users_database.at(client_id);

		if (user.IsOnline()) {
			// client exists and is online
			bool ans = 0;
			send_all(client_fd, &ans, sizeof(bool));
			
			close(client_fd);

			printf("Client %s already connected.\n", client_id.c_str());
			
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
		// cout << "[DEBUG] inserted user to database: " << client_id << " - " << user.GetFd() << endl;
	}

	// show message for new and reconnected users
	printf("New client %s connected from %s:%d.\n", client_id.c_str(),
			inet_ntoa(client_addr.sin_addr), client_addr.sin_port);
	
	bool ans = 1;
	send_all(client_fd, &ans, sizeof(bool));

	// add the new client to the poll
	poll_fds.push_back({client_fd, POLLIN, 0});
}

void Server::ProcessUdpData() {
	int rc;
	sockaddr_in udp_client_addr;
	socklen_t udp_client_len = sizeof(udp_client_addr);

	string buffer(MAX_BUFF_SIZE, '\0');

	// receive data
	rc = recvfrom(udp_socket_fd, &buffer[0], MAX_BUFF_SIZE, 0,
		(struct sockaddr *)&udp_client_addr, &udp_client_len);
	DIE(rc < 0, "recvfrom udp");

	buffer.resize(rc);

	// cout << "[DEBUG] received udp packet:" << endl;

	// parse content received
	string topic;
	uint8_t data_type;
	string content;

	auto pos = buffer.substr(0, MAX_TOPIC_SIZE).find('\0');

	if (pos != string::npos) {
		topic = buffer.substr(0, pos);
	} else {
		topic = buffer.substr(0, MAX_TOPIC_SIZE);
	}

	data_type = buffer.at(MAX_TOPIC_SIZE);
	content = buffer.substr(MAX_TOPIC_SIZE + 1, rc - (MAX_TOPIC_SIZE + 1));

	// cout << "[DEBUG] topic: " << topic << endl;
	// cout << "[DEBUG] data_type: " << (char)(data_type + '0') << endl;

	// set the message sent to the users
	string message;
	message = inet_ntoa(udp_client_addr.sin_addr);
	message = message + ":" + to_string(udp_client_addr.sin_port) + " - " + topic + " - ";
	
	if (data_type == 0) {
		message = message + "INT" + " - ";
		if (content[0] == 1) {
			message += "-";
		}
		uint32_t num = ntohl(*((uint32_t *)(&content[1])));
		message += to_string(num);
		//cout << "[DEBUG] content: " << num << endl;

	} else if (data_type == 1) {
		message = message + "SHORT_REAL" + " - ";
		uint16_t num = ntohs(*((uint16_t *)(&content[0])));

		ostringstream out;
		out.precision(2);
		out << fixed << num / 100.0;
		message += move(out).str();

	} else if (data_type == 2) {
		message = message + "FLOAT" + " - ";
		if (content[0] == 1) {
			message += "-";
		}
		uint32_t num = ntohl(*((uint32_t *)(&content[1])));
		//cout << "number: " << num << endl;

		uint8_t power =  *((uint8_t *)(&content[5]));
		//cout << "power: " << power + 0 << endl;

		float power_val = pow(10, power);

		ostringstream out;
		out.precision(power);
		out << fixed << num / power_val;
		message += move(out).str();

		
	}else if (data_type == 3) {
		message = message + "STRING" + " - ";

		auto pos = content.find('\0');

		if (pos != string::npos) {
			message += content.substr(0, pos);
		} else {
			message += content;
			message += '\0';
		}
		
	}

	//printf("[DEBUG] message sent from udp: %s\n", message.c_str());


	// notify users
	for (auto user : users_database) {
		user.second.NotifyUser(topic, message);
	}

	// cout << "[DEBUG] sent udp packet:" << endl;
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

// TODO: give &poll_fd as argument
void Server::ProcessClientRequest(pollfd &poll_fd) {
	int rc;
	string buffer(MAX_USER_COMMAND_SIZE, '\0');

	pair<string, User&> user = FindUserByFd(poll_fd.fd);
	// cout << "[DEBUG] << FindById: user is online - " << user.second.IsOnline() << endl;

	rc = recv_all(poll_fd.fd, &buffer[0], MAX_USER_COMMAND_SIZE);
	buffer.reserve(rc);

	// cout << "[DEBUG] received data of size - " << rc << " - from tcp: " << buffer << endl;


	if (rc == 0) {
		printf("Client %s disconnected.\n", user.first.c_str());
		
		// user has disconnected
		user.second.SetOnline(false);
		user.second.SetFd(-1);

		// close the connection fd
		close(poll_fd.fd);
		poll_fd.fd = -1;
		
		return;
	}

	if (buffer[0] == 's') {
		// subscribe user to topic
		string topic = buffer.substr(1, rc - 3);
		bool sf = atoi(&buffer[rc - 2]);
		cout << "sf:" << sf;

		// ? --------->>>>> needs Setter?
		user.second.GetSubbedTopic().insert({topic, sf});
		//cout << "[DEBUG] user has subscribed to topic: " << topic << " - sf: " << sf << endl;



	} else if (buffer[0] == 'u') {
		// unsubscribe user from topic
		string topic = buffer.substr(1, rc - 2);

		user.second.GetSubbedTopic().erase(topic);
		//cout << "[DEBUG] user has unsubscribed from topic: " << topic << endl;
	}
}

void Server::ExitServer() {
	// close connections
	for (auto user : users_database) {
		send_all(user.second.GetFd(), NULL, 0);
	}

	for (auto poll_fd : poll_fds) {
		if (poll_fd.fd >= 0)
			close(poll_fd.fd);
	}
}

// TODO: return by address
pair<string, User&> Server::FindUserByFd(int fd) {
	// ? ----->>>> User* ?
	for (auto user : users_database) {
		if (user.second.GetFd() == fd) {
			return {user.first, users_database.at(user.first)};
		}
	}
	return {"NO_USER", users_database.at(0)};
}
