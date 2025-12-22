#include <cstddef>
#include <cstring>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>

#include <pthread.h>
#include <unistd.h>
#include <mutex>

#include <boost/multiprecision/cpp_int.hpp>

using boost::multiprecision::uint512_t;
using boost::multiprecision::uint1024_t;

std::ifstream shared_input;
uint1024_t total_sum = 0;
size_t total_processed_count = 0;
size_t total_count = 0;
size_t batch_size = 0;

std::mutex file_mutex{};
std::mutex memory_mutex{};
std::mutex sum_mutex{};

auto start_time = std::chrono::high_resolution_clock::now();
auto end_time = std::chrono::high_resolution_clock::now();

//#define MEM_PER_THREAD

#ifndef MEM_PER_THREAD
uint512_t* memory = nullptr;
#endif

void* worker_func(void* arg) {
#ifdef MEM_PER_THREAD
    uint512_t* memory = new uint512_t[batch_size];
    if (memory == nullptr) {
        std::cerr << "Ошибка: не удалось выделить память для буфера потока.\n";
        return (void*)1;
    }
#endif

    std::string hex_number;

    while (true) {
        size_t current_batch_size = 0;

        file_mutex.lock();
        if (total_processed_count >= total_count) {
            file_mutex.unlock();
            break;
        }
        current_batch_size = std::min(batch_size, total_count - total_processed_count);

        memory_mutex.lock();
        for (size_t i = 0; i < current_batch_size; ++i) {
            if (!(shared_input >> hex_number)) {
                current_batch_size = i;
                total_processed_count = total_count;
                break;
            }

            if (hex_number.substr(0, 2) != "0x") {
                hex_number = "0x" + hex_number;
            }

            memory[i] = uint512_t(hex_number);
        }

        total_processed_count += current_batch_size;
        file_mutex.unlock();

        uint1024_t local_sum = 0;
        for (size_t i = 0; i < current_batch_size; ++i) {
            local_sum += memory[i];
        }
        memset(memory, 0, batch_size);
        memory_mutex.unlock();

        sum_mutex.lock();
        total_sum += local_sum;
        sum_mutex.unlock();
    }

#ifdef MEM_PER_THREAD
    delete [] memory;
#endif

    return (void*)0;
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::cerr << "Использование: " << argv[0] << " --threads <N> --memory <bits>\n";
        return 1;
    }

    size_t thread_count = 0;
    size_t memory_limit = 0;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--threads" || arg == "-t") {
            if (i + 1 < argc) {
                thread_count = std::stoul(argv[i + 1]);
                ++i;
            }
        } else if (arg == "--memory" || arg == "-m") {
            if (i + 1 < argc) {
                memory_limit = std::stoul(argv[i + 1]);
                ++i;
            }
        }
    }

    if (thread_count == 0) {
        std::cerr << "Ошибка: количество потоков должно быть больше нуля.\n";
        return 1;
    }
    size_t num_size = sizeof(uint512_t);
    if (memory_limit < num_size) {
        std::cerr << "Ошибка: лимит ОЗУ должен быть не меньше " << num_size << " байт (для одного числа).\n";
        return 1;
    }

    batch_size = memory_limit / num_size;
    if (batch_size == 0) {
         std::cerr << "Ошибка: при лимите ОЗУ " << memory_limit << " байт размер батча равен 0.\n";
         return 1;
    }

    std::cout << "Параметры запуска\n";
    std::cout << "Количество потоков: " << thread_count << "\n";
    std::cout << "Лимит ОЗУ на батч: " << memory_limit << " бит\n";
    std::cout << "Размер 1 числа:    " << num_size << " бит\n";
    std::cout << "Размер батча (чисел): " << batch_size << "\n";
    std::cout << "------------------------------------\n";

    std::string filename;
    std::cout << "Введите имя файла с числами: ";
    std::cin >> filename;

    shared_input.open(filename);
    if (!shared_input.is_open()) {
        std::cerr << "Не удалось открыть файл: " << filename << "\n";
        return 1;
    }

    // Общее количество чисел
    shared_input >> total_count;
    if (total_count == 0) {
        std::cerr << "В файле 0 чисел для обработки.\n";
        shared_input.close();
        return 0;
    }
    std::cout << "Ожидается " << total_count << " чисел.\n";

    std::cout << "PID процесса: " << getpid() << "\n";
    std::cout << "Сейчас запущен 1 (главный) поток.\n";

    std::cout << "Нажмите Enter для создания рабочих потоков...";
    std::cin.ignore();
    std::cin.get();

#ifndef MEM_PER_THREAD
    memory = new uint512_t[batch_size];
#endif

    std::vector<pthread_t> threads(thread_count);

    for (size_t i = 0; i < thread_count; ++i) {
        pthread_create(&threads[i], NULL, worker_func, NULL);
    }

    std::cout << "Потоки созданы и работают. (Всего " << thread_count + 1 << " потоков в процессе).\n";

    start_time = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < thread_count; ++i) {
        pthread_join(threads[i], NULL);
    }

    end_time = std::chrono::high_resolution_clock::now();
    std::cout << "Все потоки завершили работу.\n";

    shared_input.close();

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    if (total_sum == 0 && total_count > 0) {
        std::cout << "Результат: 0\n";
    } else {
        uint1024_t average = (total_sum + total_count / 2) / total_count;
        std::cout << "------------------------------------\n";
        std::cout << "Сумма: " << total_sum << "\n";
        std::cout << "Среднее арифм. (hex): 0x" << std::hex << average << "\n";
        std::cout << "Среднее арифм. (dec): " << std::dec << average << "\n";
        std::cout << "------------------------------------\n";
    }

    std::cout << "\n=== РЕЗУЛЬТАТЫ ПРОИЗВОДИТЕЛЬНОСТИ ===\n";
    std::cout << "Время обработки: " << elapsed.count() << " мс\n";
    std::cout << "Конфигурация: ";
#ifdef MEM_PER_THREAD
    std::cout << "MEM_PER_THREAD (память выделяется на поток)\n";
#else
    std::cout << "SHARED_MEMORY (общая память для всех потоков)\n";
#endif

#ifndef MEM_PER_THREAD
    delete [] memory;
#endif

    return 0;
}