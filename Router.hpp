#pragma once

#include "Utility.hpp"

class Router {
    private:
        static std::vector<char> getFileBuffer(const std::string& path);
        static std::string setContentType(std::string& path);
    public :
        inline static const std::map<uint16_t, std::string> error_routes = {
            { 400 , "./webpage/errors/400.html"},
            { 404 , "./webpage/errors/404.html"}
        };
        static Client route(Client& client);
};
