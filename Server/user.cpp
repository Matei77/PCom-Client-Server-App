// Copyright Ionescu Matei-Stefan - 323CAb - 2022-2023
#include <string>
#include <sys/socket.h>
#include <iostream>

#include "../Utils/utils.hpp"
#include "user.hpp"

using namespace std;

void User::NotifyUser(string topic, string message) {
	//cout << "Notifying user: " << fd << endl;
	//cout << "topic:" << topic << endl;
	//cout << "stored topic:" << subbed_topics.begin()->first << endl;
	if (subbed_topics.count(topic)) {
		cout << "subbed topic sf:" << subbed_topics.find(topic)->second << endl;
		cout << "online" << online << endl;
		if (online) {
			// if the user is online send the message
			int rc = send_all(fd, &message[0], message.size());
			DIE(rc < 0, "send");

		} else if (subbed_topics.find(topic)->second) {
			// if the user is offline and has store and forward active,
			// store the message
			cout << "pushing to queue" << endl;
			queued_messages.push(message);
			cout << "queued messages: " << &queued_messages << endl;
		}
	}
}

void User::ReconnectUser() {
	cout << "reconnected user" << endl;
	cout << "queued messages: " << &queued_messages << endl;
	cout << "empty? " << queued_messages.empty() << endl;
	while (!queued_messages.empty()) {
		string message = queued_messages.front();
		queued_messages.pop();

		send_all(fd, &message[0], message.size() + 1);
	}

	// cout << "[DEBUG] reconnected user" << endl;
}
