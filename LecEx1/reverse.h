char * reverse( char * s )
{
    
    char* iter = s;
    size_t size = 0;
    char* end = s;

    while (*iter != '\0') iter++, size++;
    char* buffer = (char*)malloc(size * sizeof(char)+1);
    *(buffer + size* sizeof(char)) = '\0';
    char* iterB = buffer;
    iter--;
    while (iter >= end) {
        *iterB = *iter;
        iter--;
        iterB++;   
    }
    iterB = buffer;
    iter = s;
    // printf("%s \n", iter);
    // printf("%s \n", iterB);
    for(int i = 0; i < size; i++, iter++, iterB++ ) {
        *iter = *iterB; 
        
    }
    free(buffer);
    return s;
}