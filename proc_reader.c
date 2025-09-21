#include "proc_reader.h"

// Function to list process directories inside /proc
int list_process_directories(void) {
    DIR *dir;
    struct dirent *entry;
    int count = 0;

    // Open the /proc directory
    dir = opendir("/proc");
    if (dir == NULL) {
        perror("opendir failed");
        return -1;
    }

    printf("Process directories in /proc:\n");
    printf("%-8s %-20s\n", "PID", "Type");
    printf("%-8s %-20s\n", "---", "----");

    // Loop through all entries in /proc
    while ((entry = readdir(dir)) != NULL) {
        // Check if the name is just numbers (so it's a PID)
        if (is_number(entry->d_name)) {
            printf("%-8s %-20s\n", entry->d_name, "process");
            count++;
        }
    }

    // Close the directory
    if (closedir(dir) == -1) {
        perror("closedir failed");
        return -1;
    }

    printf("Found %d process directories\n", count);
    printf("SUCCESS: Process directories listed!\n");

    return 0;
}

// Function to read process info (status + cmdline)
int read_process_info(const char* pid) {
    char filepath[256];

    printf("\n--- Process Information for PID %s ---\n", pid);

    // Build path to /proc/[pid]/status
    snprintf(filepath, sizeof(filepath), "/proc/%s/status", pid);
    if (read_file_with_syscalls(filepath) == -1) {
        fprintf(stderr, "Failed to read %s\n", filepath);
        return -1;
    }

    // Build path to /proc/[pid]/cmdline
    snprintf(filepath, sizeof(filepath), "/proc/%s/cmdline", pid);
    printf("\n--- Command Line ---\n");
    if (read_file_with_syscalls(filepath) == -1) {
        fprintf(stderr, "Failed to read %s\n", filepath);
        return -1;
    }

    printf("\nSUCCESS: Process information read!\n");
    return 0;
}

// Function to show CPU + memory info
int show_system_info(void) {
    const int MAX_LINES = 10;
    int line_count = 0;
    char line[256];
    FILE *file;

    printf("\n--- CPU Information (first %d lines) ---\n", MAX_LINES);
    file = fopen("/proc/cpuinfo", "r");
    if (file == NULL) {
        perror("fopen failed (cpuinfo)");
        return -1;
    }
    while (fgets(line, sizeof(line), file) != NULL && line_count < MAX_LINES) {
        printf("%s", line);
        line_count++;
    }
    fclose(file);

    printf("\n--- Memory Information (first %d lines) ---\n", MAX_LINES);
    line_count = 0;
    file = fopen("/proc/meminfo", "r");
    if (file == NULL) {
        perror("fopen failed (meminfo)");
        return -1;
    }
    while (fgets(line, sizeof(line), file) != NULL && line_count < MAX_LINES) {
        printf("%s", line);
        line_count++;
    }
    fclose(file);

    printf("SUCCESS: System information displayed!\n");
    return 0;
}

// Function to compare system calls vs library functions
void compare_file_methods(void) {
    const char* test_file = "/proc/version";

    printf("Comparing file reading methods for: %s\n\n", test_file);

    printf("=== Method 1: Using System Calls ===\n");
    read_file_with_syscalls(test_file);

    printf("\n=== Method 2: Using Library Functions ===\n");
    read_file_with_library(test_file);

    printf("\nNOTE: Run this program with strace to see the difference!\n");
    printf("Example: strace -e trace=openat,read,write,close ./lab2\n");
}

// Read file using system calls (low-level)
int read_file_with_syscalls(const char* filename) {
    int fd;
    char buffer[1024];
    ssize_t bytes_read;

    fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("open failed");
        return -1;
    }

    // Read until there is no more data
    while ((bytes_read = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0'; // Null-terminate so printf works
        printf("%s", buffer);
    }

    if (bytes_read == -1) {
        perror("read failed");
        close(fd);
        return -1;
    }

    if (close(fd) == -1) {
        perror("close failed");
        return -1;
    }

    return 0;
}

// Read file using library functions (high-level)
int read_file_with_library(const char* filename) {
    FILE *file;
    char line[256];

    file = fopen(filename, "r");
    if (file == NULL) {
        perror("fopen failed");
        return -1;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        printf("%s", line);
    }

    if (fclose(file) == EOF) {
        perror("fclose failed");
        return -1;
    }

    return 0;
}

// Check if string is all digits (used for PID detection)
int is_number(const char* str) {
    if (str == NULL || *str == '\0') {
        return 0;
    }
    while (*str) {
        if (!isdigit((unsigned char)*str)) {
            return 0;
        }
        str++;
    }
    return 1;
}
