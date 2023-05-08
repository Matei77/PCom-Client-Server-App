// Copyright Ionescu Matei-Stefan - 323CAb - 2022-2023
#ifndef TYPES_HPP_
#define TYPES_HPP_

#include <string>

using namespace std;

struct udp_message_t {
	string topic;
	char data_type;
	string content;
};

#endif // TYPES_HPP_
