#pragma once

#include <iostream>
#include <string>
#include <exception>

class CustomeExecption : public std::exception {
    private:
        std::string message;
    public:
        CustomeExecption(const std::string& msg) noexcept;
        CustomeExecption(const CustomeExecption& other) noexcept;
        CustomeExecption& operator=(const CustomeExecption& other) noexcept;
        ~CustomeExecption() noexcept;

        const char* what() const noexcept;

};
