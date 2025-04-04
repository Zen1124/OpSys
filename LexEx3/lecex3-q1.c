#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/shm.h>
#include <unistd.h>

int lecex3_q1_child(int pipe_fd) {
    int shm_size, shm_key;
    
    if (read(pipe_fd, &shm_size, sizeof(int)) != sizeof(int) ||
        read(pipe_fd, &shm_key, sizeof(int)) != sizeof(int)) {
        fprintf(stderr, "Error: Failed to read from pipe\n");
        close(pipe_fd);
        return EXIT_FAILURE;
    }

    close(pipe_fd);

    int shm_id = shmget(shm_key, shm_size, 0666);
    if (shm_id == -1) {
        fprintf(stderr, "Error: Failed to access shared memory\n");
        return EXIT_FAILURE;
    }

    char *shm_ptr = (char *)shmat(shm_id, NULL, 0);
    if (shm_ptr == (char *)-1) {
        fprintf(stderr, "Error: Failed to attach to shared memory\n");
        return EXIT_FAILURE;
    }

    char *ptr = shm_ptr;
    char *end = shm_ptr + shm_size;
    while (ptr < end) {
        if (isupper(*ptr)) {
            *ptr = tolower(*ptr);
        } else if (isdigit(*ptr)) {
            *ptr = ' ';
        }
        ptr++;
    }

    if (shmdt(shm_ptr) == -1) {
        fprintf(stderr, "Error: Failed to detach from shared memory\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
