#include "CustomeException.hpp"

CustomeExecption::CustomeExecption(const std::string& msg) noexcept
: message(msg){}

CustomeExecption::CustomeExecption(const CustomeExecption& other) noexcept
:message(other.message){}

CustomeExecption& CustomeExecption::operator=(const CustomeExecption& other) noexcept {
    if(this != &other) {
        this->message = other.message;
    }
    return (*this);
}

CustomeExecption::~CustomeExecption() noexcept {}

const char* CustomeExecption::what() const noexcept {
    return (message.c_str());
}
