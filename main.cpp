#include <iostream>
#include "Server.hpp"

using namespace HTTP;

int main (void) {
    try {
        Server myServer("127.0.0.1", "8812");
        myServer.startServer();
    } catch (const std::exception& e) {
        std::cerr << "Error :" << e.what() << "\n";
    }
    return (EXIT_SUCCESS);
}
