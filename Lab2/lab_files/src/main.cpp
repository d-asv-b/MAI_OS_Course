#include <iostream>
#include <fstream>
#include <string>
#include <boost/multiprecision/cpp_int.hpp>
using namespace boost::multiprecision;

int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::cerr << "Укажите количество потоков (--threads или -t) и количество ОЗУ (--memory or -m) в аргументах командной строки.\n";
        return 1;
    }

    size_t thread_count = 0, memory_limit = 0;
    
    for (unsigned i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--threads" || arg == "-t") {
            if (i + 1 < argc) {
                thread_count = std::stoul(argv[++i]);
            } else {
                std::cerr << "Отсутствует значение для количества потоков.\n";

                return 1;
            }
        } else if (arg == "--memory" || arg == "-m") {
            if (i + 1 < argc) {
                memory_limit = std::stoul(argv[++i]);
            } else {
                std::cerr << "Отсутствует значение для количества ОЗУ.\n";

                return 1;
            }
        }
    }

    if (thread_count == 0) {
        std::cerr << "Количество потоков должно быть больше нуля.\n";
        return 1;
    }
    else if (memory_limit < 128) {
        std::cerr << "Количество ОЗУ должно быть не меньше 128 байт.\n";
        return 1;
    }

    std::string filename;
    std::cout << "Введите имя файла с числами:\n";
    std::cin >> filename;

    std::ifstream input(filename);
    if (!input.is_open()) {
        std::cerr << "Не удалось открыть файл: " << filename << "\n";
        return 1;
    }

    std::string hex_number;
    while (input >> hex_number) {
        
    }


    return 0;
}