#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <errno.h>

#define FILENAME "performance_test.data"
#define FILESIZE (100 * 1024 * 1024) // 100 MB
#define BUFSIZE (8 * 1024)           // 8 KB

void handle_error(const char* message) {
    perror(message);
    exit(EXIT_FAILURE);
}

void write_test() {
    int fd;
    char buffer[BUFSIZE];
    memset(buffer, 'A', BUFSIZE);
    
    // Open file for writing
    if ((fd = open(FILENAME, O_CREAT | O_WRONLY | O_TRUNC, 0644)) < 0) {
        handle_error("Error opening file for writing");
    }

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (size_t total_written = 0; total_written < FILESIZE; total_written += BUFSIZE) {
        if (write(fd, buffer, BUFSIZE) < 0) {
            handle_error("Error writing to file");
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    close(fd);

    double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1E9;
    printf("Write Test Completed in %.2f seconds\n", time_taken);
}

void read_test() {
    int fd;
    char buffer[BUFSIZE];

    // Open file for reading
    if ((fd = open(FILENAME, O_RDONLY)) < 0) {
        handle_error("Error opening file for reading");
    }

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (size_t total_read = 0; total_read < FILESIZE; total_read += BUFSIZE) {
        if (read(fd, buffer, BUFSIZE) < 0) {
            handle_error("Error reading from file");
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    close(fd);

    double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1E9;
    printf("Read Test Completed in %.2f seconds\n", time_taken);
}

int main() {
    printf("Starting Write Test...\n");
    write_test();

    printf("Starting Read Test...\n");
    read_test();

    printf("Performance test completed.\n");
    return 0;
}