#pragma once

#include "Utility.hpp"

class Response {
    public :
        inline static const std::map<uint16_t, std::string> static_table = {
            { 200, "OK" },
            { 201, "Created" },
            { 202, "Accepted"},
            { 400, "Bad Request" },
            { 403, "Forbidden" },
            { 404, "Not Found" },
            { 406, "Not Acceptable" },
            { 413, "Payload Too Large" },
            { 500, "Internal Server Error" },
            { 501, "Not Implemented" },
            { 505, "HTTP Version Not Supported" },
            { 508, "Loop Detected" }
        };

        static std::string buildHeader(Client& client);
        static void buildResponse(Client& client);
        static void sendResponse(Client& client);
};
