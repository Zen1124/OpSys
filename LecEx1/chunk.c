#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

int main(int argc, char ** argv) {
    struct stat st;

    if (argc != 3) {
        printf("Usage: <byte count> <filename>\n");
        return 0;
    }

    int chunk_size = atoi(*(argv + 1));
    off_t offset = chunk_size;

    int file_open = open(*(argv + 2), O_RDONLY);
    if (file_open == -1) {
        printf("Error while opening file\n");
        return 0;
    }

    fstat(file_open, &st);

    //printf("File size: %ld \n", st.st_size);
    int buffer_size =  0;
    buffer_size = (int) st.st_size - 1;
    char * buffer = (char*)malloc(chunk_size * sizeof(char) + 1);
    *(buffer + chunk_size) =  '\0';
    if (buffer_size + 1 < chunk_size || chunk_size <= 0) {
        printf("\n");
        close(file_open);
        free(buffer);
        return 0;
    }
    int file_read = read(file_open, buffer, chunk_size); 
    if (file_read == -1) {
        printf("Error while reading file");
        return 0;
    }
    
    int limit = buffer_size - chunk_size + 1;
    //printf("file limit: %d\n", limit);
    printf("%s", buffer);
    int file_seek = lseek(file_open, offset, SEEK_CUR);
    if (file_seek == -1) {
        close(file_open);
        free(buffer);
        printf("File size too small");
        return 0;
    }
    

    while (file_read != 0 && (file_seek <= limit)) {
        file_read = read(file_open, buffer, chunk_size);
        
        file_seek = lseek(file_open, offset, SEEK_CUR);
        //printf("|%d", file_seek);
        printf("|%s", buffer);
        
    }
    printf("\n");
    close(file_open);
    free(buffer);
    return 0;
}