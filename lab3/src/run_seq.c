#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    pid_t child_pid = fork();

    if (child_pid < 0) {
        perror("fork failed");
        return 1;
    }

    if (child_pid == 0) {
        // Дочерний процесс → запускаем sequential_min_max
        // argv[0] = ./run_seq, дальше идут параметры (--seed и т.п.)
        // Мы должны передать их sequential_min_max

        char **new_argv = malloc((argc + 1) * sizeof(char *));
        if (!new_argv) {
            perror("malloc failed");
            exit(1);
        }

        new_argv[0] = "./sequential_min_max"; // исполняемый файл
        for (int i = 1; i < argc; i++) {
            new_argv[i] = argv[i]; // копируем все параметры
        }
        new_argv[argc] = NULL; // конец массива аргументов

        execvp(new_argv[0], new_argv);

        // если exec вернулся → значит ошибка
        perror("exec failed");
        free(new_argv);
        exit(1);
    } else {
        // Родитель ждёт завершения ребёнка
        int status;
        waitpid(child_pid, &status, 0);

        if (WIFEXITED(status)) {
            printf("Child finished with code %d\n", WEXITSTATUS(status));
        } else {
            printf("Child process terminated abnormally\n");
        }
    }

    return 0;
}
