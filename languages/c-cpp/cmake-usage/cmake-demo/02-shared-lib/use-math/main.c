#include <stdio.h>
#include "math.h"

int main()
{
    int x = add(1, 2);
    int y = sub(4, 3);
    printf("%d, %d\n", x, y);
    return 0;
}

// gcc -I./include -L./lib -ljmath -o main main.c