#include "./include/Server.hpp"

int main() {
    Server server;

    server.initialize();
    server.run();
    server.shutdown();

    return 0;
}
