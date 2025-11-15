#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#include <getopt.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <pthread.h>

#include "common.h"

struct Server {
    char ip[255];
    int port;
};

struct ThreadData {
    struct Server server;
    uint64_t begin;
    uint64_t end;
    uint64_t mod;
    uint64_t result;
};

bool ConvertStringToUI64(const char *str, uint64_t *val) {
    char *end = NULL;
    unsigned long long i = strtoull(str, &end, 10);
    if (errno == ERANGE) {
        fprintf(stderr, "Out of uint64_t range: %s\n", str);
        return false;
    }

    if (errno != 0)
        return false;

    *val = i;
    return true;
}

void *ThreadServerCommunication(void *args) {
    struct ThreadData *data = (struct ThreadData *)args;
    
    struct sockaddr_in server_addr;
    int sck = socket(AF_INET, SOCK_STREAM, 0);
    if (sck < 0) {
        fprintf(stderr, "Socket creation failed!\n");
        data->result = 0;
        return NULL;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(data->server.port);
    
    if (inet_pton(AF_INET, data->server.ip, &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid IP address: %s\n", data->server.ip);
        close(sck);
        data->result = 0;
        return NULL;
    }

    if (connect(sck, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "Connection failed to %s:%d\n", data->server.ip, data->server.port);
        close(sck);
        data->result = 0;
        return NULL;
    }

    char request[sizeof(uint64_t) * 3];
    memcpy(request, &data->begin, sizeof(uint64_t));
    memcpy(request + sizeof(uint64_t), &data->end, sizeof(uint64_t));
    memcpy(request + 2 * sizeof(uint64_t), &data->mod, sizeof(uint64_t));

    if (send(sck, request, sizeof(request), 0) < 0) {
        fprintf(stderr, "Send failed\n");
        close(sck);
        data->result = 0;
        return NULL;
    }

    char response[sizeof(uint64_t)];
    if (recv(sck, response, sizeof(response), 0) < 0) {
        fprintf(stderr, "Recv failed\n");
        close(sck);
        data->result = 0;
        return NULL;
    }

    memcpy(&data->result, response, sizeof(uint64_t));
    close(sck);
    return NULL;
}

int main(int argc, char **argv) {
    uint64_t k = 0;
    uint64_t mod = 0;
    char servers[255] = {'\0'};

    while (true) {
        static struct option options[] = {
            {"k", required_argument, 0, 0},
            {"mod", required_argument, 0, 0},
            {"servers", required_argument, 0, 0},
            {0, 0, 0, 0}
        };

        int option_index = 0;
        int c = getopt_long(argc, argv, "", options, &option_index);

        if (c == -1)
            break;

        switch (c) {
        case 0: {
            switch (option_index) {
            case 0:
                if (!ConvertStringToUI64(optarg, &k)) {
                    fprintf(stderr, "Invalid k value: %s\n", optarg);
                    return 1;
                }
                break;
            case 1:
                if (!ConvertStringToUI64(optarg, &mod)) {
                    fprintf(stderr, "Invalid mod value: %s\n", optarg);
                    return 1;
                }
                break;
            case 2:
                strncpy(servers, optarg, sizeof(servers) - 1);
                servers[sizeof(servers) - 1] = '\0';
                break;
            default:
                printf("Index %d is out of options\n", option_index);
            }
        } break;

        case '?':
            printf("Unknown argument\n");
            break;
        default:
            fprintf(stderr, "getopt returned character code 0%o?\n", c);
        }
    }

    if (k == 0 || mod == 0 || !servers[0]) {
        fprintf(stderr, "Using: %s --k 1000 --mod 5 --servers /path/to/servers.txt\n", argv[0]);
        return 1;
    }

    FILE *servers_file = fopen(servers, "r");
    if (!servers_file) {
        fprintf(stderr, "Cannot open servers file: %s\n", servers);
        return 1;
    }

    struct Server *server_list = NULL;
    int server_count = 0;
    char buffer[256];

    while (fgets(buffer, sizeof(buffer), servers_file)) {
        char ip[255];
        int port;
        
        if (sscanf(buffer, "%254[^:]:%d", ip, &port) == 2) {
            server_list = realloc(server_list, (server_count + 1) * sizeof(struct Server));
            strncpy(server_list[server_count].ip, ip, sizeof(server_list[server_count].ip) - 1);
            server_list[server_count].ip[sizeof(server_list[server_count].ip) - 1] = '\0';
            server_list[server_count].port = port;
            server_count++;
        }
    }
    fclose(servers_file);

    if (server_count == 0) {
        fprintf(stderr, "No valid servers found in file\n");
        free(server_list);
        return 1;
    }

    pthread_t threads[server_count];
    struct ThreadData thread_data[server_count];
    
    uint64_t numbers_per_server = k / server_count;
    
    for (int i = 0; i < server_count; i++) {
        thread_data[i].server = server_list[i];
        thread_data[i].begin = 1 + i * numbers_per_server;
        thread_data[i].end = (i == server_count - 1) ? k : (i + 1) * numbers_per_server;
        thread_data[i].mod = mod;
        thread_data[i].result = 1;

        if (pthread_create(&threads[i], NULL, ThreadServerCommunication, (void *)&thread_data[i])) {
            fprintf(stderr, "Error: pthread_create failed!\n");
            free(server_list);
            return 1;
        }
    }

    uint64_t total = 1;
    for (int i = 0; i < server_count; i++) {
        pthread_join(threads[i], NULL);
        total = MultModulo(total, thread_data[i].result, mod);
    }

    printf("Final result: %lu! mod %lu = %lu\n", k, mod, total);
    
    free(server_list);
    return 0;
}