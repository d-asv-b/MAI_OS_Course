#include <iostream>
#include <cstring>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <fstream>

#define BUFFER_SIZE 1024
#define MMAP_FILE_1 "mmap_child1"
#define MMAP_FILE_2 "mmap_child2"

struct SharedData {
    bool ready;
    bool done;
    char data[BUFFER_SIZE];
};

std::string removeLetters(const std::string& input) {
    std::string result;
    for (char c : input) {
        char lower_c = tolower(c);
        if (lower_c != 'a' && lower_c != 'e' && lower_c != 'i' && 
            lower_c != 'o' && lower_c != 'u' && lower_c != 'y') {
            result += c;
        }
    }
    return result;
}

void childProcess(int childNum, const std::string& outputFile, const char* mmapFile) {
    int fd = open(mmapFile, O_RDWR);
    if (fd == -1) {
        std::cerr << "Child: open mmap file failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    SharedData* shared_data = (SharedData*)mmap(nullptr, sizeof(SharedData),
                                                PROT_READ | PROT_WRITE, MAP_SHARED,
                                                fd, 0);
    if (shared_data == MAP_FAILED) {
        std::cerr << "Child: mmap failed" << std::endl;
        close(fd);

        exit(EXIT_FAILURE);
    }

    close(fd);

    std::ofstream outFile(outputFile, std::ios::app);
    if (!outFile.is_open()) {
        perror("Child: Cannot open output file");
        munmap(shared_data, sizeof(SharedData));
        exit(EXIT_FAILURE);
    }

    std::cout << "Child " << childNum << " started (PID: " << getpid() << ")" << std::endl;

    while (true) {
        while (!shared_data->ready && !shared_data->done) {
            sleep(0);
        }

        if (shared_data->done) {
            std::cout << "Child " << childNum << " finished" << std::endl;
            break;
        }

        std::string input(shared_data->data);
        std::string processed = removeLetters(input);

        outFile << processed << std::endl;
        outFile.flush();

        shared_data->ready = false;
    }

    outFile.close();

    munmap(shared_data, sizeof(SharedData));

    exit(EXIT_SUCCESS);
}

int main() {
    std::string file1, file2;

    std::cout << "Enter output file name for child process 1: ";
    std::cin >> file1;
    if (file1.empty()) {
        std::cerr << "Error: File name cannot be empty\n";
        return EXIT_FAILURE;
    }

    std::cout << "Enter output file name for child process 2: ";
    std::cin >> file2;
    if (file2.empty()) {
        std::cerr << "Error: File name cannot be empty\n";
        return EXIT_FAILURE;
    }

    // Очистка файлов
    std::ofstream outFile1(file1, std::ios::trunc);
    outFile1.close();
    std::ofstream outFile2(file2, std::ios::trunc);
    outFile2.close();

    unlink(MMAP_FILE_1);
    unlink(MMAP_FILE_2);

    int fd_1 = open(MMAP_FILE_1, O_CREAT | O_RDWR, 0666);
    int fd_2 = open(MMAP_FILE_2, O_CREAT | O_RDWR, 0666);

    if (fd_1 == -1 || fd_2 == -1) {
        std::cerr << "Parent: open mmap file failed" << std::endl;
        return EXIT_FAILURE;
    }

    if (
        ftruncate(fd_1, sizeof(SharedData)) == -1 ||
        ftruncate(fd_2, sizeof(SharedData)) == -1
    ) {
        std::cerr << "Parent: ftruncate failed" << std::endl;

        close(fd_1);
        close(fd_2);

        return EXIT_FAILURE;
    }

    SharedData* shared_data_1 = (SharedData*)mmap(nullptr, sizeof(SharedData),
                                                   PROT_READ | PROT_WRITE, MAP_SHARED,
                                                   fd_1, 0);
    SharedData* shared_data_2 = (SharedData*)mmap(nullptr, sizeof(SharedData),
                                                   PROT_READ | PROT_WRITE, MAP_SHARED,
                                                   fd_2, 0);

    if (shared_data_1 == MAP_FAILED || shared_data_2 == MAP_FAILED) {
        std::cerr << "Parent: mmap failed" << std::endl;

        close(fd_1);
        close(fd_2);

        return EXIT_FAILURE;
    }

    memset(shared_data_1, 0, sizeof(SharedData));
    memset(shared_data_2, 0, sizeof(SharedData));

    shared_data_1->ready = shared_data_1->done = shared_data_2->ready = shared_data_2->done = false;

    close(fd_1);
    close(fd_2);

    pid_t pid1 = fork();
    if (pid1 == -1) {
        std::cerr << "Parent: first fork failed" << std::endl;
        return EXIT_FAILURE;
    }

    if (pid1 == 0) {
        childProcess(1, file1, MMAP_FILE_1);
    }

    pid_t pid2 = fork();
    if (pid2 == -1) {
        std::cerr << "Parent: second fork failed" << std::endl;
        return EXIT_FAILURE;
    }

    if (pid2 == 0) {
        childProcess(2, file2, MMAP_FILE_2);
    }

    sleep(1);
    std::cout << "\n=== Main process ===\n";
    std::cout << "Enter text:\n";
    std::cout << "Type '~quit' to exit" << std::endl;
    std::cout << "> ";

    std::string input;
    while (true) {
        if (!std::getline(std::cin, input)) {
            break;
        }

        if (input == "~quit") {
            break;
        }

        if (input.empty()) {
            continue;
        }

        int length = input.length();
        if (length % 2 == 1) {
            strncpy(shared_data_1->data, input.c_str(), BUFFER_SIZE - 1);
            shared_data_1->data[BUFFER_SIZE - 1] = '\0';
            shared_data_1->ready = true;

            while (shared_data_1->ready) {
                sleep(0);
            }
        } else {
            strncpy(shared_data_2->data, input.c_str(), BUFFER_SIZE - 1);
            shared_data_2->data[BUFFER_SIZE - 1] = '\0';
            shared_data_2->ready = true;

            while (shared_data_2->ready) {
                sleep(0);
            }
        }

        std::cout << "> ";
    }

    shared_data_1->done = true;
    shared_data_2->done = true;

    int status1, status2;
    if (waitpid(pid1, &status1, 0) == -1) {
        std::cerr << "Parent: waitpid for child 1 failed" << std::endl;
    }
    if (waitpid(pid2, &status2, 0) == -1) {
        std::cerr << "Parent: waitpid for child 2 failed" << std::endl;
    }

    munmap(shared_data_1, sizeof(SharedData));
    munmap(shared_data_2, sizeof(SharedData));

    unlink(MMAP_FILE_1);
    unlink(MMAP_FILE_2);

    std::cout << "\nParent process finished!" << std::endl;
    return EXIT_SUCCESS;
}
