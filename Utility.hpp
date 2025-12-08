#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <string_view>
#include <ranges>
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
        addToBuffer(str);
    }
};

struct Data {
    METHOD      http_method;
    VERSION     http_version;
    std::string path;
    std::string host;
    std::string body;
    bool        is_keep_alive;
};

