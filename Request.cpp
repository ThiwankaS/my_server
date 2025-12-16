#include "Request.hpp"

Request::Request(Client &client) {
    client = parseRequest(client);
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

bool Request::isValidRequest(Client& client) {
    
    METHOD& method      = client.request.http_method; 
    std::string& path   = client.request.path; 
    VERSION& version    = client.request.http_version;
    
    if(method == METHOD::UNKNOWN || version == VERSION::UNKNOWN || !path.starts_with("/")) {
        return (false);
    }

    return (true);
}

Client Request::parseRequest(Client& client) {

    client.request = RequestData();
    
    std::vector<std::string> sections   = slplit(client.request_buffer, "\r\n\r\n");
    std::string& header = sections.at(0);
    client.clearAndUpdateRequestBuffer("");

    std::vector<std::string> lines      = slplit(header, "\r\n");
    
    std::cout << "line size : " << lines.size() << "\n";

    if(lines.empty()) {
        client.response.status_code = 400;
        client.request.is_valid = false;
        client.request.is_keep_alive = false;
        return (client);
    }

    std::istringstream request_stream(lines.at(0));
    std::string method, raw_path, version;
    request_stream >> method >> raw_path >> version;

    client.request.http_method     = setMethod(method);
    client.request.http_version    = setVersion(version);

    size_t pos = raw_path.find('?');
    if(pos != std::string::npos) {
        client.request.path = raw_path.substr(0, pos);
        client.request.query_string = raw_path.substr(pos + 1);
    } else {
        client.request.path = raw_path;
        client.request.query_string = "";
    }

    if(!isValidRequest(client)) {
        client.response.status_code = 400;
        client.request.is_valid = false;
        client.request.is_keep_alive = false;
        return (client);
    }

    for(size_t i = 1; i < lines.size(); ++i) {
        const std::string&  line = lines[i];

        if(line.starts_with("Host:")){
            size_t pos = line.find(":");
            if(pos != std::string::npos){
                std::string value = line.substr(pos + 1);
                size_t start = value.find_first_not_of(" \t");
                if(start != std::string::npos) {
                    client.request.host = value.substr(start);
                }
            }
        } else if (line.starts_with("Connection:")) {
            if(line.find("keep-alive") != std::string::npos){
                client.request.is_keep_alive = true;
            } else {
                client.request.is_keep_alive = false;
            }
        } else if (line.starts_with("Content-Length:")) {
            size_t pos = line.find(":");
            if(pos != std::string::npos) {
                try {
                    client.request.content_length = std::stoul(line.substr(pos + 1));
                } catch (...) {
                    client.request.content_length = 0;
                }
            }
        } else if (line.starts_with("Content-Type:")) {
            size_t pos = line.find(":");
            if(pos != std::string::npos) {
                std::string value = line.substr(pos + 1);
                size_t start = value.find_first_not_of(" \t");
                if(start != std::string::npos){
                    client.request.content_type = value.substr(start);
                }
            }
        }
    }

    if(sections.size() > 1) {
        client.request.body        = sections.at(1);
    } else {
        client.request.body        = "";
    }
    client.response.status_code    = 200;
    client.request.is_valid        = true;
    return (client);
}
