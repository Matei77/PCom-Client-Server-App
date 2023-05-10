// Copyright Ionescu Matei-Stefan - 323CAb - 2022-2023
#include <sys/socket.h>
#include <sys/types.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <algorithm>

#include "utils.hpp"

using namespace std;

// Receive size then content.
uint32_t recv_all(int sockfd, void *buffer, uint32_t max_len) {
	// receive len
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

	// receive content
	bytes_received = 0;
	bytes_remaining = min(len, max_len);
	buff = (char *)buffer;

	while(bytes_remaining) {
		bytes_received = recv(sockfd, buff, bytes_remaining, 0);
		DIE(bytes_received < 0, "recv_all data content");

		buff += bytes_received;
		bytes_remaining -= bytes_received;
	}

   return min(len, max_len);
}

// Send size then content.
uint32_t send_all(int sockfd, void *buffer, uint32_t len) {
	// send len
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

	// send content
	bytes_sent = 0;
	bytes_remaining = len;
	buff = (char *)buffer;

	while(bytes_remaining) {
		bytes_sent = send(sockfd, buff, bytes_remaining, 0);
		DIE(bytes_sent < 0, "send_all data content");

		bytes_remaining -= bytes_sent;
		buff += bytes_sent;
	}

	return len;
}
