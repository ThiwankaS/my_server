#include "Response.hpp"

void Response::sendHeader(int client_fd, const ResponseData& res) {
    std::ostringstream os;

    os << "HTTP/1.1 " << res.status_code << "\r\n"
        << "Content-Type: " + res.content_type + "\r\n"
        << "Content-Length: " + std::to_string(res.buffer.size()) + "\r\n"
        << "Connection: keep-alive\r\n"
        << "\r\n";

    std::string header = os.str();
    send(client_fd, header.c_str(), header.size(), 0);
}

void Response::sendResponse(int client_fd, const ResponseData& res) {
    sendHeader(client_fd, res);
    send(client_fd, res.buffer.data(), res.buffer.size(), 0);
}
