#include "Server.hpp"

namespace HTTP {
    /**
     * argument constructor to initialize and set class data
    */
    Server::Server(const char* address, const char* p):
    sockfd(-1),
    port(p),
    ip_address(address),
    hints(),
    servinfo(nullptr),
    sa(),
    request(){
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;
    };

    /**
     * destructor
    */
    Server::~Server () {
        freeaddrinfo(servinfo);
    }

    /**
     * creating the connection between IP address and port
    */
    void Server::setConncetion(void) {
        if(getaddrinfo(ip_address, port, &hints, &servinfo) != 0) {
            throw ServerException("Fail establishing connection !");
        }
        std::cout << "Connecetion sucessful....\n";
    }

    /**
     * binding the socket with the Ip address, will loop through all
     * available socket and bind it with thge port
    */
    int Server::createSocket(void) {
        struct addrinfo *p;
        int sockfd, yes = 1;

        // loop thorugh all the addresses and try to create a socket and bind it
        for(p = servinfo; p != NULL; p = p->ai_next) {
            // creating a socket
            if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
                std::cerr << strerror(errno) << "\n";
                continue;
            }
            // if socket is already taken try to resue it
            if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes ,sizeof(int)) == -1){
                std::ostringstream ss;
                ss << port
                    << " already in used";
                throw ServerException(ss.str());
            }
            // binding the socket and the address
            if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
                std::cerr << strerror(errno) << "\n";
                continue;
            }

            break;
        }

        // non of the connections was able to bind to an availabe socket
        if(p == NULL) {
            throw ServerException("Socket/Port binding fail!");
        }
        return (sockfd);
    }

    /**
     * will start to listen on the aloocated socket
    */
    void Server::startListening(void) {
        if(listen(sockfd, BACKLOG) < 0) {
            throw ServerException("Fail to start listening!");
        }
    }

    const std::string Server::get_in_addr(struct addrinfo& sa) {
        char s[INET6_ADDRSTRLEN];
        const char* address = nullptr;

        if(sa.ai_family == AF_INET){ // IPv4
            struct sockaddr_in* IPv4 = (struct sockaddr_in*)sa.ai_addr;
            void* ptr = &(IPv4->sin_addr);
            address = inet_ntop(AF_INET, ptr, s, sizeof(s));
        } else {
            struct sockaddr_in6* IPv6 = (struct sockaddr_in6*)sa.ai_addr;
            void* ptr = &(IPv6->sin6_addr);
            address = inet_ntop(AF_INET6, ptr, s, sizeof(s));
        }
        if(address == nullptr){
            throw ServerException("Fail to conver the IP address!");
        }
        return (ip_address);
    }

    void Server::startServer(void) {

        std::ostringstream ss;                      //to display the srever status

        HTTP::Server::setConncetion();              //connection establish
        sockfd = HTTP::Server::createSocket();      //socket binded
        HTTP::Server::startListening();             //start to liesten through the socket
        HTTP::Server::setSocketNonBlocking(sockfd); //set the socket non-blocking
        //signal handling
        sa.sa_handler = HTTP::sigchild_handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART;
        if(sigaction(SIGCHLD, &sa, NULL) == -1) {
                throw ServerException("Failt to remove zombies!");
        }
        // displaying the srever status
        ss << "server host at "
            << ip_address
            << " is listening on "
            << port;
        std::cout << ss.str() <<"\n";

        //creating a instance
        epollFD = epoll_create1(0);
        if(epollFD == -1) {
            throw ServerException("Epoll creation fail!");
        }

        //define the behavior for the socket fd
        struct epoll_event listen_event;
        listen_event.events = EPOLLIN | EPOLLET;
        listen_event.data.fd = sockfd;

        //add the fd to the epoll instance
        if(epoll_ctl(epollFD, EPOLL_CTL_ADD, sockfd, &listen_event) == -1) {
            close(epollFD);
            throw ServerException("Epoll ctl fail!");
        }

        //cerate a epoll event vector with the size MAX_EVENT
        const int MAX_EVENT = 64;
        std::vector <struct epoll_event> events(MAX_EVENT);

        while(true) {
            /**
             * epoll wait will return the number of events ready following the
             * events set above (EPOLLIN, EPOLLET) Thsi number can be anything
             * in between 1 - 64(MAX_EVENT)
            */
            int num_ready = epoll_wait(epollFD, events.data(), MAX_EVENT, -1);

            if(num_ready == -1){
                if(errno == EINTR) {
                    continue;
                }
                throw ServerException("Epoll wait fail!");
            }
            /**
             * iterate thorough all the rady events and check their FDs
            */
            for(int i = 0; i < num_ready; i++){
                int currentFD = events[i].data.fd;
                uint32_t current_event = events[i].events;
                //meaning a new client has connected
                if(currentFD == sockfd){
                    HTTP::Server::handleNewConnection(sockfd, epollFD);
                /**
                 * meaning currentFD != sockfd so the currentFD is one already
                 * existing in the epoll event instance. So no need to add the FD
                 * back to the instance, can proceed with the request
                */
                } else {
                    HTTP::Server::handleClientIO(currentFD, current_event);
                }

            }
        }
    }

    void Server::handleNewConnection(int listenFD, int epollFD) {
        struct sockaddr_storage client_addr;

        while (true) {
            //Reset size for the new client
            socklen_t size = sizeof(client_addr);
            /**
             * will accpet the new client and check it's FD
            */
            int newFD = accept(listenFD, (struct sockaddr*)&client_addr, &size);
            if(newFD == -1) {
                // only stop when the OS says "queue is empty"
                if(errno == EAGAIN || errno == EWOULDBLOCK) {
                    break;
                }
                std::cerr << strerror(errno) << "\n";
                continue; //move to the next available client
            }
            //make new client FD non-blocking
            if(!HTTP::Server::setSocketNonBlocking(newFD)) {
                close(newFD);
                continue;
            }
            //define the behavior for the new client
            struct epoll_event event;
            event.events = EPOLLIN | EPOLLET;
            event.data.fd = newFD;
            //adding the client fd to the epoll event instance
            if(epoll_ctl(epollFD, EPOLL_CTL_ADD, newFD, &event) == -1){
                std::cerr << "Epoll add client error!";
                close(newFD);
            }
            // adding client to the active client list
            Client new_client;
            new_client.client_Fd    = newFD;
            new_client.is_complete  = false;
            new_client.buffer_data  = "";
            active_clients.emplace(newFD, new_client);
        }
    }

    void Server::handleClientIO(int clientFD, uint32_t client_event){

        if(client_event & (EPOLLHUP | EPOLLERR)){
            close(clientFD);
            return;
        }

        auto result = read_request(clientFD);
        if(result.first == false || result.second.empty()){
            std::cerr << "Bad request!\n";
        }

        Client& client = active_clients[clientFD];
        client.addToBuffer(result.second);
        if(client.is_complete) {
            Data client_data = Request::parse_request(client);
        }
        close(clientFD);
    }

    void Server::send_header(int new_fd, std::string type, size_t size) {
        std::string header = "HTTP/1.1 200 OK\r\n";
            header += "Content-Type: " + type + "\r\n";
            header += "Content-Length: " + std::to_string(size) + "\r\n";
            header += "Connection: close\r\n";
            header += "\r\n";
        send(new_fd, header.c_str(), header.size(), 0);
    }

    void Server::serve_binary_file(int new_fd, const std::string& path, const std::string& type) {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if(!file) {
            std::cerr << "Can not open image file " + path + "\n";
            return;
        }
        std::streamsize file_size = file.tellg();
        file.seekg(0, std::ios::beg);
        std::vector<char> file_buffer (file_size);
        if(!file.read(file_buffer.data(), file_size)) {
            std::cerr << "Can not read image file " + path + "\n";
            return;
        }
        Server::send_header(new_fd, type, file_size);
        send(new_fd, file_buffer.data(), file_size, 0);
    }

    void Server::serve_html_file(int new_fd, const std::string& path) {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if(!file) {
            std::cerr << "Can not open file " + path + "\n";
            return;
        }
        std::streamsize file_size = file.tellg();
        file.seekg(0, std::ios::beg);
        std::vector<char> file_buffer (file_size);
        if(!file.read(file_buffer.data(), file_size)) {
            std::cerr << "Can not read file " + path + "\n";
            return;
        }
        Server::send_header(new_fd, "text/html", file_size);
        send(new_fd, file_buffer.data(), file_size, 0);
    }

    std::pair<bool, std::string> Server::read_request(int new_fd) {
        std::string out;
        char buffer[4096];
        ssize_t bytes_read;

        while(true) {
            bytes_read = recv(new_fd, buffer, sizeof(buffer), 0);
            if(bytes_read == -1) {
                if(errno == EINTR) {
                    continue;
                }
                if(errno == EAGAIN || errno == EWOULDBLOCK) {
                    break;
                }
                return (std::make_pair(false, ""));
            }
            if(bytes_read == 0) {
                return (std::make_pair(false, ""));
            }
            out.append(buffer, bytes_read);
        }
        return (std::make_pair(true, out));
    }

    /**
     * classic signal handling for
     */
    void sigchild_handler(int s) {
        (void)s;
        // caching the errno in case following processes over write
        int saved_errno = errno;
        // clean-up all terminated child processes
        while(waitpid(-1, NULL, WNOHANG) > 0);
        // re-assiging correct errno
        errno = saved_errno;
    }

    /**
     * make a scket non-blocking
    */
    bool Server::setSocketNonBlocking(int fd) {
        int flags = fcntl(fd, F_GETFL, 0);
        if(flags == -1) {
            std::cerr << "Failt to retrieve the flag from FD!";
            return (false);
        }
        flags |= O_NONBLOCK;
        if(fcntl(fd, F_SETFL, flags) == -1) {
            std::cerr << "Failt to set the Non block flag!";
            return (false);
        }
        return (true);
    }

    void Server::process_request(int clientFD) {
        (void) clientFD;
        std::cout << "Request in server side : \n"
                    << request << "\n";
        std::stringstream ss(request);
        std::cout << ss.str() << "\n";
    }
}


