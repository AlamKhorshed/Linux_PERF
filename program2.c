#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <malloc.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

#define NUM_OPERATIONS (1e8)
#define MEMSIZE (500 * 1024 * 1024) // 500 MB
#define PORT 12345
#define BUFSIZE 1024

void handle_error(const char* message) {
    perror(message);
    exit(EXIT_FAILURE);
}

// Function to perform CPU test
void cpu_test() {
    double x = 0.0;
    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);

    for (long i = 0; i < NUM_OPERATIONS; i++) {
        x = x + 1.0;
        x = x * 1.1;
        x = x / 1.1;
        x = x - 1.0;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1E9;
    printf("CPU Test Completed: %.2f seconds\n", time_taken);
}

// Function to perform Memory test
void memory_test() {
    struct timespec start, end;
    char *memory_block = (char *)malloc(MEMSIZE);
    if (!memory_block) {
        handle_error("Memory allocation failed");
    }

    clock_gettime(CLOCK_MONOTONIC, &start);

    memset(memory_block, 'A', MEMSIZE);
    volatile char temp;
    for (size_t i = 0; i < MEMSIZE; i++) {
        temp = memory_block[i];
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    free(memory_block);

    double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1E9;
    printf("Memory Test Completed: %.2f seconds\n", time_taken);
}

// Function to perform Network test
void* network_server(void* arg) {
    int server_fd, client_fd;
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    char buffer[BUFSIZE];

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        handle_error("Server socket creation failed");
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        handle_error("Bind failed");
    }

    if (listen(server_fd, 5) < 0) {
        handle_error("Listen failed");
    }

    if ((client_fd = accept(server_fd, (struct sockaddr *)&addr, &addr_len)) < 0) {
        handle_error("Accept failed");
    }

    while (recv(client_fd, buffer, BUFSIZE, 0) > 0);

    close(client_fd);
    close(server_fd);
    return NULL;
}

void network_test() {
    pthread_t server_thread;
    pthread_create(&server_thread, NULL, network_server, NULL);

    sleep(1); // Give the server time to set up

    int client_fd;
    struct sockaddr_in addr;
    char buffer[BUFSIZE];
    memset(buffer, 'A', BUFSIZE);

    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        handle_error("Client socket creation failed");
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (connect(client_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        handle_error("Connect failed");
    }
clock_gettime(CLOCK_MONOTONIC, &start);

    for (long i = 0; i < (MEMSIZE / BUFSIZE); i++) {
        if (send(client_fd, buffer, BUFSIZE, 0) < 0) {
            handle_error("Send failed");
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    close(client_fd);
    pthread_join(server_thread, NULL);

    double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1E9;
    printf("Network Test Completed: %.2f seconds\n", time_taken);
}

int main() {
    printf("Starting Performance Tests...\n");

    printf("Running CPU Test...\n");
    cpu_test();

    printf("Running Memory Test...\n");
    memory_test();

    printf("Running Network Test...\n");
    network_test();

    printf("Performance tests completed.\n");
    return 0;
}