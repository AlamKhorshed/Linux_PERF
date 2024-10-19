#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#define NUM_THREADS 4
#define MEMSIZE (500 * 1024 * 1024) // 500 MB
#define FILESIZE (100 * 1024 * 1024) // 100 MB
#define BUFSIZE (8 * 1024)           // 8 KB

void handle_error(const char* message) {
    perror(message);
    exit(EXIT_FAILURE);
}

// CPU stress function
void* cpu_stress(void* arg) {
    double x = 0.0;
    while (1) {
        for (long i = 0; i < 1e6; i++) {
            x = x + 1.0;
            x = x * 1.1;
            x = x / 1.1;
            x = x - 1.0;
        }
    }
    return NULL;
}

// Memory stress function
void* memory_stress(void* arg) {
    char *memory_block = (char *)malloc(MEMSIZE);
    if (!memory_block) {
        handle_error("Memory allocation failed");
    }

    while (1) {
        memset(memory_block, 'A', MEMSIZE);
        volatile char temp;
        for (size_t i = 0; i < MEMSIZE; i++) {
            temp = memory_block[i];
        }
    }

    free(memory_block);
    return NULL;
}

// Disk I/O stress function
void* disk_stress(void* arg) {
    const char *filename = "stress_test.data";
    int fd;
    char buffer[BUFSIZE];
    memset(buffer, 'A', BUFSIZE);

    if ((fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644)) < 0) {
        handle_error("Error opening file for writing");
    }

    while (1) {
        for (size_t total_written = 0; total_written < FILESIZE; total_written += BUFSIZE) {
            if (write(fd, buffer, BUFSIZE) < 0) {
                handle_error("Error writing to file");
            }
        }
        if (lseek(fd, 0, SEEK_SET) < 0) {
            handle_error("Error resetting file position");
        }
    }

    close(fd);
    return NULL;
}

int main() {
    pthread_t cpu_threads[NUM_THREADS];
    pthread_t mem_thread, disk_thread;

    // Create CPU stress threads
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&cpu_threads[i], NULL, cpu_stress, NULL) != 0) {
            handle_error("Error creating CPU stress thread");
        }
    }

    // Create Memory stress thread
    if (pthread_create(&mem_thread, NULL, memory_stress, NULL) != 0) {
        handle_error("Error creating memory stress thread");
    }

    // Create Disk I/O stress thread
    if (pthread_create(&disk_thread, NULL, disk_stress, NULL) != 0) {
        handle_error("Error creating disk stress thread");
    }

    // Join CPU stress threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(cpu_threads[i], NULL);
    }

    // Join Memory stress thread
    pthread_join(mem_thread, NULL);

    // Join Disk I/O stress thread
    pthread_join(disk_thread, NULL);

    return 0;
}