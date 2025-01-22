#include <stdio.h>
#include <stdlib.h>
#include "reverse.h"

int main () {
    char test1[] = "Hello, World!";
    char test2[] = "  Eat more";
    char test3[] = "pe44eccspm22a";
    char test4[] = " ";
    char test5[] = "";

    printf("test1: %s \n", test1);
    printf("test2: %s \n", test2);
    printf("test3: %s \n", test3);
    printf("test4: %s \n", test4);
    printf("test5: %s \n", test5);

    printf("\n===%s===\n\n", "Reversed");

    printf("test1: %s \n", reverse(test1));
    printf("test2: %s \n", reverse(test2));
    printf("test3: %s \n", reverse(test3));
    printf("test4: %s \n", reverse(test4));
    printf("test5: %s \n", reverse(test5));

    return 0;
}