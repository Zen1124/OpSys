#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>

int lecex2_child( unsigned int n ) {
    off_t offset = n - 1;
    int file_open = open("lecex2.txt", O_RDONLY);
    if (file_open == -1) {
        fprintf(stderr,"CHILD: Error: Opening file\n");
        close(file_open);
        abort();
    }
    int file_seek = lseek(file_open, offset, SEEK_CUR);
    //printf("%d\n", file_seek);

    if (file_seek == -1) {
        close(file_open);
        fprintf(stderr,"CHILD: File size too small\n");
        abort();
    }
    char buffer;
    int file_read = read(file_open, &buffer, 1); 
    //printf("%d\n", file_read);
    if (file_read < 1) {
        fprintf(stderr, "CHILD: Error while reading file\n");
        close(file_open);
        abort();
    }
    close(file_open);
    exit(buffer);
}
int lecex2_parent() {
    int status;
    pid_t pid = waitpid(-1, &status, 0);

    if (pid == -1) {
        fprintf(stderr,"PARENT: fork() failed\n");
        return EXIT_FAILURE;
    }

    if (WIFEXITED(status)) {
        char exit_status = WEXITSTATUS(status);
        printf("PARENT: child process exited with status \'%c\'\n", exit_status);
        return EXIT_SUCCESS;
    } else if (WIFSIGNALED(status)) {
        printf("PARENT: child process terminated abnormally!\n");
        return EXIT_FAILURE;
    } else {
        //printf("PARENT: child process terminated unexpectedly!\n");
        return EXIT_FAILURE;
    }
}