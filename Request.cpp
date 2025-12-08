#include "Request.hpp"

Request::Request(Client &client) {
    RequestData request_data = parse_request(client);
}

std::vector<std::string> Request::slplit(std::string& str, std::string_view delimeter) {
    std::vector<std::string> splitted_strings;

    auto splitted1 = str | std::views::lazy_split(delimeter);
    for(const auto& part : splitted1) {
        auto common_part = part | std::views::common;
        splitted_strings.emplace_back(common_part.begin(), common_part.end());
    }
    return (splitted_strings);
}

METHOD Request::setMethod(const std::string& method) {
    if(method == "GET") {
        return METHOD::GET;
    }
    if(method == "POST") {
        return METHOD::POST;
    }
    if(method == "DELETE") {
        return METHOD::DELETE;
    }
    if(method == "PUT") {
        return METHOD::PUT;
    }
    return METHOD::UNKNOWN;
}

VERSION Request::setVersion(const std::string& version) {
    if(version == "HTTP/1.1") {
        return VERSION::ONE_POINT_ONE;
    }
    if(version == "HTTP/1.0") {
        return VERSION::ONE_POINT_ZERO;
    }
    if(version == "HTTP/0.9") {
        return VERSION::POINT_NINE;
    }
    return VERSION::UNKNOWN;
}

bool Request::setConnection(const std::vector<std::string>& lines) {
    for(const auto& line : lines){
        if(line.starts_with("Connection:")){
            if(line.find("keep-alive") != std::string::npos){
                return (true);
            }
        }
    }
    return (false);
}

std::string Request::setHost(const std::vector<std::string>& lines) {
    for(const auto& line : lines){
        if(line.starts_with("Host:")){
            size_t pos = line.find(":");
            if(pos != std::string::npos){
                return (line.substr(pos + 1));
            }
        }
    }
    return ("");
}

RequestData Request::parse_request(Client& client) {

    std::vector<std::string> sections   = slplit(client.buffer_data, "\r\n\r\n");
    std::string& header = sections.at(0);
    std::vector<std::string> lines      = slplit(header, "\r\n");

    std::istringstream request_stream(lines.at(0));
    std::string method, path, version;
    request_stream >> method >> path >> version;

    RequestData client_data;
    client_data.http_method     = setMethod(method);
    client_data.http_version    = setVersion(version);
    client_data.is_keep_alive   = setConnection(lines);
    client_data.host            = setHost(lines);
    client_data.path            = path;
    if(sections.size() > 1) {
        client_data.body        = sections.at(1);
    } else {
        client_data.body        = "";
    }
    client_data.is_valid        = true;
    return (client_data);

        // if(path == "/" || path == "/index.html"){
        //     request = result.first;
        //     HTTP::Server::process_request(clientFD);
        //     HTTP::Server::serve_html_file(clientFD, "./webpage/index.html");
        // }
        // if(path == "/cpp.png"){
        //     request = result.first;
        //     HTTP::Server::process_request(clientFD);
        //     HTTP::Server::serve_binary_file(clientFD, "./webpage/cpp.png", "image/png");
        // }
        // if(path == "/style.css"){
        //     request = result.first;
        //     HTTP::Server::process_request(clientFD);
        //     HTTP::Server::serve_binary_file(clientFD, "./webpage/style.css", "text/css");
        // }
        // if(path == "/script.js") {
        //     request = result.first;
        //     HTTP::Server::process_request(clientFD);
        //     HTTP::Server::serve_binary_file(clientFD, "./webpage/script.js", "text/javascript");
        // }
}
