// Copyright Ionescu Matei-Stefan - 323CAb - 2022-2023
#ifndef USER_HPP_
#define USER_HPP_

#include <queue>
#include <unordered_map>

using namespace std;

class User {
	private:
		// user's socket connection fd
		int fd;

		// user's online status
		bool online;

		// map topic to SF status
		unordered_map<string, bool> subbed_topics;

		// queued messages for SF
		queue<string> queued_messages;

	public:
		User(int fd, bool online);

		void NotifyUser(string topic, string message);

		void ReconnectUser();

		// Getters and Setters
		bool IsOnline() { return this->online; }

		void SetOnline(bool online) { this->online = online; }
};

// constructor for User class
inline User::User(int fd, bool online) {
	this->fd = fd;
	this->online = online;
}

#endif // USER_HPP_
