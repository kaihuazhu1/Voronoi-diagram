#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "stage1.h"
#include "malloc.h"
#include "intersect.h"
#include "dcel.h"
#include "stage2.h"
#include "read.h"
#include "stage3.h"
#include "stage4.h"

#define MAX_LENGTH (200)  

// Produces information about watchtowers and points based on input 
int main(int argc, char **argv) {

    // STAGE 1
    if(atoi(argv[1]) == 1) {
        stage1(argv[2], argv[3]);
    }
    // STAGE 2
    else if(atoi(argv[1]) == 2) {
        stage2(argv[2], argv[3], argv[4]);
    }
    // STAGE 3
    else if(atoi(argv[1]) == 3) {
        stage3(argv[2], argv[3], argv[4]);
    }
    // STAGE 4
    else {
        stage4(argv[2], argv[3], argv[4]);
    }
    return 0;
}