#include <iostream>
#include <fstream>
#include <string>
#include <boost/multiprecision/cpp_int.hpp>

using namespace boost::multiprecision;

uint512_t* shared_data = nullptr;
std::ifstream shared_input;

pthread_mutex_t shared_mutex;

uint512_t calc_average_value_in_batch(size_t batch_size) {
    pthread_mutex_lock(&shared_mutex);
    uint512_t batch_average = 0;

    if (!shared_input.is_open()) {
        std::cerr << "Файл для чтения не был открыт." << "\n";
        return 1;
    }
    
    if (shared_data == nullptr) {
        std::cerr << "Память не была инициализирована." << "\n";
        return 1;
    }

    std::string hex_number;
    for (size_t i = 0; i < batch_size; ++i) {
        shared_input >> hex_number;

        if (hex_number.substr(0, 2) != "0x") {
            hex_number = "0x" + hex_number;
        }
        shared_data[i] = uint512_t(hex_number);
    }

    for (size_t i = 0; i < batch_size; ++i) {
        batch_average += batch_size;
    }
    batch_average /= batch_size;

    pthread_mutex_unlock(&shared_mutex);

    return batch_average;
}

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
    if (memory_limit < 128) {
        std::cerr << "Количество ОЗУ должно быть не меньше 128 байт.\n";
        return 1;
    }
    if (memory_limit % 64 != 0) {
        std::cerr << "Количество ОЗУ должно нацело делиться на 64 (минимум 128 байт).\n";
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

    size_t count;
    input >> count;

    size_t batch_size = memory_limit / 64;

    

    return 0;
}