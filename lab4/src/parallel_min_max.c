#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

volatile sig_atomic_t timeout_occurred = 0;
pid_t* child_pids = NULL;
int child_count = 0;

void timeout_handler(int sig) {
    timeout_occurred = 1;
    printf("Timeout occurred! Sending SIGKILL to child processes...\n");
    
    for (int i = 0; i < child_count; i++) {
        if (child_pids[i] > 0) {
            kill(child_pids[i], SIGKILL);
        }
    }
}

int main(int argc, char **argv) {
    int seed = -1;
    int array_size = -1;
    int pnum = -1;
    int timeout = 0;
    bool with_files = false;

    while (true) {
        int current_optind = optind ? optind : 1;

        static struct option options[] = {
            {"seed", required_argument, 0, 0},
            {"array_size", required_argument, 0, 0},
            {"pnum", required_argument, 0, 0},
            {"by_files", no_argument, 0, 'f'},
            {"timeout", required_argument, 0, 0},
            {0, 0, 0, 0}
        };

        int option_index = 0;
        int c = getopt_long(argc, argv, "f", options, &option_index);

        if (c == -1) break;

        switch (c) {
            case 0:
                switch (option_index) {
                    case 0:
                        seed = atoi(optarg);
                        if (seed <= 0) {
                            printf("Seed must be > 0\n");
                            return 1;
                        }
                        break;
                    case 1:
                        array_size = atoi(optarg);
                        if (array_size <= 0) {
                            printf("Array size must be > 0\n");
                            return 1;
                        }
                        break;
                    case 2:
                        pnum = atoi(optarg);
                        if (pnum <= 0) {
                            printf("Pnum must be > 0\n");
                            return 1;
                        }
                        break;
                    case 3:
                        with_files = true;
                        break;
                    case 4:
                        timeout = atoi(optarg);
                        if (timeout <= 0) {
                            printf("Timeout must be > 0\n");
                            return 1;
                        }
                        break;
                    default:
                        printf("Index %d is out of options\n", option_index);
                }
                break;
            case 'f':
                with_files = true;
                break;
            case '?':
                break;
            default:
                printf("getopt returned character code 0%o?\n", c);
        }
    }

    if (optind < argc) {
        printf("Has at least one no option argument\n");
        return 1;
    }

    if (seed == -1 || array_size == -1 || pnum == -1) {
        printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" [--timeout \"seconds\"]\n",
               argv[0]);
        return 1;
    }

    child_pids = malloc(sizeof(pid_t) * pnum);
    child_count = pnum;

    if (timeout > 0) {
        signal(SIGALRM, timeout_handler);
        alarm(timeout);
    }

    int *array = malloc(sizeof(int) * array_size);
    GenerateArray(array, array_size, seed);

    int pipefd[pnum][2];
    if (!with_files) {
        for (int i = 0; i < pnum; i++) {
            if (pipe(pipefd[i]) == -1) {
                perror("pipe");
                return 1;
            }
        }
    }

    int active_child_processes = 0;
    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    int chunk = array_size / pnum;

    for (int i = 0; i < pnum; i++) {
        pid_t child_pid = fork();
        if (child_pid >= 0) {
            active_child_processes += 1;
            child_pids[i] = child_pid; 
            
            if (child_pid == 0) {
                // child process
                int begin = i * chunk;
                int end = (i == pnum - 1) ? array_size : (i + 1) * chunk;

                struct MinMax local_min_max = GetMinMax(array, begin, end);

                if (with_files) {
                    char filename[256];
                    sprintf(filename, "proc_%d.txt", i);
                    FILE *fp = fopen(filename, "w");
                    if (fp == NULL) {
                        perror("fopen");
                        exit(1);
                    }
                    fprintf(fp, "%d %d\n", local_min_max.min, local_min_max.max);
                    fclose(fp);
                } else {
                    close(pipefd[i][0]); // close read end
                    write(pipefd[i][1], &local_min_max, sizeof(struct MinMax));
                    close(pipefd[i][1]);
                }
                free(array);
                exit(0);
            }
        } else {
            printf("Fork failed!\n");
            return 1;
        }
    }


    while (active_child_processes > 0) {
        int status;
        pid_t finished_pid = waitpid(-1, &status, WNOHANG);
        
        if (finished_pid > 0) {
            active_child_processes -= 1;
            
            for (int i = 0; i < pnum; i++) {
                if (child_pids[i] == finished_pid) {
                    child_pids[i] = 0;
                    break;
                }
            }
        } else if (finished_pid == 0) {
            if (timeout_occurred) {
                usleep(10000); // 10ms
            } else {
                usleep(1000); // 1ms
            }
        } else {
            perror("waitpid");
            break;
        }
    }

    if (timeout > 0) {
        alarm(0);
    }

    struct MinMax min_max;
    min_max.min = INT_MAX;
    min_max.max = INT_MIN;

    for (int i = 0; i < pnum; i++) {
        int min = INT_MAX;
        int max = INT_MIN;

        if (with_files) {
            char filename[256];
            sprintf(filename, "proc_%d.txt", i);
            FILE *fp = fopen(filename, "r");
            if (fp != NULL) {
                fscanf(fp, "%d %d", &min, &max);
                fclose(fp);
                remove(filename); // удалить файл после чтения
            }
        } else {
            close(pipefd[i][1]); // close write end
            struct MinMax local_min_max;
            if (read(pipefd[i][0], &local_min_max, sizeof(struct MinMax)) == sizeof(struct MinMax)) {
                min = local_min_max.min;
                max = local_min_max.max;
            }
            close(pipefd[i][0]);
        }

        if (min < min_max.min) min_max.min = min;
        if (max > min_max.max) min_max.max = max;
    }

    struct timeval finish_time;
    gettimeofday(&finish_time, NULL);

    double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
    elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

    free(array);
    free(child_pids);

    printf("Min: %d\n", min_max.min);
    printf("Max: %d\n", min_max.max);
    printf("Elapsed time: %fms\n", elapsed_time);
    
    if (timeout_occurred) {
        printf("Program terminated by timeout after %d seconds\n", timeout);
    }
    
    fflush(NULL);
    return 0;
}