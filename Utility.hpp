#pragma once

#include <algorithm>
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <errno.h>
#include <fstream>
#include <fcntl.h>
#include <filesystem>
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

const size_t MAX_REQUEST_SIZE = 5 * 1024 * 1024;

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

struct RequestData {
    METHOD      http_method;
    VERSION     http_version;
    std::string path;
    std::string query_string;
    std::string host;
    std::string content_type;
    size_t      content_length;
    std::string body;
    bool        is_keep_alive;
    bool        is_valid;

    RequestData() {
        http_method     = METHOD::UNKNOWN;
        http_version    = VERSION::UNKNOWN;
        path = "";
        query_string = "";
        host = "";
        content_type = "";
        content_length = 0;
        body = "";
        is_keep_alive = false;
        is_valid      = false;
    }
};

struct ResponseData {
    uint16_t            status_code;
    std::string         content_type;
    std::vector<char>   buffer;

    ResponseData() {
        status_code  = 0;
        content_type = "";
    }
};

struct Client {
    size_t          client_Fd;
    bool            is_complete;
    std::string     request_buffer;
    std::string     response_buffer;
    size_t          size_sent;
    size_t          size_recv;
    ResponseData    response;
    RequestData     request;

    Client(size_t newFD) : client_Fd(newFD), 
        is_complete(false), 
        request_buffer(""),
        response_buffer(""),
        size_sent(0),
        size_recv(0),
        response (),
        request  () {
    }

    void addToRequestBuffer(std::string str) {
        request_buffer.append(str);
        if(request_buffer.find("\r\n\r\n") != std::string::npos){
            is_complete = true;
        }
    }

    void clearAndUpdateRequestBuffer(std::string str) {
        request_buffer.clear();
        is_complete = false;
        addToRequestBuffer(str);
    }

    void addToResponseBuffer(std::string str) {
        response_buffer.append(str);
    }

    void clearAndUpdateResponseBuffer(std::string str) {
        response.status_code = 0;
        response_buffer.clear();
        addToResponseBuffer(str);
    }

    bool isResponseCompleted(void) {
        if (size_sent >= response_buffer.size()) {
            return (true);
        }
        return (false);
    }
};
