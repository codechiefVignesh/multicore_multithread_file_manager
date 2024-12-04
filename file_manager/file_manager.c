// file_manager.c
#include "file_manager.h"

static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
static FileOperation* file_ops;
static int max_files = 100;

void init_file_manager(void) {
    file_ops = (FileOperation*)malloc(sizeof(FileOperation) * max_files);
    for (int i = 0; i < max_files; i++) {
        pthread_rwlock_init(&file_ops[i].lock, NULL);
        file_ops[i].reader_count = 0;
        file_ops[i].filename[0] = '\0';
    }
}

void cleanup_file_manager(void) {
    for (int i = 0; i < max_files; i++) {
        pthread_rwlock_destroy(&file_ops[i].lock);
    }
    free(file_ops);
}

// Find or create file operation entry
static FileOperation* get_file_operation(const char* filename) {
    for (int i = 0; i < max_files; i++) {
        if (strlen(file_ops[i].filename) == 0) {
            strcpy(file_ops[i].filename, filename);
            return &file_ops[i];
        }
        if (strcmp(file_ops[i].filename, filename) == 0) {
            return &file_ops[i];
        }
    }
    return NULL;
}

int read_file(const char* filename, char** buffer, size_t* size) {
    FileOperation* op = get_file_operation(filename);
    if (!op) return -1;

    pthread_rwlock_rdlock(&op->lock);
    
    FILE* file = fopen(filename, "rb");
    if (!file) {
        pthread_rwlock_unlock(&op->lock);
        return -1;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate buffer and read file
    *buffer = (char*)malloc(*size + 1);
    size_t read_size = fread(*buffer, 1, *size, file);
    (*buffer)[read_size] = '\0';

    fclose(file);
    pthread_rwlock_unlock(&op->lock);
    log_operation("read", filename, 0);
    return 0;
}

int write_file(const char* filename, const char* content) {
    FileOperation* op = get_file_operation(filename);
    if (!op) return -1;

    pthread_rwlock_wrlock(&op->lock);
    
    FILE* file = fopen(filename, "wb");
    if (!file) {
        pthread_rwlock_unlock(&op->lock);
        return -1;
    }

    size_t written = fwrite(content, 1, strlen(content), file);
    fclose(file);
    
    pthread_rwlock_unlock(&op->lock);
    log_operation("write", filename, written == strlen(content) ? 0 : -1);
    return written == strlen(content) ? 0 : -1;
}

int delete_file(const char* filename) {
    FileOperation* op = get_file_operation(filename);
    if (!op) return -1;

    pthread_rwlock_wrlock(&op->lock);
    int result = remove(filename);
    pthread_rwlock_unlock(&op->lock);
    
    log_operation("delete", filename, result);
    return result;
}

int rename_file(const char* old_name, const char* new_name) {
    FileOperation* op_old = get_file_operation(old_name);
    FileOperation* op_new = get_file_operation(new_name);
    if (!op_old || op_new) return -1;

    pthread_rwlock_wrlock(&op_old->lock);
    int result = rename(old_name, new_name);
    pthread_rwlock_unlock(&op_old->lock);
    
    if (result == 0) {
        strcpy(op_old->filename, new_name);
    }
    log_operation("rename", old_name, result);
    return result;
}

int copy_file(const char* source, const char* destination) {
    FileOperation* op_src = get_file_operation(source);
    if (!op_src) return -1;

    pthread_rwlock_rdlock(&op_src->lock);
    
    FILE* src = fopen(source, "rb");
    FILE* dst = fopen(destination, "wb");
    
    if (!src || !dst) {
        if (src) fclose(src);
        if (dst) fclose(dst);
        pthread_rwlock_unlock(&op_src->lock);
        return -1;
    }

    char buffer[4096];
    size_t bytes;
    int status = 0;

    while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        if (fwrite(buffer, 1, bytes, dst) != bytes) {
            status = -1;
            break;
        }
    }

    fclose(src);
    fclose(dst);
    pthread_rwlock_unlock(&op_src->lock);
    
    log_operation("copy", source, status);
    return status;
}

int get_file_metadata(const char* filename, FileMetadata* metadata) {
    struct stat st;
    if (stat(filename, &st) != 0) {
        return -1;
    }

    strcpy(metadata->filename, filename);
    metadata->size = st.st_size;
    metadata->creation_time = st.st_ctime;
    metadata->permissions = st.st_mode;
    
    return 0;
}

int compress_file(const char* source, const char* destination) {
    FileOperation* op = get_file_operation(source);
    if (!op) return -1;

    pthread_rwlock_rdlock(&op->lock);
    
    FILE* src = fopen(source, "rb");
    gzFile dst = gzopen(destination, "wb");
    
    if (!src || !dst) {
        if (src) fclose(src);
        if (dst) gzclose(dst);
        pthread_rwlock_unlock(&op->lock);
        return -1;
    }

    char buffer[4096];
    size_t bytes;
    int status = 0;

    while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        if (gzwrite(dst, buffer, bytes) != bytes) {
            status = -1;
            break;
        }
    }

    fclose(src);
    gzclose(dst);
    pthread_rwlock_unlock(&op->lock);
    
    log_operation("compress", source, status);
    return status;
}

int decompress_file(const char* source, const char* destination) {
    FileOperation* op = get_file_operation(source);
    if (!op) return -1;

    pthread_rwlock_rdlock(&op->lock);
    
    gzFile src = gzopen(source, "rb");
    FILE* dst = fopen(destination, "wb");
    
    if (!src || !dst) {
        if (src) gzclose(src);
        if (dst) fclose(dst);
        pthread_rwlock_unlock(&op->lock);
        return -1;
    }

    char buffer[4096];
    int bytes;
    int status = 0;

    while ((bytes = gzread(src, buffer, sizeof(buffer))) > 0) {
        if (fwrite(buffer, 1, bytes, dst) != bytes) {
            status = -1;
            break;
        }
    }

    gzclose(src);
    fclose(dst);
    pthread_rwlock_unlock(&op->lock);
    
    log_operation("decompress", source, status);
    return status;
}

void log_operation(const char* operation, const char* filename, int status) {
    pthread_mutex_lock(&log_mutex);
    
    FILE* log = fopen(LOG_FILE, "a");
    if (log) {
        time_t now = time(NULL);
        char* timestamp = ctime(&now);
        timestamp[strlen(timestamp) - 1] = '\0';  // Remove newline
        
        fprintf(log, "[%s] Operation: %s, File: %s, Status: %d\n",
                timestamp, operation, filename, status);
        fclose(log);
    }
    
    pthread_mutex_unlock(&log_mutex);
}