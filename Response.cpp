#include "Response.hpp"

std::string Response::buildHeader(Client& client) {
    std::ostringstream os;
    const ResponseData& res = client.response;

    os << "HTTP/1.1 " << res.status_code << "\r\n"
        << "Content-Type: " + res.content_type + "\r\n"
        << "Content-Length: " + std::to_string(res.buffer.size()) + "\r\n"
        << "Connection: keep-alive\r\n"
        << "\r\n";

    std::string header = os.str();
    return (header);
}

void Response::buildResponse(Client& client) {
    const ResponseData& res = client.response;

    client.addToResponseBuffer(buildHeader(client));
    client.addToResponseBuffer(std::string(res.buffer.begin(), res.buffer.end()));
}

void Response::sendResponse(Client& client) {
    if(!client.isResponseCompleted()) {
        client.size_sent += send(
            client.client_Fd, 
            client.response_buffer.c_str() + client.size_sent,
            client.response_buffer.size() - client.size_sent, 0); 
    }
    if(client.isResponseCompleted()) {
        client.clearAndUpdateResponseBuffer("");
        client.size_sent = 0;
    }
}