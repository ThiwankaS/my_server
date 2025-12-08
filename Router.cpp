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
    return ("text/html;charset=utf-8");
}

ResponseData Router::route(const RequestData& client_data) {
    ResponseData res;

    std::string root = "./webpage";
    std::string path = root + client_data.path;
    std::string error_path = root + "/error/404.html";

    if(!client_data.is_valid || client_data.http_method == METHOD::UNKNOWN) {
        res.buffer              = getFileBuffer(error_path);
        res.status_code         = "404 Not Found";
        res.content_type        = "text/html; charset=utf-8";

    }

    struct stat s;
    if(stat(path.c_str(), &s) == 0) {
        if(s.st_mode & S_IFDIR) {
            path += "index.html";
        }
    }

    std::ifstream file_check(path.c_str());
    if(client_data.http_method == METHOD::GET){
        if(file_check.good()){
            file_check.close();
            res.buffer          = getFileBuffer(path);
            res.status_code     = "200 OK";
            res.content_type    = setContentType(path);
        } else {
            res.buffer          = getFileBuffer(error_path);
            res.status_code     = "404 Not Found";
            res.content_type    = "text/html; charset=utf-8";
        }
    }
    return (res);
}
