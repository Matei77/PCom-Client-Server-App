#include "utils.hpp"

#include <sys/socket.h>
#include <sys/types.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <algorithm>

#include <iostream>

using namespace std;

// receive size then content
uint32_t recv_all(int sockfd, void *buffer, uint32_t max_len) {
	// receive data length
	// cout << "start recv_all" << endl;
	uint32_t len;
	ssize_t bytes_received = 0;
	size_t bytes_remaining = sizeof(uint32_t);
	char *buff = (char *)&len;

	while(bytes_remaining) {
		bytes_received = recv(sockfd, buff, bytes_remaining, 0);
		DIE(bytes_received < 0, "recv_all data lenght");

		buff += bytes_received;
		bytes_remaining -= bytes_received;
	}

	len = ntohl(len);

	// cout << "received len:" << len << endl; 

	// receive content
	bytes_received = 0;
	bytes_remaining = min(len, max_len);
	buff = (char *)buffer;

	while(bytes_remaining) {
		bytes_received = recv(sockfd, buff, bytes_remaining, 0);
		DIE(bytes_received < 0, "recv_all data content");

		// cout << "bytes received: " << bytes_received << endl;
		buff += bytes_received;
		bytes_remaining -= bytes_received;
	}

	// cout << "end recv_all" << endl;
   return min(len, max_len);
}

// send size then content
uint32_t send_all(int sockfd, void *buffer, uint32_t len) {
	// send the size of the data
	// cout << "start send" << endl;
	uint32_t conv = htonl(len);

	ssize_t bytes_sent = 0;
	size_t bytes_remaining = sizeof(uint32_t);
	char *buff = (char *)&conv;

	while (bytes_remaining) {
		bytes_sent = send(sockfd, buff, bytes_remaining, 0);
		DIE(bytes_sent < 0, "send_all data length");

		bytes_remaining -= bytes_sent;
		buff += bytes_sent;
	}

	// cout << "sent len:" << len << endl;

	// send the content
	bytes_sent = 0;
	bytes_remaining = len;
	buff = (char *)buffer;

	while(bytes_remaining) {
		bytes_sent = send(sockfd, buff, bytes_remaining, 0);
		DIE(bytes_sent < 0, "send_all data content");

		// cout << "bytes send: " << bytes_sent << endl;
		bytes_remaining -= bytes_sent;
		buff += bytes_sent;
	}

	// cout << "end send" << endl;
	return len;
}

uint32_t recv_udp(int sockfd, void *buffer, uint32_t len, sockaddr_in& udp_client_addr, socklen_t& udp_client_len) {
	int rc;
	ssize_t bytes_received = 0;
	char *buff = (char *)buffer;
	ssize_t bytes_remaining = len;

	while (bytes_remaining) {
		rc = recvfrom(sockfd, buff, bytes_remaining, 0,
			(struct sockaddr *)&udp_client_addr, &udp_client_len);
		DIE(rc < 0, "recvfrom udp");

		buff += rc;
		bytes_received += rc;
		bytes_remaining -= rc;
	}

	return bytes_received;
}