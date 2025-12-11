#pragma once

#include "CustomeException.hpp"
#include "Request.hpp"
#include "Router.hpp"
#include "Response.hpp"
#include "Utility.hpp"

#define BACKLOG 10          // how many pending connections queue will hold

namespace HTTP {
    class Server {
        private:
            int sockfd;
            int epollFD;
            const char* port;
            const char* ip_address;
            struct addrinfo hints;
            struct addrinfo *servinfo;
            struct sigaction sa;
            std::string request;
            std::map<size_t, Client> active_clients;

            void setConncetion(void);
            void startListening(void);
            void handleNewConnection(int listenFD, int epollFD);
            void handleClientRead(int clientFD, int epollFD);
            void handleClientWrite(int clientFD, int epollFD);
            void process_request(int clientFD);
            int createSocket(void);
            bool setSocketNonBlocking(int fd);
            const std::string getInAddr(struct addrinfo& sa);
            std::pair<bool, std::string> readRequest(int new_fd);
            bool sendRequest(int new_fd);

        public:
            struct ServerException : public CustomeExecption {
                ServerException(const std::string& msg)
                    :CustomeExecption(msg){}
            };

            Server(const char* address, const char* p);
            Server& operator=(const Server& other) = delete;
            Server(const Server& other) = delete;
            ~Server();
            void startServer(void);
    };

    void sigchild_handler(int s);
}
