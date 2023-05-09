// Copyright Ionescu Matei-Stefan - 323CAb - 2022-2023
#include <string>
#include <vector>
#include <sys/poll.h>

using namespace std;

#define SUBSCRIBE_COMMAND "subscribe"
#define UNSUBSCRIBE_COMMAND "unsubscribe"
#define EXIT_COMMAND "exit"

class Subscriber {
	private:
		// the id of the client
		string id;

		// the ip and port of the server the client will connect to
		string server_ip;
		uint16_t server_port;

		// the file descriptor for the connection
		int socket_fd;
		
		// fd vector for poll
		vector<pollfd> poll_fds;

	public:
		Subscriber(string id, string server_ip, uint16_t server_port);

		void RunClient();

	private:
		void ConnectToServer();

		void ProcessServerMessage();
		
		bool ProcessStdinCommand();
};

inline Subscriber::Subscriber(string id, string server_ip, uint16_t server_port) {
	this->id = id;
	this->server_ip = server_ip;
	this->server_port = server_port;
}