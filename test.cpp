#include <iostream>
#include <string>
#include <fstream>

int main(void) {
    std::string line;
    std::ifstream file("request.txt");
    while(std::getline(file, line)){
        std::cout << line << "\n";
        line.clear();
    }
    return (EXIT_SUCCESS);
}
