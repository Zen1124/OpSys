#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "hw1.h"



int hash(char* input, int cache_size) {
    char* iter = input;
    int sum = 0;
    while(*iter != '\0') {
        //*iter = tolower(*iter);
        sum += (int) tolower(*iter);
        
        iter++;
    }

    sum = sum % cache_size;
    return sum;
}


int main(int argc, char **argv) {
    struct stat st;

    if (argc < 3) {
        printf("%d\n", argc);
        perror("ERROR: Atleast 2 arguments required");
        return 0;
    }

    int cache_size = atoi(*(argv+1));
    char** hash_table = calloc(cache_size, sizeof(char*));
    int** size_table = (int**)calloc(cache_size, sizeof(int*));


    for(int i = 0; i < cache_size; i++) {
        *(size_table + i) = (int*)calloc(1, sizeof(int*));
    }

    int file_size_sum = 0;
    int file_open = 0;
    for(int x = 2; x < argc; x++) {
        file_open = open(*(argv + x), O_RDONLY);
        if (file_open == -1) {
            perror("Error: Opening file\n");
            return 0;
        }
        fstat(file_open, &st);
        file_size_sum += (int)st.st_size;
    }

    int buffer_size =  0;
    buffer_size = file_size_sum - 1;
    char * buffer = (char*)calloc(buffer_size + 1, sizeof(char));
    *(buffer + buffer_size) =  '\0';

    int offset_bytes = 0;
    for(int x = 2; x < argc; x++) {
        file_open = open(*(argv + x), O_RDONLY);
        if (file_open == -1) {
            perror("Error: Opening file\n");
            return 0;
        }
        int file_read = read(file_open, buffer + offset_bytes, buffer_size - offset_bytes); 
        if (file_read == -1) {
            printf("Error while reading file");
            return 0;
        }
        offset_bytes += file_read;
        close(file_open);
    }

    char** output = tokenize(buffer);
    if (output != NULL) {
        char **ptr = output;
        while (*ptr != NULL) {
            int index = hash(*(ptr), cache_size);
            int data_size = strlen(*ptr) + 1;
            if(*(hash_table + index) == NULL) {
                //NEW INDEX
                *(hash_table + index) = (char*) calloc(data_size, sizeof(char*));
                
                strcpy(*(hash_table + index), *ptr);

                printf("Word \"%s\" ==> %d (%s)\n", *(hash_table + index), index, "calloc");
                **(size_table + index) = data_size; 

            } else {
            
                if (**(size_table + index) == data_size) {
                    //SAME SIZE COLLISION
                    strcpy(*(hash_table + index), *ptr);
                    
                    //printf("%d ", **(size_table + index));
                    printf("Word \"%s\" ==> %d (%s)\n", *(hash_table + index), index, "nop");

                } else {
                    //DIFFERENT SIZE COLLISION
                    char* temp = realloc(*(hash_table + index), data_size* sizeof(char*));
                    //temp = ;
                    *(hash_table + index) = temp;
                    strcpy(*(hash_table + index), *ptr);

                    printf("Word \"%s\" ==> %d (%s)\n", *(hash_table + index), index, "realloc");
                    **(size_table + index) = data_size;

                }
                
            }
            
            ptr++;
        }
        free(output);
    }
    printf("\nCache:\n");
    for(int i = 0; i < cache_size; i++) {
        if(*(hash_table + i) == NULL) {
            free(*(hash_table + i));
            free(*(size_table + i));
            continue;
        }
        for(int b = 0; b < **(size_table + i);b++) {
            *(*(hash_table + i) + b) = tolower(*(*(hash_table + i) + b));
        }
        printf("%c%d%c ==> \"%s\"\n", 91, i, 93, *(hash_table + i));
        free(*(hash_table + i));
        free(*(size_table + i));
    }

    free(buffer);
    free(hash_table);
    free(size_table);

    return 0;
}