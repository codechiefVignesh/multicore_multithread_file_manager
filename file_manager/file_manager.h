// file_manager.h
#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <zlib.h>

#define MAX_PATH 256
#define MAX_READERS 10
#define LOG_FILE "file_operations.log"

// Structure to hold file metadata
typedef struct {
    char filename[MAX_PATH];
    size_t size;
    time_t creation_time;
    mode_t permissions;
} FileMetadata;

// Structure to manage read-write locks
typedef struct {
    pthread_rwlock_t lock;
    int reader_count;
    char filename[MAX_PATH];
} FileOperation;

// Function declarations
void init_file_manager(void);
void cleanup_file_manager(void);

// File operations
int read_file(const char* filename, char** buffer, size_t* size);
int write_file(const char* filename, const char* content);
int delete_file(const char* filename);
int rename_file(const char* old_name, const char* new_name);
int copy_file(const char* source, const char* destination);
int get_file_metadata(const char* filename, FileMetadata* metadata);
int compress_file(const char* source, const char* destination);
int decompress_file(const char* source, const char* destination);

// Logging
void log_operation(const char* operation, const char* filename, int status);

#endif
