// Copyright Ionescu Matei-Stefan - 323CAb - 2022-2023
#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int recv_all(int sockfd, void *buffer, uint32_t max_len);

int send_all(int sockfd, void *buffer, uint32_t len);

// macro for handling error codes
#define DIE(assertion, call_description)                                       \
	do {                                                                       \
		if (assertion) {                                                       \
			fprintf(stderr, "(%s, %d): ", __FILE__, __LINE__);                 \
			perror(call_description);                                          \
			exit(errno);                                                       \
		}                                                                      \
	} while (0)

#endif  // UTILS_HPP_