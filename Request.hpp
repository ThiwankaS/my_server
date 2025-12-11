#pragma once

#include "Utility.hpp"

class Request {
    private:
        std::string http_version = "HTTP/1.1";
    public:
        Request(Client& client);
        static Client parse_request(Client& client);
        static std::vector<std::string> slplit(std::string& str, std::string_view delimeter);
        static METHOD setMethod(const std::string& method);
        static VERSION setVersion(const std::string& version);
        static bool setConnection(const std::vector<std::string>& lines);
        static std::string setHost(const std::vector<std::string>& lines);
};
