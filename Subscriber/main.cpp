// Copyright Ionescu Matei-Stefan - 323CAb - 2022-2023
#include <iostream>

#include "subscriber.hpp"
#include "../Utils/utils.hpp"

int main(int argc, char *argv[]) {
	// disable buffering
	setvbuf(stdout, NULL, _IONBF, BUFSIZ);

	// check the number of arguments received
	if (argc != 4) {
		printf("\n Usage: %s <CLIENT_ID> <SERVER_IP> <SERVER_PORT>\n", argv[0]);
		return EXIT_FAILURE;
	}

	// get the port
	uint16_t port;
	int rc = sscanf(argv[3], "%hu", &port);
	DIE(rc != 1, "Given port is invalid");


	Subscriber subscriber(argv[1], argv[2], port);

	subscriber.RunClient();

	return 0;
}
