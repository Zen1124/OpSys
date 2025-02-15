#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>

int checkValidMove(int m, int n, int r, int c, int** board) {
    if (r < 0 || r >= m || c < 0 || c >= n) {
        return 0;
    }
    if (*(*(board+r)+c) != 0) {
        return 0;
    }
    return 1;
}

int checkClosed(int m, int n, int r, int c, int** board) {
    int newRow, newCol;
    for(int i = 1;i <= 8; i++) {
        if (i == 1) {
            newRow = r-2;
            newCol = c+1;
        } else if (i== 2) {
            newRow = r-1;
            newCol = c+2;
        } else if (i == 3) {
            newRow = r+1;
            newCol = c+2;
        } else if (i == 4) {
            newRow = r+2;
            newCol = c+1;
        } else if (i == 5) {
            newRow = r+2;
            newCol = c-1;
        } else if (i == 6) {
            newRow = r+1;
            newCol = c-2;
        } else if (i == 7) {
            newRow = r-1;
            newCol = c-2;
        } else if (i == 8) {
            newRow = r-2;
            newCol = c-1;
        }
        if (newRow >= 0 && newRow < m && newCol >= 0 && newCol < n) {
            if (*(*(board+newRow)+newCol) == 1) {
                return 1;
            }
        } 
    }
    return 0;
}

void makeMove(int m, int n, int r, int c, int** board, int moveCt, int pid, int* pipefd) {
    int newRow, newCol;

    //Mark move on board
    *(*(board + r) + c) = moveCt;

    //Prevent infinite loop
    if(moveCt > m*n) {
        exit(EXIT_FAILURE);
    }

    //Check for completion
    if (moveCt == m*n) {
        int closed = 0;
        close(*(pipefd));
        //printf("PID %d: Found an open knight's tour; notifying top-level parent\n", getpid());

        if (checkClosed(m,n,r,c,board)) {
            closed = 1;
            #ifndef QUIET
            printf("PID %d: Found an closed knight's tour; notifying top-level parent\n", getpid());
            #endif
        } else {
            #ifndef QUIET
            printf("PID %d: Found an open knight's tour; notifying top-level parent\n", getpid());
            #endif
        }
        write(*(pipefd+1), &closed, sizeof(int));
        
        for (int i = 0; i < m; i++) {
            free(*(board+i));
        }
        free(board);
        free(pipefd);
        exit(0);
    }

    //Count valid moves
    int moveOptions = 0;
    #ifdef DEBUG
    if(moveCt == 4 || moveCt == 3 || moveCt == 2) {
        printf("==PID %d: Move #%d: knight at row %d column %d\n", getpid(), moveCt, r, c);
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < n; j++) {
                printf("%d\t",*(*(board + i) + j));
            }
            printf("\n");
        }
        
    }
    #endif
    for(int i = 1;i <= 8; i++) {
        if (i == 1) {
            newRow = r-2;
            newCol = c+1;
        } else if (i== 2) {
            newRow = r-1;
            newCol = c+2;
        } else if (i == 3) {
            newRow = r+1;
            newCol = c+2;
        } else if (i == 4) {
            newRow = r+2;
            newCol = c+1;
        } else if (i == 5) {
            newRow = r+2;
            newCol = c-1;
        } else if (i == 6) {
            newRow = r+1;
            newCol = c-2;
        } else if (i == 7) {
            newRow = r-1;
            newCol = c-2;
        } else if (i == 8) {
            newRow = r-2;
            newCol = c-1;
        }
        if (checkValidMove(m,n,newRow,newCol,board)) {
            moveOptions++;
        } else if(moveCt == 4) {
            #ifdef DEBUG
                printf("==PID %d: Move #%d: Trying to place knight at row %d column %d\n", getpid(), moveCt, newRow, newCol);
            #endif
        }
    }
    int maxCoverage = moveCt;
    if (moveOptions == 0 && moveCt != m*n) { //No valid moves
        //write(*(pipefd+1), board, sizeof(board));
        #ifndef QUIET
        printf("PID %d: Dead end at move #%d\n", getpid(), moveCt);
        #endif
        for (int i = 0; i < m; i++) {
            free(*(board+i));
        }
        free(board);
        free(pipefd);
        exit(moveCt);
    } else {
        
        if (moveOptions > 1) {
            #ifndef QUIET
            printf("PID %d: %d possible moves after move #%d; creating %d child processes...\n", getpid(), moveOptions, moveCt, moveOptions);
            #endif
        }
        for(int i = 1;i <= 8; i++) {
            if (i == 1) {
                newRow = r-2;
                newCol = c+1;
            } else if (i== 2) {
                newRow = r-1;
                newCol = c+2;
            } else if (i == 3) {
                newRow = r+1;
                newCol = c+2;
            } else if (i == 4) {
                newRow = r+2;
                newCol = c+1;
            } else if (i == 5) {
                newRow = r+2;
                newCol = c-1;
            } else if (i == 6) {
                newRow = r+1;
                newCol = c-2;
            } else if (i == 7) {
                newRow = r-1;
                newCol = c-2;
            } else if (i == 8) {
                newRow = r-2;
                newCol = c-1;
            }
            if (checkValidMove(m,n,newRow,newCol,board)) {
                //One valid move, no fork
                if (moveOptions == 1) {
                    return makeMove(m,n,newRow,newCol,board,moveCt+1, getpid(), pipefd);
                    //exit(0);
                } else {
                    pid_t p1 = fork();
                    //Failed fork
                    if (p1 == -1) {
                        fprintf(stderr,"ERROR: fork() failed\n");
                        exit(EXIT_FAILURE);
                    }
                    //Child process
                    if (p1 == 0) {
                        makeMove(m,n, newRow, newCol, board, moveCt+1, getpid(), pipefd);
                        exit(0);
                    }
                    #ifdef NO_PARALLEL
                        int status;
                        waitpid(p1, &status, 0);   
                        if (WIFEXITED(status)) {
                            int coverage = WEXITSTATUS(status);
                            if (coverage > maxCoverage) {
                                maxCoverage = coverage;
                            }
                        }
                    #endif
                }  
            }
        }
        #ifndef NO_PARALLEL
            int processes = moveOptions;
            while(processes > 0) {
                int status;
                waitpid(-1, &status, 0);
                if (WIFEXITED(status)) {
                    int coverage = WEXITSTATUS(status);
                    if (coverage > maxCoverage) {
                        maxCoverage = coverage;
                    }
                    processes--;
                }
            }
        #endif
        for (int i = 0; i < m; i++) {
            free(*(board+i));
        }
        free(board);
        free(pipefd);
        if(maxCoverage > 0) { //Bypass recursion warning
            exit(maxCoverage);

        }
    }
}

int main(int argc, char** argv) {
    int m,n,r,c;
    setvbuf ( stdout, NULL, _IONBF, 0 ); 
    //Arguments check
    if (argc < 5) {
        fprintf(stderr,"ERROR: Invalid argument(s)\nUSAGE: ./hw2.out <m> <n> <r> <c>\n");
        return EXIT_FAILURE;
    } else {
        //Convert arguments to integers
        m = atoi(*(argv+1));
        n = atoi(*(argv+2));
        r = atoi(*(argv+3));
        c = atoi(*(argv+4));
        //Check if arguments are valid
        if (m < 2 || n < 2 || r < 0 || c < 0) {
            fprintf(stderr,"ERROR: Invalid argument(s)\nUSAGE: ./hw2.out <m> <n> <r> <c>\n");
            return EXIT_FAILURE;
        }
    }
    //m = 3;n=4;r=1;c=0;
    //Initialize board
    int** board = calloc(m, sizeof(int*));
    for (int i = 0; i < m; i++) {
        *(board+i) = calloc(n, sizeof(int));
    }

    //Place knight at starting position
    *(*(board+r)+c) = -1;

    #ifdef DEBUG
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            printf("%d\t",*(*(board + i) + j));
        }
        printf("\n");
    }
    #endif

    //Declare pipe
    int* pipefd = (int*) calloc(2, sizeof(int));

    //Error check pipe
    if(pipe(pipefd) == -1) {
        fprintf(stderr,"ERROR: pipe() failed\n");
        return EXIT_FAILURE;
    }

    
    printf("PID %d: Solving the knight's tour problem for a %dx%d board\n", getpid(), m, n);
    printf("PID %d: Starting at row %d and column %d (move #1)\n", getpid(), r, c);

    
    pid_t p1 = fork();
    if (p1 == 0) {
        //Child process
        close(*pipefd);
        makeMove(m,n,r,c,board,1,getpid(), pipefd);
    } else if (p1 > 0) {
        //Parent process
        close(*(pipefd + 1));
        int open = 0;
        int closed = 0;
        int tour;
        while (read(*(pipefd), &tour, sizeof(int)) > 0) {
            if (tour) {
                closed++;
            } else {
                open++;
            }
        }
        close(*(pipefd));
        close(*(pipefd + 1));
        int status;
        waitpid(p1, &status, 0);
        int maxCoverage = WEXITSTATUS(status);
        if (closed > 0 || open > 0) {
            printf("PID %d: Search complete; found %d open tours and %d closed tours\n", getpid(), open, closed);
        } else {
            printf("PID %d: Search complete; best solution(s) visited %d squares out of %d\n", getpid(), maxCoverage, m*n);
        }
    } else {
        fprintf(stderr,"ERROR: fork() failed\n");
        exit(0);
    }




    //Free board
    for (int i = 0; i < m; i++) {
        free(*(board+i));
    }
    free(board);
    free(pipefd);
    
    return 0;
}