#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

pthread_mutex_t write_lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    char file_path[MAX_PATH_LENGTH];
    char *data;
} write_args_t;

void *read_from_file(void *file_path) {
    char *path = (char *)file_path;
    FILE *file = fopen(path, "r");
    
    if (file == NULL) {
        perror("Error opening file for reading");
        return NULL;
    }

    printf("Reading from file: %s\n", path);

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        printf("%s", buffer);
    }

    fclose(file);
    return NULL;
}

void *write_to_file(void *args) {
    write_args_t *write_args = (write_args_t *)args;
    char *path = write_args->file_path;
    char *data = write_args->data;

    pthread_mutex_lock(&write_lock);

    FILE *file = fopen(path, "a");
    if (file == NULL) {
        perror("Error opening file for writing");
        pthread_mutex_unlock(&write_lock);
        return NULL;
    }

    printf("Writing to file: %s\n", path);

    if (fprintf(file, "%s\n", data) < 0) {
        perror("Error writing to file");
    }

    fclose(file);

    pthread_mutex_unlock(&write_lock);

    return NULL;
}
