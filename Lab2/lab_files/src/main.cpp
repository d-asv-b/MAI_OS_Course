#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <boost/multiprecision/cpp_int.hpp>

#include <pthread.h>
#include <unistd.h>

using namespace boost::multiprecision;

// Общие данные для всех потоков
std::ifstream shared_input;
uint512_t total_sum = 0;
size_t total_processed_count = 0;
size_t total_count = 0;
size_t batch_size = 0;

pthread_mutex_t file_mutex;
pthread_mutex_t sum_mutex;

void* worker_function(void* arg) {
    uint512_t* local_data = new (std::nothrow) uint512_t[batch_size];
    if (local_data == nullptr) {
        std::cerr << "Ошибка: не удалось выделить память для буфера потока.\n";
        return (void*)1;
    }

    uint512_t local_sum = 0;
    std::string hex_number;

    while (true) {
        size_t current_batch_size = 0;

        pthread_mutex_lock(&file_mutex);
        if (total_processed_count >= total_count) {
            pthread_mutex_unlock(&file_mutex);
            break;
        }

        current_batch_size = std::min(batch_size, total_count - total_processed_count);
        
        for (size_t i = 0; i < current_batch_size; ++i) {
            if (!(shared_input >> hex_number)) {
                current_batch_size = i;
                total_processed_count = total_count;
                break; 
            }

            if (hex_number.substr(0, 2) != "0x") {
                hex_number = "0x" + hex_number;
            }
            local_data[i] = uint512_t(hex_number);
        }
        
        total_processed_count += current_batch_size;
        pthread_mutex_unlock(&file_mutex);

        for (size_t i = 0; i < current_batch_size; ++i) {
            local_sum += local_data[i];
        }
    }

    pthread_mutex_lock(&sum_mutex);
    total_sum += local_sum;
    pthread_mutex_unlock(&sum_mutex);

    delete[] local_data;

    return (void*)0;
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::cerr << "Использование: " << argv[0] << " --threads <N> --memory <bytes>\n";
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
    std::cout << "Лимит ОЗУ на батч: " << memory_limit << " байт\n";
    std::cout << "Размер 1 числа:    " << num_size << " байт\n";
    std::cout << "Размер батча (чисел): " << batch_size << "\n";
    std::cout << "-------------------------\n";

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

    pthread_mutex_init(&file_mutex, NULL);
    pthread_mutex_init(&sum_mutex, NULL);

    std::vector<pthread_t> threads(thread_count);

    for (size_t i = 0; i < thread_count; ++i) {
        pthread_create(&threads[i], NULL, worker_function, NULL);
    }

    std::cout << "Потоки созданы и работают. (Всего " << thread_count + 1 << " потоков в процессе).\n";
    std::cout << "Нажмите Enter для ожидания завершения и расчета результата...";
    std::cin.get();

    for (size_t i = 0; i < thread_count; ++i) {
        pthread_join(threads[i], NULL);
    }

    std::cout << "Все потоки завершили работу.\n";

    shared_input.close();
    pthread_mutex_destroy(&file_mutex);
    pthread_mutex_destroy(&sum_mutex);

    if (total_sum == 0 && total_count > 0) {
        std::cout << "Результат: 0\n";
    } else {
        uint1024_t average = (total_sum + total_count / 2) / total_count;
        std::cout << "=========================\n";
        std::cout << "Среднее арифм. (hex): 0x" << std::hex << average << "\n";
        std::cout << "Среднее арифм. (dec): " << std::dec << average << "\n";
        std::cout << "=========================\n";
    }

    return 0;
}