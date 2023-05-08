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
		bool IsOnline();

		void SetOnline(bool online);

		int GetFd();
	
		void SetFd(int fd);

		unordered_map<string, bool> GetSubbedTopic();
};

// constructor for User class
inline User::User(int fd, bool online) {
	this->fd = fd;
	this->online = online;
}

// Getters and Setters
inline bool User::IsOnline() {
	return this->online;
}

inline void User::SetOnline(bool online) {
	this->online = online;
}

inline void User::SetFd(int fd) {
	this->fd = fd;
}

inline int User::GetFd() {
	return this->fd;
}

inline unordered_map<string, bool> User::GetSubbedTopic() {
	return this->subbed_topics;
}

#endif // USER_HPP_
