#pragma once

#include <algorithm>
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <errno.h>
#include <fstream>
#include <fcntl.h>
#include <iostream>
#include <iomanip>
#include <map>
#include <netinet/in.h>
#include <netdb.h>
#include <ranges>
#include <signal.h>
#include <sstream>
#include <string>
#include <string_view>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>


enum class METHOD {
    GET,
    POST,
    DELETE,
    PUT,
    HEAD,
    UNKNOWN
};

enum class VERSION {
    POINT_NINE,
    ONE_POINT_ZERO,
    ONE_POINT_ONE,
    UNKNOWN
};

struct Client {
    size_t      client_Fd;
    bool        is_complete;
    std::string buffer_data;

    void addToBuffer(std::string str) {
        buffer_data.append(str);
        if(buffer_data.find("\r\n\r\n") != std::string::npos){
            is_complete = true;
        }
    }

    void clearAndUpdateBuffer(std::string str) {
        buffer_data.clear();
        is_complete = false;
        addToBuffer(str);
    }
};

struct RequestData {
    METHOD      http_method;
    VERSION     http_version;
    std::string path;
    std::string host;
    std::string body;
    bool        is_keep_alive;
    bool        is_valid;
};

struct ResponseData {
    std::string         status_code;
    std::string         content_type;
    std::vector<char>   buffer;
};
