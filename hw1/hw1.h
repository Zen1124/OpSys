#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

char ** tokenize( char * string ) {
    char* iter = string;
    
    int buffer_count = 0;
    int token_count = 1;

    char** token = (char**)calloc((8 * token_count + 1), sizeof(char *)); //Must be 8 bytes to cover size of pointer
    buffer_count = 0;
    int i = 0;
    while ( *iter != '\0') { 
        if ((!isalpha(*iter)) && buffer_count >= 2) {
            if (token_count < i ) {
                char** new_token = (char**)realloc(token, (8 * token_count * 2) * sizeof(char *));
                token_count *= 2;
                token = new_token;
            }
            *iter = '\0';
            *(token + i) = (iter - buffer_count);
            buffer_count = 0;
            i++;
        } else if (!isalpha(*iter)) {
            buffer_count = 0;
        } else {
            buffer_count++;
        }
        iter++;
    }

    //Last token
    if (buffer_count >= 2) {
        if (token_count < i ) {
            char** new_token = (char**)realloc(token, (8 * token_count * 2) * sizeof(char *));
            token_count *= 2;
            token = new_token;
        }
        *(token + i) = (iter - buffer_count);
        i++;
    }

    *(token+ i) = NULL;
 
    return token;          
}