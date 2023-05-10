// Copyright Ionescu Matei-Stefan - 323CAb - 2022-2023
#include <string>
#include <sys/socket.h>
#include <iostream>

#include "../Utils/utils.hpp"
#include "user.hpp"

using namespace std;

// Sends or stores the message if the user is subscribed to topic.
void User::NotifyUser(string topic, string message) {
	if (subbed_topics.count(topic)) {
		if (online) {
			// if the user is online send the message
			int rc = send_all(fd, &message[0], message.size() + 1);
			DIE(rc < 0, "send");

		} else if (subbed_topics.find(topic)->second) {
			// if the user is offline and has store and forward active,
			// store the message
			queued_messages.push(message);
		}
	}
}

// Sends all the messages from the queue.
void User::ReconnectUser() {
	while (!queued_messages.empty()) {
		// if user has queued messages send them
		string message = queued_messages.front();
		queued_messages.pop();

		send_all(fd, &message[0], message.size() + 1);
	}
}
