#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>

extern long next_thread_number;
extern int max_squares;
extern int total_open_tours;
extern int total_closed_tours;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct
{
    int m, n, r, c, moves;
    int **board;
    long thread_number;
} kdata;

typedef struct
{
    int r, c;
} new_move;

int checkValidMove(int m, int n, int r, int c, int **board)
{
    if (r < 0 || r >= m || c < 0 || c >= n)
    {
        return 0;
    }
    if (*(*(board + r) + c) != 0)
    {
        return 0;
    }
    return 1;
}

void free_board(int **board, int m) {
    for (int i = 0; i < m; i++) {
        free(*(board + i));
    }
    free(board);
}

new_move getNewMove(int i, int r, int c)
{
    new_move output = {0, 0};
    int newRow, newCol;
    if (i == 1)
    {
        newRow = r - 2;
        newCol = c + 1;
    }
    else if (i == 2)
    {
        newRow = r - 1;
        newCol = c + 2;
    }
    else if (i == 3)
    {
        newRow = r + 1;
        newCol = c + 2;
    }
    else if (i == 4)
    {
        newRow = r + 2;
        newCol = c + 1;
    }
    else if (i == 5)
    {
        newRow = r + 2;
        newCol = c - 1;
    }
    else if (i == 6)
    {
        newRow = r + 1;
        newCol = c - 2;
    }
    else if (i == 7)
    {
        newRow = r - 1;
        newCol = c - 2;
    }
    else if (i == 8)
    {
        newRow = r - 2;
        newCol = c - 1;
    } else {
        return output;
    }
    output.r = newRow;
    output.c = newCol;
    return output;
}

int checkClosed(int m, int n, int r, int c, int **board)
{
    int newRow, newCol;
    for (int i = 1; i <= 8; i++)
    {
        new_move next = getNewMove(i, r, c);
        newRow = next.r;
        newCol = next.c;
        if (newRow >= 0 && newRow < m && newCol >= 0 && newCol < n)
        {
            if (*(*(board + newRow) + newCol) == 1)
            {
                return 1;
            }
        }
    }
    return 0;
}

void *knight_tour(void *args)
{
    kdata *k_args = (kdata *)args;
    int m = k_args->m;
    int n = k_args->n;
    int r = k_args->r;
    int c = k_args->c;
    int moves = k_args->moves;
    int **board = k_args->board;
    long thread_number = k_args->thread_number;

    int on = 1;
    // While loop so no need to recurse
    while(on) {
        //Check if move count is greater than amount of squares
        if (moves == m * n) {
            pthread_mutex_lock(&mutex);
            // Check if this is the best solution so far
            if (k_args->m * k_args->n > max_squares) {
                max_squares = k_args->m * k_args->n;
            }
            pthread_mutex_unlock(&mutex);

            pthread_mutex_lock(&mutex);
            if (checkClosed(m, n, r, c, board)) {
                total_closed_tours++;
                #ifndef QUIET
                    if (thread_number == 0)
                        printf("MAIN: Found a closed knight's tour; incremented total_closed_tours\n");
                    else
                        printf("T%ld: Found a closed knight's tour; incremented total_closed_tours\n", k_args->thread_number);
                #endif
            } else {
                total_open_tours++;
                #ifndef QUIET
                    if (thread_number == 0)
                        printf("MAIN: Found an open knight's tour; incremented total_open_tours\n");
                    else
                        printf("T%ld: Found an open knight's tour; incremented total_open_tours\n", k_args->thread_number);
                #endif
            }
            pthread_mutex_unlock(&mutex);   
            break;  
        } 
        //Get possible moves
        int possible_moves = 0;
        for (int i = 1; i <= 8; i++)
        {
            new_move next = getNewMove(i, r, c);
            int newRow = next.r;
            int newCol = next.c;
            
            if (checkValidMove(m, n, newRow, newCol, board))
            {
                possible_moves++;
            }
        }
            
        if (possible_moves == 0)
        {
            #ifndef QUIET
            if (k_args->thread_number == 0)
                printf("MAIN: Dead end at move #%d\n", k_args->moves);
            else
                printf("T%ld: Dead end at move #%d\n", k_args->thread_number, k_args->moves);
            #endif
            pthread_mutex_lock(&mutex);
            if (k_args->moves > max_squares) {
                max_squares = k_args->moves;
            }
            pthread_mutex_unlock(&mutex);
            return NULL;
        } else if (possible_moves == 1) {
            for (int i = 1; i <= 8; i++)
            {
                new_move next = getNewMove(i, r, c);
                int newRow = next.r;
                int newCol = next.c;
                if (checkValidMove(m, n, newRow, newCol, board))
                {
                    *(*(board + newRow) + newCol) = moves + 1;
                    moves++;
                    k_args->r = newRow;
                    k_args->c = newCol;
                    k_args->moves = moves;
                    break;
                }
            }
            continue;
        } else {
            #ifndef QUIET
                if (k_args->thread_number == 0)
                    printf("MAIN: %d possible moves after move #%d; creating %d child threads...\n", possible_moves, k_args->moves, possible_moves);
                else
                    printf("T%ld: %d possible moves after move #%d; creating %d child threads...\n", k_args->thread_number, possible_moves, k_args->moves, possible_moves);
            #endif

            kdata *child_args = calloc(possible_moves, sizeof(kdata));
            pthread_t *child_threads = calloc(possible_moves, sizeof(pthread_t));
            int valid_moves = 0;
            for(int i = 1; i < 8; i++) {
                new_move next = getNewMove(i, r, c);
                int newRow = next.r;
                int newCol = next.c;
                if (checkValidMove(m, n, newRow, newCol, board))
                {
                    pthread_mutex_lock(&mutex);
                    (child_args + valid_moves)->thread_number = next_thread_number;
                    next_thread_number++;
                    pthread_mutex_unlock(&mutex);
                    
                    // Copy board to new board
                    int **board = calloc(k_args->m, sizeof(int *));
                    for (int j = 0; j < k_args->m; j++) {
                        *(board + j) = calloc(k_args->n, sizeof(int));
                    }

                    // Copy board from args to new board
                    for (int j = 0; j < k_args->m; j++) {
                        for (int k = 0; k < k_args->n; k++) {
                            *(*(board + j) + k) = *(*(k_args->board + j) + k);
                        }
                    }

                    // Place knight at new position
                    *(*(board + newRow) + newCol) = k_args->moves + 1;

                    (child_args+ valid_moves)->m = k_args->m;
                    (child_args+ valid_moves)->n = k_args->n;
                    (child_args+ valid_moves)->r = newRow;
                    (child_args+ valid_moves)->c = newCol;
                    (child_args+ valid_moves)->moves = k_args->moves+ 1;
                    (child_args+ valid_moves)->board = board;
                    
                    pthread_create(child_threads + valid_moves, NULL, knight_tour, child_args + valid_moves);

                #ifdef NO_PARALLEL
                    pthread_join(*(child_threads + i), NULL);
                    #ifndef QUIET
                    if (k_args->thread_number == 0) {
                        printf("MAIN: T%ld joined\n", (child_args+ i)->thread_number);
                    } else {
                        printf("T%ld: T%ld joined\n", k_args->thread_number, (child_args + i)->thread_number);
                    #endif
                    free_board((child_args + valid_moves)->board, k_args->m);
                    }
                #endif

                    valid_moves++;
                }
            }
            #ifndef NO_PARALLEL
            for (int i = 0; i < possible_moves; i++) {
                pthread_join(*(child_threads + i), NULL);
        #ifndef QUIET
            if (k_args->thread_number == 0) {
                printf("MAIN: T%ld joined\n", (child_args + i)->thread_number);
            } else {
                printf("T%ld: T%ld joined\n", k_args->thread_number, (child_args + i)->thread_number);
            }
        #endif
            free_board((child_args + i)->board, k_args->m);
        }
        #endif
        free(child_args);
        free(child_threads);
        break;
        }
    }
    return NULL;
}

int solve(int argc, char **argv) {
    int m, n, r, c;
    setvbuf(stdout, NULL, _IONBF, 0);
    // Arguments check
    if (argc < 5) {
        fprintf(stderr, "ERROR: Invalid argument(s)\nUSAGE: ./hw3.out <m> <n> <r> <c>\n");
        return EXIT_FAILURE;
    } else {
        // Convert arguments to integers
        m = atoi(*(argv + 1));
        n = atoi(*(argv + 2));
        r = atoi(*(argv + 3));
        c = atoi(*(argv + 4));
        // Check if arguments are valid
        if (m < 2 || n < 2 || r < 0 || c < 0)
        {
            fprintf(stderr, "ERROR: Invalid argument(s)\nUSAGE: ./hw3.out <m> <n> <r> <c>\n");
            return EXIT_FAILURE;
        }
    }

    // Prepare arguments for knight_tour thread
    kdata *args = (kdata *)calloc(1, sizeof(kdata));
    args->m = m;
    args->n = n;
    args->r = r;
    args->c = c;
    args->thread_number = 0;
    args->moves = 1;

    // Initialize board
    args->board = (int **)calloc(m, sizeof(int *));
    for (int i = 0; i < m; i++) {
        *(args->board + i) = (int *)calloc(n, sizeof(int));
    }
    // Place knight at starting position
    *(*(args->board + r) + c) = 1;

    printf("MAIN: Solving the knight's tour problem for a %dx%d board\n", m, n);
    printf("MAIN: Starting at row %d and column %d (move #1)\n", r, c);

    pthread_mutex_lock(&mutex);
    next_thread_number++;
    pthread_mutex_unlock(&mutex);

    pthread_t root_thread;
    pthread_create(&root_thread, NULL, knight_tour, args);
    pthread_join(root_thread, NULL);

    

    if (max_squares == m * n) {
        printf("MAIN: Search complete; found %d open tours and %d closed tours\n", total_open_tours, total_closed_tours);
    } else {
    #ifndef QUIET
        printf("MAIN: Search complete; best solution(s) visited %d squares out of %d\n", max_squares, m * n);
    #endif
    }

    free_board(args->board, m);
    free(args);

    return 0;
}

