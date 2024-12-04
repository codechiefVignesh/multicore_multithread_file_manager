# Multithreaded File Manager with Synchronization

A multithreaded file manager designed to perform synchronized file operations across threads and processes. This application supports a variety of file operations, including read, write, copy, metadata retrieval, compression, and decompression, while ensuring thread and process safety using POSIX semaphores and read-write locks.

---

## Features

- **Multithreaded Operations**  
  Perform concurrent file operations using threads with proper synchronization via read-write locks.

- **Process Synchronization**  
  Uses a named semaphore (`/tmp/file_manager.lock`) to ensure safe access across multiple processes.

- **Interactive Content Input**  
  Supports multiline content input for write operations, with a maximum of 1024 characters per file.

- **Rich File Operations**  
  - Reading and writing files.
  - Retrieving file metadata (size, permissions, creation time).
  - Copying, deleting, and renaming files.
  - Compressing and decompressing files using `zlib`.

- **Detailed Logging**  
  Logs all file operations in `file_operations.log` with timestamps for debugging and audit purposes.

---

## Setup Instructions

### Dependencies
Ensure the following libraries are installed:
- `pthread`
- `zlib`
- `semaphore.h`
- Standard C libraries (`stdio.h`, `stdlib.h`, etc.)

### Compilation
Use the following command to compile the project:
```bash
gcc -o file_manager main.c file_manager.c -lpthread -lz
