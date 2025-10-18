#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    printf("Создаю зомби-процесс...\n");
    
    pid_t pid = fork();
    
    if (pid == 0) {
        printf("Дочерний: PID=%d завершается\n", getpid());
        exit(0);
    } else {
        printf("ps aux | grep %d\n", pid);
        sleep(30);
        
        printf("wait()...\n");
        wait(NULL);
        printf("Зомбэ убран!\n");
    }
    
    return 0;
}