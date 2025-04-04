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
} knight_args;

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

int checkClosed(int m, int n, int r, int c, int **board)
{
    int newRow, newCol;
    for (int i = 1; i <= 8; i++)
    {
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
        }
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
    knight_args *k_args = (knight_args *)args;
    int newRow, newCol;
    int m = k_args->m, n = k_args->n, r = k_args->r, c = k_args->c, moves = k_args->moves;

    char *thread_name = (char *)calloc(20, sizeof(char));

    if (k_args->thread_number != 0)
    {
        pthread_mutex_lock(&mutex);
        k_args->thread_number = next_thread_number++;
        if (moves > max_squares)
            max_squares = moves;
        pthread_mutex_unlock(&mutex);
        sprintf(thread_name, "T%ld", k_args->thread_number);
    }
    else
    {
        thread_name = "MAIN";
    }

    int **board = calloc(m, sizeof(int *));
    for (int i = 0; i < m; i++)
    {
        *(board + i) = calloc(n, sizeof(int));
    }

    // Copy board from args to new board
    int i = 0;
    while (i < m)
    {
        int j = 0;
        while (j < n)
        {
            *(*(board + i) + j) = *(*(k_args->board + i) + j);
            j++;
        }
        i++;
    }

#ifdef DEBUG
    printf("DEBUG===%s: Move #%d: knight at row %d column %d\n", thread_name, moves, r, c);
    for (int i = 0; i < m; i++)
    {
        for (int j = 0; j < n; j++)
        {
            printf("%d\t", *(*(board + i) + j));
        }
        printf("\n");
    }
#endif

    int possible_moves = 0;

    // Count possible moves
    for (int i = 1; i <= 8; i++)
    {
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
        }
        printf("DEBUG===%s: Move #%d: Trying to place knight at row %d column %d\n", thread_name, moves, newRow, newCol);
        if (checkValidMove(m, n, newRow, newCol, board))
        {
            possible_moves++;
        }
    }

    // Dead End
    if (possible_moves == 0)
    {
        if (k_args->moves == m * n)
        {
            if (checkClosed(m, n, r, c, board))
            {
                pthread_mutex_lock(&mutex);
                total_closed_tours++;
                pthread_mutex_unlock(&mutex);
#ifndef QUIET
                printf("%s: Found a closed knight's tour; incremented total_closed_tours\n", thread_name);
#endif
            }
            else
            {
                pthread_mutex_lock(&mutex);
                total_open_tours++;
                pthread_mutex_unlock(&mutex);
#ifndef QUIET
                printf("%s: Found an open knight's tour; incremented total_open_tours\n", thread_name);
#endif
            }
        }
        else
        {
#ifndef QUIET
            printf("%s: Dead end at move #%d\n", thread_name, moves);
#endif
        }
        free(k_args);
        pthread_exit(NULL);
    }

// Display possible moves message
#ifndef QUIET
    if (possible_moves > 1)
    {
        printf("%s: %d possible moves after move #%d; creating %d child threads...\n", thread_name, possible_moves, k_args->moves, possible_moves);
    }
#endif

    // Create threads for each possible move
    pthread_t *threads;
    knight_args **new_args;
    if (possible_moves > 0)
    {
        // allocate memory for threads and args
        threads = (pthread_t *)calloc(possible_moves, sizeof(pthread_t));
        new_args = (knight_args **)calloc(possible_moves, sizeof(knight_args *));

        int index = 0;
        for (int i = 1; i <= 8; i++)
        {
            // Get new move coordinates
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
            }
            // Check if move is valid
            // If valid, create new args and thread
            if (checkValidMove(m, n, newRow, newCol, board))
            {
                *(new_args + index) = (knight_args *)calloc(1, sizeof(knight_args));
                (*(new_args) + index)->r = newRow;
                (*(new_args) + index)->c = newCol;
                (*(new_args) + index)->c = newCol;
                (*(new_args) + index)->m = m;
                (*(new_args) + index)->n = n;
                (*(new_args) + index)->moves = k_args->moves + 1;

                // Copy board from k_args to new board
                (*(new_args) + index)->board = (int **)calloc(m, sizeof(int *));
                for (int i = 0; i < m; i++)
                {
                    (*(new_args) + index)->board[i] = (int *)calloc(n, sizeof(int));
                }
                i = 0;
                // Copy board from local to new board
                while (i < m)
                {
                    int j = 0;
                    while (j < n)
                    {
                        *(*((*(new_args + index))->board + i) + j) = *(*(board + i) + j);
                        j++;
                    }
                    i++;
                }
                // Place knight at new position
                *(*((*(new_args + index))->board + newRow) + newCol) = (*(new_args + index))->moves;

#ifdef DEBUG
                printf("DEBUG===%s: Move #%d: knight at row %d column %d\n", thread_name, moves, newRow, newCol);
                for (int i = 0; i < m; i++)
                {
                    for (int j = 0; j < n; j++)
                    {
                        printf("%d\t", *(*((*(new_args + index))->board + i) + j));
                    }
                    printf("\n");
                }
#endif

                // Free board
                for (int i = 0; i < m; i++)
                {
                    free(*(board + i));
                }
                free(board);
                // free(thread_name);
                pthread_create(*threads, NULL, knight_tour, *(new_args) + index);
#ifdef NO_PARALLEL
                pthread_join(threads, NULL);
#endif
                index++;
            }
        }

#ifndef NO_PARALLEL
        for (int i = 0; i < possible_moves; i++)
        {
            pthread_join(*threads, NULL);
#ifndef QUIET
            printf("%s: T%ld joined\n", thread_name, *threads);
#endif
        }
#endif

        free(threads);
        free(new_args);
    }
    else if (possible_moves == 1)
    {
        for (int i = 1; i <= 8; i++)
        {
            // Get new move coordinates
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
            }
            if (checkValidMove(m, n, newRow, newCol, board))
            {
                *(*(args->board + newRow) + newCol) = args->moves;
                args->row = newRow;
                args->col = newCol;
                args->moves++;
                knight_tour(args);
                return NULL;
            }
        }
    }

#ifndef NO_PARALLEL
    for (int i = 0; i < child_count; ++i)
    {
        pthread_join(threads[i], NULL);
#ifndef QUIET
        if (thread_args[i]->thread_id >= 1)
            printf("MAIN: T%ld joined\n", thread_args[i]->thread_id);
#endif
        free_board(thread_args[i]->board, thread_args[i]->m);
        free(thread_args[i]);
    }
#endif
    if (possible == 0)
    {
        pthread_mutex_lock(&lock);
        if (args->move_num > max_squares)
            max_squares = args->move_num;

        if (args->move_num == args->m * args->n)
        {
            if (is_closed_tour(args->board, args->m, args->n,
                               args->start_row, args->start_col, args->row, args->col))
            {
#ifndef QUIET
                if (args->thread_id >= 1)
                    printf("T%ld: Found a closed knight's tour; incremented total_closed_tours\n", args->thread_id);
#endif
                total_closed_tours++;
            }
            else
            {
#ifndef QUIET
                if (args->thread_id >= 1)
                    printf("T%ld: Found an open knight's tour; incremented total_open_tours\n", args->thread_id);
#endif
                total_open_tours++;
            }
        }
        else
        {
#ifndef QUIET
            if (args->thread_id >= 1)
                printf("T%ld: Dead end at move #%d\n", args->thread_id, args->move_num);
#endif
        }
        pthread_mutex_unlock(&lock);
    }

    free(k_args);
    pthread_exit(NULL);
}

int solve(int argc, char **argv)
{
    int m, n, r, c;
    setvbuf(stdout, NULL, _IONBF, 0);
    // Arguments check
    if (argc < 5)
    {
        fprintf(stderr, "ERROR: Invalid argument(s)\nUSAGE: ./hw3.out <m> <n> <r> <c>\n");
        return EXIT_FAILURE;
    }
    else
    {
        // Convert arguments to integers
        m = atoi(*(argv + 1));
        n = atoi(*(argv + 2));
        r = atoi(`*(argv + 3));
        c = atoi(*(argv + 4));
        // Check if arguments are valid
        if (m < 2 || n < 2 || r < 0 || c < 0)
        {
            fprintf(stderr, "ERROR: Invalid argument(s)\nUSAGE: ./hw3.out <m> <n> <r> <c>\n");
            return EXIT_FAILURE;
        }
    }

    knight_args *args = (knight_args *)calloc(1, sizeof(knight_args));
    args->m = m;
    args->n = n;
    args->r = r;
    args->c = c;
    args->thread_number = 0;
    args->moves = 1;

    // Initialize board
    args->board = (int **)calloc(m, sizeof(int *));
    int i = 0;
    while (i < m)
    {
        *(args->board + i) = (int *)calloc(n, sizeof(int));
        i++;
    }

    // Place knight at starting position
    *(*(args->board + r) + c) = 1;

#ifdef DEBUG
    for (int i = 0; i < m; i++)
    {
        for (int j = 0; j < n; j++)
        {
            printf("%d\t", *(*(args->board + i) + j));
        }
        printf("\n");
    }
#endif

    printf("MAIN: Solving the knight's tour problem for a %dx%d board\n", m, n);
    printf("MAIN: Starting at row %d and column %d (move #1)\n", r, c);

#ifdef NO_PARALLEL
    // knight_tour(args);
    pthread_create(&root_thread, NULL, knight_tour, args);
    pthread_join(root_thread, NULL);
#else
    pthread_t root_thread;
    pthread_create(&root_thread, NULL, knight_tour, args);
    pthread_join(root_thread, NULL);
#endif

    printf("MAIN: Search complete; found %d open tours and %d closed tours\n", total_open_tours, total_closed_tours);

    // Free board
    for (int i = 0; i < m; i++)
    {
        free(*(args->board + i));
    }
    free(args->board);

    return 0;
}