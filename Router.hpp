#pragma once

#include "Utility.hpp"

class Router {
    private:
        static std::vector<char> getFileBuffer(const std::string& path);
        static std::string setContentType(std::string& path);
    public :
        static Client route(Client& client);
};
