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
    std::string raw_path = root + path;

    if(!client.request.is_valid && client.response.status_code == 400) {

        client.response.buffer              = getFileBuffer(error_routes.at(client.response.status_code));
        client.response.content_type        = "text/html; charset=utf-8";
        return (client);
    }
    
    if(client.request.http_method == METHOD::GET) {

        client.response.status_code         = 404;
        client.response.buffer              = getFileBuffer(error_routes.at(client.response.status_code));
        client.response.content_type        = "text/html; charset=utf-8";

        bool is_safe = false;
        if(std::filesystem::exists(raw_path)) {
            std::filesystem::path root_abs = std::filesystem::canonical(root);
            std::filesystem::path requested_abs = std::filesystem::canonical(raw_path);

            if(requested_abs.string().starts_with(root_abs.string())) {
                is_safe = true;
            }
        }

        if(is_safe) {
            struct stat s;
            if(stat(raw_path.c_str(), &s) == 0) {
                if(s.st_mode & S_IFDIR) {
                    raw_path += "index.html";
                }
            }
            std::ifstream file_check(raw_path.c_str());
            if(raw_path.find("/cgi-bin/") != std::string::npos && raw_path.ends_with(".py")) {
                if(file_check.good()){
                    file_check.close();
                    client.response.buffer              = getFileBuffer("./webpage/result.html");
                    client.response.status_code         = 200;
                    client.response.content_type        = "text/html; charset=utf-8";
                } 
            } else {
                if(file_check.good()){
                    file_check.close();
                    client.response.buffer          = getFileBuffer(raw_path);
                    client.response.status_code     = 200;
                    client.response.content_type    = setContentType(raw_path);
                }
            }
        }
    }
    
    if(client.request.http_method == METHOD::POST) {
        client.response.buffer              = getFileBuffer("./webpage/result.html");
        client.response.status_code         = 200;
        client.response.content_type        = "text/html; charset=utf-8";
    }
    
    return (client);
}
