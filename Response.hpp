#pragma once

#include "Utility.hpp"

class Response {
    public :
        static std::string buildHeader(Client& client);
        static void buildResponse(Client& client);
        static void sendResponse(Client& client);
};
