#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "stage1.h"
#include "malloc.h"

// Calculates and prints the equation of the bisector
void *stage1(char *input_file, char *output_file) {
    FILE *input;
    FILE *output = myfopen(output_file, "w");
    if((input = myfopen(input_file, "r")) == NULL) {
        printf("Can't open file.\n");
        exit(1);
    }
    double x1, y1, x2, y2;
    double *midpoint_x = mymalloc(sizeof(double));
    double *midpoint_y = mymalloc(sizeof(double));
    double *gradient = mymalloc(sizeof(double));
    while(fscanf(input, "%lf%lf%lf%lf", &x1, &y1, &x2, &y2) == 4) {
        *midpoint_x = (x1 + x2)/2;
        *midpoint_y = (y1 + y2)/2;
        if(isfinite(-((x1-x2)/(y1-y2))) == 1) {
            *gradient  = -((x1-x2)/(y1-y2));
            if( *gradient == -0) {
                *gradient = 0;
            }
            fprintf(output, "y = %f * (x - %f) + %f\n", *gradient, *midpoint_x, *midpoint_y);
        }   
        else {
            fprintf(output, "x = %f\n", *midpoint_x);
        }
    }
    free(midpoint_x);
    free(midpoint_y);
    free(gradient);
    fclose(input);
    fclose(output);
    return 0;
}