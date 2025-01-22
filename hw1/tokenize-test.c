#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "hw1.h"

int main() {
    char test1[] = " RPI) is a private research university in Troy, New York, United Sta";

    char** output = tokenize(test1);
    while(*output != NULL) {
        printf("%s\n", *output);
        output++;
    }
}