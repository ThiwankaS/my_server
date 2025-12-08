#pragma once

#include "Utility.hpp"

class Response {
    public :
        static void sendResponse(int client_fd, const ResponseData& res);
        static void sendHeader(int new_client_fdfd, const ResponseData& res);
};
