#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

extern void *copy_file(void *arg);

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("MAIN: Successfully copied 0 bytes via 0 child threads\n");
        return EXIT_SUCCESS;
    }

    int num_threads = argc - 1;
    pthread_t *threads = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
    int **bytes_copied = (int **)malloc(num_threads * sizeof(int *));
    int total_bytes = 0;

    pthread_t *t_ptr = threads;
    int **b_ptr = bytes_copied;
    char **arg_ptr = argv + 1;

    while (t_ptr < threads + num_threads) {
        printf("MAIN: Creating thread to copy \"%s\"\n", *arg_ptr);
        if (pthread_create(t_ptr, NULL, copy_file, *arg_ptr) != 0) {
            perror("MAIN: pthread_create() failed");
            return EXIT_FAILURE;
        }
        t_ptr++;
        arg_ptr++;
    }

    t_ptr = threads;
    b_ptr = bytes_copied;
    arg_ptr = argv + 1;

    while (t_ptr < threads + num_threads) {
        if (pthread_join(*t_ptr, (void **)b_ptr) != 0) {
            perror("MAIN: pthread_join() failed");
            return EXIT_FAILURE;
        }
        printf("MAIN: Thread completed copying %d bytes for \"%s\"\n", **b_ptr, *arg_ptr);
        total_bytes += **b_ptr;
        free(*b_ptr);
        t_ptr++;
        b_ptr++;
        arg_ptr++;
    }

    printf("MAIN: Successfully copied %d bytes via %d child thread%s\n", total_bytes, num_threads, num_threads > 1 ? "s" : "");

    free(threads);
    free(bytes_copied);
    return EXIT_SUCCESS;
}
