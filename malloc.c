#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "malloc.h"

// Concatenates default malloc and assert functions
void *mymalloc(size_t size) {
    void *p = malloc(size);
    assert(p);
    return p;
}

// Concatenates default realloc and assert functions
void *myrealloc(void *old, size_t size) {
    void *p = realloc(old, size);
    assert(p);
    return p;
}

// Concatenates default fopen and assert functions
FILE *myfopen(const char *fname, const char *mode) {
    FILE *fp = fopen(fname, mode); 
    assert(fp);
    return fp;
}