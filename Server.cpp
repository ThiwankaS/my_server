#include "Server.hpp"

namespace HTTP {
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

    Server::~Server () {
        freeaddrinfo(servinfo);
    }

    void Server::setConncetion(void) {
        if(getaddrinfo(ip_address, port, &hints, &servinfo) != 0) {
            throw ServerException("Fail establishing connection !");
        }
        std::cout << "Connecetion sucessful....\n";
    }

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
        std::ostringstream ss;

        HTTP::Server::setConncetion();
        sockfd = HTTP::Server::createSocket();
        HTTP::Server::startListening();
        sa.sa_handler = HTTP::sigchild_handler;
        HTTP::Server::setSocketNonBlocking(sockfd);
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART;
        if(sigaction(SIGCHLD, &sa, NULL) == -1) {
                throw ServerException("Failt to remove zombies!");
        }

        ss << "server host at "
            << ip_address
            << " is listening on "
            << port;
        std::cout << ss.str() <<"\n";

        struct epoll_event listen_event;
        epollFD = epoll_create1(0);
        if(epollFD == -1) {
            throw ServerException("Epoll creation fail!");
        }
        listen_event.events = EPOLLIN | EPOLLET;
        listen_event.data.fd = sockfd;

        if(epoll_ctl(epollFD, EPOLL_CTL_ADD, sockfd, &listen_event) == -1){
            throw ServerException("Epoll ctl fail!");
        }
        const int MAX_EVENT = 64;
        std::vector <struct epoll_event> events(MAX_EVENT);

        while(true) {
            int num_ready = epoll_wait(epollFD, events.data(), MAX_EVENT, -1);

            if(num_ready == -1){
                if(errno == EINTR) {
                    continue;
                }
                throw ServerException("Epoll wait fail!");
            }

            for(int i = 0; i < num_ready; i++){
                int currentFD = events[i].data.fd;
                uint32_t current_event = events[i].events;
                if(currentFD == sockfd){
                    HTTP::Server::handleNewConnection(sockfd, epollFD);
                } else {
                    HTTP::Server::handleClientIO(currentFD, current_event);
                }

            }
        }
    }

    void Server::send_header(int new_fd, size_t size) {
        char header[512];
        std::snprintf(header, sizeof(header),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %zu\r\n"
            "Connection: close\r\n"
            "\r\n",
            size
        );
        send(new_fd, header, std::strlen(header), 0);
    }

    std::string Server::load_home_page(void) {
        std::string file_name = "./webpage/index.html", line;
        std::ifstream home_page(file_name);
        std::ostringstream ss;
        if(home_page.is_open()) {
            while(std::getline(home_page, line)) {
                ss << line << "\n";
            }
        }
        return (ss.str());
    }

    void Server::image_header(int new_fd, size_t size) {
        std::string header = "HTTP/1.1 200 OK\r\n";
            header += "Content-Type: image/png\r\n";
            header += "Content-Length: " + std::to_string(size) + "\r\n";
            header += "Connection: close\r\n";
            header += "\r\n";
        send(new_fd, header.c_str(), header.size(), 0);
    }

    void Server::send_image(int new_fd) {
        std::ifstream file("./webpage/cpp.png", std::ios::binary | std::ios::ate);
        if(!file) {
            std::cerr << "Can not open image file \n";
        }
        std::streamsize file_size = file.tellg();
        file.seekg(0, std::ios::beg);
        std::vector<char> file_buffer (file_size);
        if(!file.read(file_buffer.data(), file_size)) {
            std::cerr << "Can not read image file \n";
            return;
        }
        Server::image_header(new_fd, file_size);
        send(new_fd, file_buffer.data(), file_size, 0);
    }

    void Server::send_response(int new_fd) {
        std::string body = load_home_page();
        send_header(new_fd, body.size());
        if(!body.empty()) {
            send(new_fd, body.data(), body.size(), 0);
        }
    }

    std::pair<std::string, bool> Server::read_request(int new_fd) {
        std::string out;
        char buffer[4096];
        size_t bytes_read;

        while(true) {
            bytes_read = recv(new_fd, buffer, sizeof(buffer), 0);
            if(bytes_read > 0) {
                out.append(buffer, bytes_read);
                if(out.find("\r\n\r\n") != std::string::npos){
                    break;
                }
                continue;
            }
            if(bytes_read == 0) {
                break;
            }
            if(errno == EINTR)
                continue;
            if(errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            out.clear();
            break;
        }
        return (std::make_pair(out, true));
    }

    void sigchild_handler(int s) {
        (void)s;
        // caching the errno in case following processes over write
        int saved_errno = errno;
        // clean-up all terminated child processes
        while(waitpid(-1, NULL, WNOHANG) > 0);
        // re-assiging correct errno
        errno = saved_errno;
    }

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

    void Server::handleNewConnection(int listenFD, int epollFD) {
        struct sockaddr_storage client_addr;
        socklen_t size = sizeof(client_addr);

        while (true) {

            int newFD = accept(listenFD, (struct sockaddr*)&client_addr, &size);
            if(newFD == -1) {
                if(errno == EAGAIN || errno == EWOULDBLOCK) {
                    break;
                }
                std::cerr << strerror(errno) << "\n";
                continue;
            }
            if(!HTTP::Server::setSocketNonBlocking(newFD)) {
                close(newFD);
                continue;
            }

            struct epoll_event event;
            event.events = EPOLLIN | EPOLLET;
            event.data.fd = newFD;

            if(epoll_ctl(epollFD, EPOLL_CTL_ADD, newFD, &event) == -1){
                std::cerr << "Epoll add client error!";
                close(newFD);
            }
        }
    }

    void Server::handleClientIO(int clientFD, uint32_t client_event){
        (void) client_event;
        auto result = read_request(clientFD);
        if(result.second == false || result.first.empty()){
            std::cerr << "Bad request!\n";
        }
        std::istringstream request_stream(result.first);
        std::string method, path, version;

        request_stream >> method >> path >> version;
        if(path == "/" || path == "/index.html"){
            request = result.first;
            HTTP::Server::process_request(clientFD);
            HTTP::Server::send_response(clientFD);
        }
        if(path == "/cpp.png"){
            request = result.first;
            HTTP::Server::process_request(clientFD);
            HTTP::Server::send_image(clientFD);
        }
        close(clientFD);
        epoll_ctl(epollFD, EPOLL_CTL_DEL, clientFD, NULL);
    }

    void Server::process_request(int clientFD) {
        (void) clientFD;
        std::cout << "Request in server side : \n"
                    << request << "\n";
        std::stringstream ss(request);
        std::cout << ss.str() << "\n";
    }
}


