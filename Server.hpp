#pragma once

#include <cerrno>
#include <cstring>
#include <string>
#include <fstream>
#include <sstream>
#include <string_view>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/epoll.h>
#include <signal.h>
#include <vector>
#include <map>

#include "CustomeException.hpp"
#include "Request.hpp"
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
            void send_header(int new_fd, std::string type, size_t size);
            void startListening(void);
            void serve_binary_file(int new_fd, const std::string& path, const std::string& type);
            void serve_html_file(int new_fd, const std::string& path);
            void handleNewConnection(int listenFD, int epollFD);
            void handleClientIO(int clientFD, uint32_t client_event);
            void process_request(int clientFD);
            int createSocket(void);
            bool setSocketNonBlocking(int fd);
            const std::string get_in_addr(struct addrinfo& sa);
            std::pair<bool, std::string> read_request(int new_fd);

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
