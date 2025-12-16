#include "Router.hpp"

std::vector<char> Router::getFileBuffer(const std::string& path) {
    std::vector<char> file_buffer = {0};
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if(!file) {
        std::cerr << "Can not open file " + path + "\n";
        return (file_buffer);
    }
    std::streamsize file_size = file.tellg();
    file_buffer.resize(file_size);
    file.seekg(0, std::ios::beg);
    if(!file.read(file_buffer.data(), file_size)) {
        std::cerr << "Can not read file " + path + "\n";
        return (file_buffer);
    }
    return (file_buffer);
}

std::string Router::setContentType(std::string& path) {
    if(path.find(".html") != std::string::npos){
            return ("text/html");
    }
    if(path.find(".css") != std::string::npos){
            return("text/css");
    }
    if(path.find(".png") != std::string::npos){
            return("image/png");
    }
    if(path.find(".js") != std::string::npos){
            return("text/javascript");
    }
    if(path.find(".jpg") != std::string::npos){
            return("image/jpeg");
    }
    return ("text/html;charset=utf-8");
}

Client Router::route(Client& client) {

    // collect required data to process
    std::string root = "./webpage";
    std::string path = client.request.path;
    std::string final_path = root + path;

    // if the request is invalid
    if(!client.request.is_valid) {
        client.response.status_code     = 400;
        client.response.buffer          = getFileBuffer(error_routes.at(client.response.status_code));
        client.response.content_type    = "text/html; charset=utf-8";
        return (client);
    }

    // if the requested resources is not availabe
    if(!std::filesystem::exists(final_path)) {
        client.response.status_code     = 404;
        client.response.buffer          = getFileBuffer(error_routes.at(client.response.status_code));
        client.response.content_type    = "text/html; charset=utf-8";
        return (client);
    }

    // if the the path is a directory
    if(std::filesystem::is_directory(final_path)) {
        final_path += "/index.html";
        if(!std::filesystem::exists(final_path)) {
            client.response.status_code     = 404;
            client.response.buffer          = getFileBuffer(error_routes.at(client.response.status_code));
            client.response.content_type    = "text/html; charset=utf-8";
            return (client);
        }
    }

    std::filesystem::path root_abs = std::filesystem::canonical(root);
    std::filesystem::path requested_abs = std::filesystem::canonical(final_path);

    // if the requested file out side the root directory 
    if(!requested_abs.string().starts_with(root_abs.string())) {
        client.response.status_code         = 403;
        client.response.buffer              = getFileBuffer(error_routes.at(client.response.status_code));
        client.response.content_type        = "text/html; charset=utf-8";
        return (client);
    }

    // handling metods
    if(client.request.http_method == METHOD::GET) {
        // check for a CGI request with URI arguments
        if(final_path.find("/cgi-bin/") != std::string::npos && final_path.ends_with(".cgi")) {
            std::string result = CGIHandler::execute("./webpage/cgi-bin/cpp_calculator.cgi", client);
            client.response.buffer.assign(result.begin(), result.end());
            client.response.status_code     = 200;
            client.response.content_type    = "text/html; charset=utf-8";
            return (client);
        } else {
            client.response.buffer          = getFileBuffer(final_path);
            client.response.status_code     = 200;
            client.response.content_type    = setContentType(final_path);
            return (client);
        }
    }
    
    if(client.request.http_method == METHOD::POST) {
        std::string result = CGIHandler::execute("./webpage/cgi-bin/cpp_calculator.cgi", client);
        client.response.buffer.assign(result.begin(), result.end());
        client.response.status_code         = 200;
        client.response.content_type        = "text/html; charset=utf-8";
        return (client);
    }
    
    return (client);
}
