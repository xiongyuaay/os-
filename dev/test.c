#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define DEVICE_PATH "/dev/mydev"
#define BUFFER_SIZE 256

void test_open_close() {
    int fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return;
    }
    printf("Device opened successfully\n");
    close(fd);
    printf("Device closed successfully\n");
}

void test_read() {
    int fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return;
    }

    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    ssize_t bytes_read = read(fd, buffer, BUFFER_SIZE);
    if (bytes_read < 0) {
        perror("Failed to read from device");
    } else {
        printf("Read %zd bytes: %s\n", bytes_read, buffer);
    }

    close(fd);
}

void test_write() {
    int fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return;
    }

    const char *data = "Hello, kernel device!";
    ssize_t bytes_written = write(fd, data, strlen(data));
    if (bytes_written < 0) {
        perror("Failed to write to device");
    } else {
        printf("Wrote %zd bytes: %s\n", bytes_written, data);
    }

    close(fd);
}

void test_llseek() {
    int fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return;
    }

    // Seek to position 10
    off_t new_pos = lseek(fd, 10, SEEK_SET);
    if (new_pos < 0) {
        perror("Failed to seek");
    } else {
        printf("Seeked to position: %lld\n", (long long)new_pos);
        char buffer[BUFFER_SIZE];
        memset(buffer, 0, BUFFER_SIZE);

        ssize_t bytes_read = read(fd, buffer, BUFFER_SIZE);
        if (bytes_read < 0) {
            perror("Failed to read from device");
        } else {
            printf("Read %zd bytes: %s\n", bytes_read, buffer);
        }
    }

    close(fd);
}

int main() {
    printf("Starting device tests...\n");

    printf("\nTesting open/close:\n");
    test_open_close();

    printf("\nTesting write:\n");
    test_write();

    printf("\nTesting read:\n");
    test_read();

    printf("\nTesting llseek:\n");
    test_llseek();

    printf("\nAll tests completed.\n");
    return 0;
}