// Copyright Ionescu Matei-Stefan - 323CAb - 2022-2023
#include <string>
#include <sys/socket.h>

#include "../utils/utils.hpp"
#include "user.hpp"

using namespace std;

void User::NotifyUser(string topic, string message) {
	if (subbed_topics.count(topic)) {
		if (online) {
			// if the user is online send the message
			int rc = send(fd, message.data(), message.size(), 0);
			DIE(rc < 0, "send");

		} else if ((*subbed_topics.find(topic)).second) {
			// if the user is offline and has store and forward active,
			// store the message
			queued_messages.push(message);
		}
	}
}
