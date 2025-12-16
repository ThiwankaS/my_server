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

    const std::string Server::getInAddr(struct addrinfo& sa) {
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
                }

                if(current_event & (EPOLLHUP | EPOLLERR)) {
                    close(currentFD);
                    active_clients.erase(currentFD);
                    continue;
                }

                if( current_event & EPOLLIN) {
                    HTTP::Server::handleClientRead(currentFD, epollFD);
                }

                if( current_event & EPOLLOUT) {
                    HTTP::Server::handleClientWrite(currentFD, epollFD);
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
                std::cerr << "Accept error :" << strerror(errno) << "\n";
                continue; //move to the next available client
            }
            //make new client FD non-blocking
            if(!HTTP::Server::setSocketNonBlocking(newFD)) {
                close(newFD);
                continue;
            }

            // adding client to the active client list
            Client client(newFD);
            active_clients.insert({ client.client_Fd, client });

            //define the behavior for the new client
            struct epoll_event event;
            event.events = EPOLLIN | EPOLLET;
            event.data.fd = newFD;

            //adding the client fd to the epoll event instance
            if(epoll_ctl(epollFD, EPOLL_CTL_ADD, newFD, &event) == -1){
                std::cerr << "Epoll add client error!";
                active_clients.erase(newFD);
                close(newFD);
            }
        }
    }

    void Server::handleClientRead(int clientFD, int epollFD) {

        auto it = active_clients.find(clientFD);
        if(it == active_clients.end()) {
            return;   
        }

        Client& client = it->second;

        auto result = readRequest(clientFD);
        if(result.first == false && result.second.empty()){
            active_clients.erase(clientFD);
            close(clientFD);
            return;
        }

        client.addToRequestBuffer(result.second);
        client.size_recv += result.second.size();

        if(client.size_recv > MAX_REQUEST_SIZE) {
            
            struct epoll_event event;
            event.events = EPOLLOUT | EPOLLET;
            event.data.fd = clientFD;
            epoll_ctl(epollFD, EPOLL_CTL_MOD, clientFD, &event);
            return;
        }

        if(client.is_complete) {
            client = Request::parseRequest(client);
            Server::printClient(client);
            client = Router::route(client);
            Response::buildResponse(client);
            struct epoll_event event;
            event.events = EPOLLOUT | EPOLLET;
            event.data.fd = clientFD;
            epoll_ctl(epollFD, EPOLL_CTL_MOD, clientFD, &event);
            return;
        }
    }

    std::pair<bool, std::string> Server::readRequest(int new_fd) {
        std::string out;
        char buffer[4096];

        while(true) {
            ssize_t bytes_read = recv(new_fd, buffer, sizeof(buffer), 0);
            
            if(bytes_read == -1) {
                if(errno == EINTR) {
                    continue;
                }
                if(errno == EAGAIN || errno == EWOULDBLOCK) {
                    return (std::make_pair(true, out));
                }
                return (std::make_pair(false, ""));
            }
            if(bytes_read == 0) {
                return (std::make_pair(false, ""));
            }
            out.append(buffer, bytes_read);
        }
    }

    void Server::handleClientWrite(int clientFD, int epollFD) {

        auto it = active_clients.find(clientFD);
        
        if(it == active_clients.end()) {
            return;   
        }

        Client& client = it->second;

        bool keep_alive = sendRequest(clientFD);
        
        if(client.isResponseCompleted()) {
            if(!keep_alive) {
                 active_clients.erase(clientFD);
                close(clientFD);
            } else {
                client.clearAndUpdateResponseBuffer("");
                client.size_sent = 0;
                client.is_complete = false;

                struct epoll_event event;
                event.events = EPOLLIN | EPOLLET;
                event.data.fd = clientFD;
                epoll_ctl(epollFD, EPOLL_CTL_MOD, clientFD, &event); 
            }
        }
    }

    bool Server::sendRequest(int new_fd) {
        
        auto it = active_clients.find(new_fd);
        if(it == active_clients.end()){
            std::runtime_error("Client connection lost!");
        }

        Client& client = it->second;
        ssize_t bytes_write;

        while(!client.isResponseCompleted()) {

            const char* data_ptr = client.response_buffer.c_str() + client.size_sent;
            size_t remaining = client.response_buffer.size() - client.size_sent;

            bytes_write = send(client.client_Fd, data_ptr, remaining, 0);
            if(bytes_write == -1) {
                if(errno == EINTR) {
                    continue;
                }
                if(errno == EAGAIN || errno == EWOULDBLOCK) {
                    break;
                }
                return (false);
            }    
            client.size_sent += bytes_write;                
        }
        return (client.request.is_keep_alive);
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

    void Server::printClient(Client& client) {
        std::cout << "Request in server side : \n";
        std::stringstream ss(request);
        if(client.request.http_method == METHOD::POST) {
                ss << "Method :  POST " << "\n";   
        }
        if(client.request.http_method == METHOD::GET) {
                ss << "Method :  GET " << "\n";   
        }
        ss << "Path : " << client.request.path << "\n";
        ss << "Query : " << client.request.query_string << "\n";
        ss << "Content size : " << client.request.content_length << "\n";
        ss << "Content type : " << client.request.content_type << "\n";
        ss << "body : {" << client.request.body << "}\n";
        std::cout << ss.str() << "\n";
    }
}


