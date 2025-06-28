#include "guards.h"
#include <stdio.h>
#include <stdlib.h>


void guards_oom(const void *ptr, char *file, int number)
{
    if (ptr == NULL)
    {
        fprintf(stderr, "Out of memory file %s at line %d\n", file, number);
        exit(EXIT_FAILURE);
    }
}
