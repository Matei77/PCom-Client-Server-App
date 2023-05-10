// Copyright Ionescu Matei-Stefan - 323CAb - 2022-2023
#include <iostream>

#include "../utils/utils.hpp"
#include "server.hpp"

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

	// create server
	Server server(port);

	// run the server
	server.RunServer();

	return 0;
}
