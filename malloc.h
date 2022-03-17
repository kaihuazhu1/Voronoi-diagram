// Function prototypes for custom helper functions
#ifndef MALLOC_H
#define MALLOC_H

#include <stdio.h>

void *mymalloc(size_t size);
void *myrealloc(void *old, size_t size);
FILE *myfopen(const char *fname, const char *mode);

#endif