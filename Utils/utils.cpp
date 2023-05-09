#include "utils.hpp"

#include <sys/socket.h>
#include <sys/types.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <algorithm>

using namespace std;

// receive size then content
int recv_all(int sockfd, void *buffer, uint32_t max_len) {
	// receive data length
	uint32_t len;
	ssize_t bytes_received = 0;
	ssize_t bytes_remaining = sizeof(uint32_t);
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

// send size then content
int send_all(int sockfd, void *buffer, uint32_t len) {
	// send the size of the data
	uint32_t conv = htonl(len);

	ssize_t bytes_sent = 0;
	ssize_t bytes_remaining = sizeof(conv);
	char *buff = (char *)&conv;

	while (bytes_remaining) {
		bytes_sent = send(sockfd, buff, bytes_remaining, 0);
		DIE(bytes_sent < 0, "send_all data length");

		bytes_remaining -= bytes_sent;
		buff += bytes_sent;
	}

	// send the content
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
