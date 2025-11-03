#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <getopt.h>

typedef struct {
    int start;
    int end;
    long long *result;
    int mod;
    pthread_mutex_t *mutex;
} thread_data_t;

void* calculate_factorial_part(void* arg) {
    thread_data_t* data = (thread_data_t*)arg;
    long long partial_result = 1;
    
    for (int i = data->start; i <= data->end; i++) {
        partial_result = (partial_result * i) % data->mod;
    }
    
    pthread_mutex_lock(data->mutex);
    *(data->result) = (*(data->result) * partial_result) % data->mod;
    pthread_mutex_unlock(data->mutex);
    
    return NULL;
}

int main(int argc, char* argv[]) {
    int k = 0;
    int pnum = 1;
    int mod = 1;
    
    static struct option long_options[] = {
        {"k", required_argument, 0, 'k'},
        {"pnum", required_argument, 0, 'p'},
        {"mod", required_argument, 0, 'm'},
        {0, 0, 0, 0}
    };
    
    int option_index = 0;
    int c;
    while ((c = getopt_long(argc, argv, "k:p:m:", long_options, &option_index)) != -1) {
        switch (c) {
            case 'k':
                k = atoi(optarg);
                break;
            case 'p':
                pnum = atoi(optarg);
                break;
            case 'm':
                mod = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s -k <number> --pnum=<threads> --mod=<modulus>\n", argv[0]);
                exit(1);
        }
    }
    
    if (k <= 0 || pnum <= 0 || mod <= 0) {
        fprintf(stderr, "All parameters must be positive integers\n");
        exit(1);
    }
    
    long long result = 1;
    pthread_t threads[pnum];
    thread_data_t thread_data[pnum];
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    
    int numbers_per_thread = k / pnum;
    int remainder = k % pnum;
    int current_start = 1;
    
    for (int i = 0; i < pnum; i++) {
        int current_end = current_start + numbers_per_thread - 1;
        if (i < remainder) {
            current_end++;
        }
        
        thread_data[i].start = current_start;
        thread_data[i].end = current_end;
        thread_data[i].result = &result;
        thread_data[i].mod = mod;
        thread_data[i].mutex = &mutex;
        
        pthread_create(&threads[i], NULL, calculate_factorial_part, &thread_data[i]);
        
        current_start = current_end + 1;
    }
    
    for (int i = 0; i < pnum; i++) {
        pthread_join(threads[i], NULL);
    }
    
    pthread_mutex_destroy(&mutex);
    
    printf("%d! mod %d = %lld\n", k, mod, result);
    
    return 0;
}