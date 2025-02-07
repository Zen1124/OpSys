#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char **argv) {
    //struct stat st;

    if (argc < 2) {
        fprintf(stderr,"PARENT: Atleast 1 argument required\n");
        return 0;
    }

    pid_t p = fork();

    if ( p == -1 )
    {
        fprintf(stderr, "PARENT: fork() failed\n" );
        return EXIT_FAILURE;
    }

    if (p == 0) // Child Process
    {
        pid_t p1 = fork();
        if ( p1 == -1 )
        {
            fprintf(stderr, "CHILD: fork() failed\n" );
            abort();
        } else if (p1 == 0) {
            // grandchild process
            int bracket_count = 0;
            int level = 0;

            int file_open = open(*(argv + 1), O_RDONLY);
            if (file_open == -1) {
                fprintf(stderr,"GRANDCHILD: open() failed: No such file or directory\n");
                close(file_open);
                abort();
                return -1;
            }

            char buffer;
            
            while (read(file_open, &buffer, 1) == 1) {
                if (buffer == '{') {
                    level++;
                } else if (buffer == '}') {
                    if (level > 0) {
                        bracket_count++;
                        level--;
                    }
                }
                if (bracket_count > 127) {
                    fprintf(stderr,"GRANDCHILD: error in bracket count\n");
                    close(file_open);
                    return -1;
                }
            }
            close(file_open);

            if (bracket_count > 1 || bracket_count == 0) {
                printf("GRANDCHILD: counted %d properly nested pairs of curly brackets\n", bracket_count);
            } else {
                printf("GRANDCHILD: counted %d properly nested pair of curly brackets\n", bracket_count);
            }
            exit(bracket_count);

        } else {
            // orginal child
            int status;
            waitpid(p1, &status, 0);
            //printf("CHILD: received %d\n", WEXITSTATUS(status));

            if (WIFEXITED(status)) {
                int count = WEXITSTATUS(status);
                if (count == 255) {
                    fprintf(stderr,"CHILD: rcvd -1 (error)\n");
                    return -1;
                }
                count *= 2;
                printf("CHILD: doubled %d; returning %d\n", count / 2, count);
                exit(count);
            } else {
                fprintf(stderr,"CHILD: grandchild process terminated abnormally\n");
                abort();
            }

        }
    } else {
        int status;
        waitpid(p, &status, 0);
        if (WIFEXITED(status)) {
            int count = WEXITSTATUS(status);
            if (count == 255) {
                fprintf(stderr,"PARENT: rcvd -1 (error)\n");
                return -1;
            }
            printf("PARENT: there are %d properly nested curly brackets\n", count);
            return 0;
        } else {
            fprintf(stderr,"PARENT: child process terminated abnormally\n");
            abort();
            return -1;
        }
    }
    

    
    
}