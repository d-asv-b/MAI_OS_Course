#include "stdio.h"
#include "fcntl.h"
#include "stdlib.h"
#include "unistd.h"
#include "string.h"
#include "sys/wait.h"

int main() {
    int pipe_fd_proc_1[2], pipe_fd_proc_2[2];

    if (pipe(pipe_fd_proc_1) == -1 || pipe(pipe_fd_proc_2) == -1) {
        perror("Не удалось создать pipe!");
        return -1;
    }

    char filename1[255], filename2[255], data[4096];
    printf("Введите имя файла для ПЕРВОГО процесса:\n");
    scanf("%s", filename1);

    printf("\nВведите имя файла для ВТОРОГО процесса:\n");
    scanf("%s", filename2);

    char* name1 = strcat(filename1, ".txt");
    char* name2 = strcat(filename2, ".txt");

    int file_fd_proc_1 = open(name1, O_WRONLY | O_CREAT, 0644);

    if (strcmp(name1, name2) == 0) {
        printf("\nИмена файлов должны быть разными!\n");
        return -1;
    }

    pid_t pid_1 = fork();
    if (pid_1 < 0) {
        perror("Не удалось создать дочерний процесс!");
        return -1;
    }

    if (pid_1 == 0) {
        char pipe_fd_1_r_str[16];
        sprintf(pipe_fd_1_r_str, "%d", pipe_fd_proc_1[0]);

        char pipe_fd_1_w_str[16];
        sprintf(pipe_fd_1_w_str, "%d", pipe_fd_proc_2[1]);
        
        char file_fd_1_str[16];
        sprintf(file_fd_1_str, "%d", file_fd_proc_1);

        execl("./child_exe", "child_proc_1", pipe_fd_1_r_str, pipe_fd_1_w_str, file_fd_1_str);

        perror("child_1");
        return -1;
    }

    int file_fd_proc_2 = open(name2, O_WRONLY | O_CREAT, 0644);

    pid_t pid_2 = fork();
    if (pid_2 < 0) {
        perror("Не удалось создать дочерний процесс!");
        return -1;
    }

    if (pid_2 == 0) {
        char pipe_fd_2_r_str[16];
        sprintf(pipe_fd_2_r_str, "%d", pipe_fd_proc_2[0]);

        char pipe_fd_2_w_str[16];
        sprintf(pipe_fd_2_w_str, "%d", pipe_fd_proc_2[1]);

        char file_fd_2_str[16];
        sprintf(file_fd_2_str, "%d", file_fd_proc_2);

        execl("./child_exe", "child_proc_2", pipe_fd_2_r_str, pipe_fd_2_w_str, file_fd_2_str);

        perror("child_2");
        return -1;
    }

    close(pipe_fd_proc_1[0]);
    close(pipe_fd_proc_2[0]);
    while(1) {
        printf("\nВведите строку:\n");
        
        if (scanf("%s", data) == EOF) {
            break;
        }

        size_t length = strlen(data);
        if (length % 2 != 0) {
            write(pipe_fd_proc_1[1], data, length * sizeof(char));
        }
        else {
            write(pipe_fd_proc_2[1], data, length * sizeof(char));            
        }
    }

    printf("\n\nFINISHING UP\n");

    close(pipe_fd_proc_1[1]);
    close(pipe_fd_proc_2[1]);

    close(file_fd_proc_1);
    close(file_fd_proc_2);

    return 0;
}