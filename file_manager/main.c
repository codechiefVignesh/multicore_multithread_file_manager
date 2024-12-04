// main.c
#include "file_manager.h"
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

// Structure for thread arguments
typedef struct {
    int thread_id;
    const char* filename;
    const char* content;
    int operation;  // 1: read, 2: write, 3: metadata, 4: copy, 5: compress, 6: decompress
} ThreadArgs;

// Thread function that handles multiple operations
void* file_operation_thread(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    char* buffer;
    size_t size;
    FileMetadata metadata;
    char dest_filename[256];
    
    printf("\nThread %d starting operation...\n", args->thread_id);
    
    switch(args->operation) {
        case 1: // Read
            if (read_file(args->filename, &buffer, &size) == 0) {
                printf("Thread %d: Successfully read %zu bytes from %s\n", 
                       args->thread_id, size, args->filename);
                free(buffer);
            } else {
                printf("Thread %d: Failed to read from %s\n", 
                       args->thread_id, args->filename);
            }
            break;
            
        case 2: // Write
            if (write_file(args->filename, args->content) == 0) {
                printf("Thread %d: Successfully wrote to %s\n", 
                       args->thread_id, args->filename);
            } else {
                printf("Thread %d: Failed to write to %s\n", 
                       args->thread_id, args->filename);
            }
            break;
            
        case 3: // Metadata
            if (get_file_metadata(args->filename, &metadata) == 0) {
                printf("Thread %d: Metadata for %s:\n", args->thread_id, args->filename);
                printf("  Size: %zu bytes\n", metadata.size);
                printf("  Created: %s", ctime(&metadata.creation_time));
                printf("  Permissions: %o\n", metadata.permissions);
            } else {
                printf("Thread %d: Failed to get metadata for %s\n", 
                       args->thread_id, args->filename);
            }
            break;
            
        case 4: // Copy
            sprintf(dest_filename, "%s_copy_%d", args->filename, args->thread_id);
            if (copy_file(args->filename, dest_filename) == 0) {
                printf("Thread %d: Successfully copied %s to %s\n", 
                       args->thread_id, args->filename, dest_filename);
            } else {
                printf("Thread %d: Failed to copy %s\n", 
                       args->thread_id, args->filename);
            }
            break;
            
        case 5: // Compress
            sprintf(dest_filename, "%s_compressed_%d.gz", args->filename, args->thread_id);
            if (compress_file(args->filename, dest_filename) == 0) {
                printf("Thread %d: Successfully compressed %s to %s\n", 
                       args->thread_id, args->filename, dest_filename);
            } else {
                printf("Thread %d: Failed to compress %s\n", 
                       args->thread_id, args->filename);
            }
            break;
            
        case 6: // Decompress
            sprintf(dest_filename, "%s_decompressed_%d", args->filename, args->thread_id);
            if (decompress_file(args->filename, dest_filename) == 0) {
                printf("Thread %d: Successfully decompressed %s to %s\n", 
                       args->thread_id, args->filename, dest_filename);
            } else {
                printf("Thread %d: Failed to decompress %s\n", 
                       args->thread_id, args->filename);
            }
            break;
    }
    
    return NULL;
}

int main() {
    init_file_manager();
    
    int num_threads;
    printf("Enter the number of threads to create: ");
    scanf("%d", &num_threads);
    
    if (num_threads <= 0) {
        printf("Invalid number of threads\n");
        return 1;
    }
    
    // Create initial test file
    const char* test_file = "test.txt";
    const char* initial_content = "This is the initial test file content.\n";
    write_file(test_file, initial_content);
    printf("Created initial test file: %s\n", test_file);
    
    // Allocate memory for threads and arguments
    pthread_t* threads = malloc(num_threads * sizeof(pthread_t));
    ThreadArgs* thread_args = malloc(num_threads * sizeof(ThreadArgs));
    
    // Get operation for each thread
    printf("\nOperation types:\n");
    printf("1: Read file\n");
    printf("2: Write to file\n");
    printf("3: Get file metadata\n");
    printf("4: Copy file\n");
    printf("5: Compress file\n");
    printf("6: Decompress file\n");
    
    for (int i = 0; i < num_threads; i++) {
        thread_args[i].thread_id = i + 1;
        thread_args[i].filename = test_file;
        thread_args[i].content = "This is new content written by a thread.\n";
        
        printf("\nEnter operation for thread %d (1-6): ", i + 1);
        scanf("%d", &thread_args[i].operation);
        
        if (thread_args[i].operation < 1 || thread_args[i].operation > 6) {
            printf("Invalid operation. Using read operation (1) as default.\n");
            thread_args[i].operation = 1;
        }
    }
    
    // Create and start all threads
    printf("\nStarting threads...\n");
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, file_operation_thread, &thread_args[i]);
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("\nAll threads completed.\n");
    printf("Check file_operations.log for detailed operation history.\n");
    
    // Cleanup
    free(threads);
    free(thread_args);
    cleanup_file_manager();
    
    return 0;
}