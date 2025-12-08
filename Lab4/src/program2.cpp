#include <iostream>
#include <iomanip>
#include <string>
#include <dlfcn.h>
#include <cstring>

typedef float (*PiFunc)(int);
typedef float (*EFunc)(int);

class DynamicLibraryManager {
private:
    void* lib1_handle;
    void* lib2_handle;
    
    PiFunc current_Pi;
    EFunc current_E;
    
    int current_lib;
    
public:
    DynamicLibraryManager() 
        : lib1_handle(nullptr), lib2_handle(nullptr), 
          current_Pi(nullptr), current_E(nullptr), current_lib(1) {
        
        // Load library 1
        this->lib1_handle = dlopen("./liblib_1.so", RTLD_LAZY);
        if (!this->lib1_handle) {
            std::cerr << "Error loading liblib_1.so: " << dlerror() << std::endl;

            throw std::runtime_error("Failed to load lib1");
        }
        
        // Load library 2
        this->lib2_handle = dlopen("./liblib_2.so", RTLD_LAZY);
        if (!this->lib2_handle) {
            std::cerr << "Error loading liblib_2.so: " << dlerror() << std::endl;
            dlclose(this->lib1_handle);

            throw std::runtime_error("Failed to load lib2");
        }
        
        switchToLib(this->current_lib);
    }
    
    ~DynamicLibraryManager() {
        if (this->lib1_handle) {
            dlclose(this->lib1_handle);
        }

        if (this->lib2_handle) {
            dlclose(this->lib2_handle);
        } 
    }
    
    void switchToLib(int libNum) {
        if (libNum != 1 && libNum != 2) {
            std::cout << "Invalid library number. Must be 1 or 2.\n";
            return;
        }
        
        void* handle = (libNum == 1) ? this->lib1_handle : this->lib2_handle;
        
        dlerror();
        
        PiFunc pi_func = (PiFunc)dlsym(handle, "Pi");
        if (!pi_func) {
            std::cerr << "Error loading Pi function: " << dlerror() << std::endl;
            return;
        }
        
        EFunc e_func = (EFunc)dlsym(handle, "E");
        if (!e_func) {
            std::cerr << "Error loading E function: " << dlerror() << std::endl;
            return;
        }
        
        this->current_Pi = pi_func;
        this->current_E = e_func;
        this->current_lib = libNum;
        
        std::cout << "Switched to ";
        if (this->current_lib == 1) {
            std::cout << "Library 1 (Leibniz series for Pi, (1+1/x)^x for e)\n";
        } 
        else {
            std::cout << "Library 2 (Wallis formula for Pi, series sum for e)\n";
        }
    }
    
    float callPi(int K) const {
        if (!this->current_Pi) {
            throw std::runtime_error("Pi function not loaded");
        }

        return this->current_Pi(K);
    }
    
    float callE(int x) const {
        if (!this->current_E) {
            throw std::runtime_error("E function not loaded");
        }

        return this->current_E(x);
    }
    
    int getCurrentLib() const {
        return this->current_lib;
    }
    
    std::string getCurrentLibName() const {
        if (this->current_lib == 1) {
            return "Library 1 (Leibniz series for Pi, (1+1/x)^x for e)";
        }
        else {
            return "Library 2 (Wallis formula for Pi, series sum for e)";
        }
    }
};

void printUsage() {
    std::cout << "\nDynamic loading of Libraries 1 and 2\n";
    std::cout << "Commands:\n";
    std::cout << "  0             - Switch between Library 1 and Library 2\n";
    std::cout << "  1 <K>         - Calculate Pi with K terms\n";
    std::cout << "  2 <x>         - Calculate e\n";
    std::cout << "  ~q            - Quit the program\n";
    std::cout << "======================================================\n" << std::endl;
}

int main() {
    try {
        DynamicLibraryManager libMgr;
        std::string cmdStr;
        
        printUsage();
        std::cout << "Current: " << libMgr.getCurrentLibName() << "\n\n";
        
        while (true) {
            std::cout << "> ";
            std::getline(std::cin, cmdStr);
            
            if (cmdStr.empty()) {
                continue;
            }
            
            if (cmdStr == "~q") {
                std::cout << "Exiting..." << std::endl;
                break;
            }
            
            int cmd = -1;
            try {
                cmd = std::stoi(cmdStr.substr(0, 1));
            } catch (...) {
                std::cout << "Invalid command. Please enter 0, 1, 2, or ~q\n";
                continue;
            }
            
            if (cmd == 0) {
                
                int nextLib = (libMgr.getCurrentLib() % 2) + 1;
                libMgr.switchToLib(nextLib);
            }
            else if (cmd == 1) {
                try {
                    int K = std::stoi(cmdStr.substr(2));
                    if (K <= 0) {
                        std::cout << "K must be positive" << std::endl;
                        continue;
                    }

                    std::cout << std::fixed << std::setprecision(6);
                    std::cout << "Pi(" << K << ") = " << libMgr.callPi(K) << "\n";
                } catch (const std::exception& e) {
                    std::cout << "Error: " << e.what() << "\n";
                } catch (...) {
                    std::cout << "Invalid argument for Pi. Usage: 1 <K>" << "\n";
                }
            }
            else if (cmd == 2) {
                try {
                    int x = std::stoi(cmdStr.substr(2));
                    if (x == 0) {
                        std::cout << "x cannot be zero" << "\n";
                        continue;
                    }

                    std::cout << std::fixed << std::setprecision(6);
                    std::cout << "E(" << x << ") = " << libMgr.callE(x) << "\n";
                } catch (const std::exception& e) {
                    std::cout << "Error: " << e.what() << "\n";
                } catch (...) {
                    std::cout << "Invalid argument for E. Usage: 2 <x>" << "\n";
                }
            }
            else {
                std::cout << "Invalid command. Please enter 0, 1, 2, or q" << "\n";
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return -1;
    }
    
    return 0;
}
