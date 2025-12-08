#include <iostream>
#include <iomanip>
#include <string>

#include "../include/lib_1.hpp"

void printUsage() {
    std::cout << "\nStatic linking to Library 1\n";
    std::cout << "Commands:\n";
    std::cout << "  1 <K>         - Calculate Pi using Leibniz series with K terms\n";
    std::cout << "  2 <x>         - Calculate e using (1+1/x)^x formula\n";
    std::cout << "  -1 *          - Quit the program\ns";
    std::cout << "========================================\n" << std::endl;
}

int main() {
    std::string command;
    
    printUsage();
    
    while (true) {
        int cmd = -1, arg;
        std::cout << "> ";
        
        std::cin >> cmd >> arg;

        if (cmd == 1) {
            if (arg <= 0) {
                std::cout << "K must be positive" << "\n";
                continue;
            }
            float result = Pi(arg);
            std::cout << std::fixed << std::setprecision(6);
            std::cout << "Pi(" << arg << ") = " << result << "\n";
        }
        else if (cmd == 2) {
            if (arg == 0) {
                std::cout << "x cannot be zero" << "\n";
                continue;
            }
            float result = E(arg);
            std::cout << std::fixed << std::setprecision(6);
            std::cout << "E(" << arg << ") = " << result << "\n";
        }
        else if (cmd == -1) {
            std::cout << "Exiting..." << std::endl;
            break;
        }
        else {
            std::cout << "Invalid command. Please enter 1, 2, or q" << "\n";
        }
    }
    
    return 0;
}
