#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "string.h"
#include <sys/wait.h>
#include <errno.h>

int main(int argc, char** argv) {
    errno = 0;

    char* end;

    long pipe_fd_r = strtol(argv[1], &end, 10);
    if (end == argv[1] || *end != '\0' || errno == ERANGE) {
        perror("\nПроцессу был передан некорректный дескриптор!\n");
        return -1;
    }

    long pipe_fd_w = strtol(argv[2], &end, 10);
    if (end == argv[2] || *end != '\0' || errno == ERANGE) {
        perror("\nПроцессу был передан некорректный дескриптор!\n");
        return -1;
    }

    long file_fd = strtol(argv[3], &end, 10);
    if (end == argv[3] || *end != '\0' || errno == ERANGE) {
        perror("\nПроцессу был передан некорректный дескриптор!\n");
        return -1;
    }

    if (dup2(file_fd, STDOUT_FILENO) == -1) {
        perror("dup2");
        return -1;
    }
    close(file_fd);
    setvbuf(stdout, NULL, _IOLBF, 0);

    close(pipe_fd_w);

    char data[4096];
    while (1) {
        int result = read(pipe_fd_r, data, sizeof(data) - 1);

        if (result == -1) {
            usleep(100000);
        }
        else if (result == 0) {
            break;
        }
        else {
            int write_idx = 0;
            for (int read_idx = 0; read_idx < result; ++read_idx) {
                char c = data[read_idx];
                if (c != 'a' && c != 'e' && c != 'i' && c != 'o' && c != 'u' &&
                    c != 'A' && c != 'E' && c != 'I' && c != 'O' && c != 'U' &&
                    c != 'а' && c != 'е' && c != 'ё' && c != 'и' && c != 'о' && c != 'у' && c != 'ы' && c != 'э' && c != 'ю' && c != 'я' &&
                    c != 'А' && c != 'Е' && c != 'Ё' && c != 'И' && c != 'О' && c != 'У' && c != 'Ы' && c != 'Э' && c != 'Ю' && c != 'Я') {
                    data[write_idx] = data[read_idx];
                    write_idx++;
                }
            }

            data[write_idx] = '\0';
            printf("%s\n", data);
            fflush(stdout);

            memset(data, 0, 4096 * sizeof(char));
        }
    }

    close(pipe_fd_r);
}