#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "stage1.h"
#include "stage2.h"
#include "malloc.h"
#include "intersect.h"
#include "dcel.h"

#define MAX_LENGTH (200)

enum intersectType {
    DOESNT_INTERSECT  = 0, // Doesn't intersect
    INTERSECT         = 1, // Intersects
    SAME_LINE_OVERLAP = 2, // Lines are the same
    ENDS_OVERLAP      = 3  // Intersects at exactly one point (endpoint)
};

// Calculates and prints the edge intersections of bisector and polygon
void *stage2(char *input_file, char *polygon, char *output) {
    int *size = mymalloc(sizeof(int));
    vertex_t *vertices = store_vertices(polygon, size);
    int n = *size;
    edge_t *edges = mymalloc(sizeof(edge_t) * n);
    edges->size = n-1;

    // create array of edges
    for(int i=0; i<n; i++) {
        halfedge_t *halfedge = create_halfedge(i, i+1, 0, i);
        halfedge->twin = NULL;
        edges[i].half_edge = halfedge;
    }
    // point head to tail
    edges[n-1].half_edge->index_end = 0;
    edges[0].half_edge->prev = edges[n-1].half_edge;
    // link half-edges
    for(int i=0; i<n-1; i++) {
        edges[i].half_edge->next = edges[i+1].half_edge;
        if(i!=0) {
            edges[i].half_edge->prev = edges[i-1].half_edge;
        }
        else {
            edges[i].half_edge->prev = NULL;
        }
    }
    // point tail to head
    edges[n-1].half_edge->next = edges[0].half_edge;

    FILE *points_file;
    if((points_file = myfopen(input_file, "r")) == NULL) {
        printf("Can't open file.\n");
        exit(1);
    }
    FILE *outfile = myfopen(output, "w");
    double x1, y1, x2, y2;
    double *midpoint_x = mymalloc(sizeof(double));
    double *midpoint_y = mymalloc(sizeof(double));
    double *gradient = mymalloc(sizeof(double));
    vertex_t *bS = mymalloc(sizeof(vertex_t));
    vertex_t *bE = mymalloc(sizeof(vertex_t));
    double *int_x = mymalloc(sizeof(double));
    double *int_y = mymalloc(sizeof(double));

    // for each point pair check and print which half-edge it intersects
    while(fscanf(points_file, "%lf%lf%lf%lf", &x1, &y1, &x2, &y2) == 4) {
        int i=0;
        *midpoint_x = (x1 + x2)/2;
        *midpoint_y = (y1 + y2)/2;
        halfedge_t *start = edges[0].half_edge;
        halfedge_t *loop_half = edges[0].half_edge;

        // checks if line is non-vertical
        if(isfinite(-((x1-x2)/(y1-y2))) == 1) {
            *gradient  = -((x1-x2)/(y1-y2));
            bS->x = *midpoint_x - MAX_LENGTH;
            bS->y = (*gradient) * (bS->x - *midpoint_x) + *midpoint_y;

            bE->x = *midpoint_x + MAX_LENGTH;
            bE->y = (*gradient) * (bE->x - *midpoint_x) + *midpoint_y;

            // checks if each half-edge in the face intersects with the line
            while(loop_half->index_end != start->index_start) {
                if(intersects(vertices[loop_half->index_start], vertices[loop_half->index_end], bS, bE, int_x, int_y) != DOESNT_INTERSECT) {
                    if(i==0) {
                        fprintf(outfile, "From Edge %d (%f, %f) ", loop_half->parent_edge, *int_x, *int_y);
                        i++;
                    }
                    else {
                        fprintf(outfile, "to Edge %d (%f, %f)\n", loop_half->parent_edge, *int_x, *int_y);
                    }
                }
                loop_half = loop_half->next;
            }
            // ensure last half-edge in loop is included 
            if(intersects(vertices[loop_half->index_start], vertices[loop_half->index_end], bS, bE, int_x, int_y) != DOESNT_INTERSECT) {
                if(i==0) {
                    fprintf(outfile, "From Edge %d (%f, %f) ", loop_half->parent_edge, *int_x, *int_y);
                    i++;
                }
                else {
                    fprintf(outfile, "to Edge %d (%f, %f)\n", loop_half->parent_edge, *int_x, *int_y);
                }
            }   
        }

        // case for vertical line
        else {
            bS->x = bE->x = *midpoint_x;
            bS->y = *midpoint_y - MAX_LENGTH;
            bE->y = *midpoint_y + MAX_LENGTH;
            while(loop_half->index_end != start->index_start) {
                if(intersects(vertices[loop_half->index_start], vertices[loop_half->index_end], bS, bE, int_x, int_y) != DOESNT_INTERSECT) {
                    if(i==0) {
                        fprintf(outfile, "From Edge %d (%f, %f) ", loop_half->parent_edge, *int_x, *int_y);
                        i++;
                    }
                    else {
                        fprintf(outfile, "to Edge %d (%f, %f)\n", loop_half->parent_edge, *int_x, *int_y);
                    }
                }
                loop_half = loop_half->next;
            }
            if(intersects(vertices[loop_half->index_start], vertices[loop_half->index_end], bS, bE, int_x, int_y) != DOESNT_INTERSECT) {
                if(i==0) {
                    fprintf(outfile, "From Edge %d (%f, %f) ", loop_half->parent_edge, *int_x, *int_y);
                    i++;
                }
                else {
                    fprintf(outfile, "to Edge %d (%f, %f)\n", loop_half->parent_edge, *int_x, *int_y);
                }
            }
        }   
    }

    // free all half-edges
    for(int i=0;i<=edges->size; i++) {
        if(edges[i].half_edge->twin != NULL) {
            free(edges[i].half_edge->twin);
        }
        if(edges[i].half_edge != NULL) {
            free(edges[i].half_edge);
        }
    }
    free(midpoint_x);
    free(midpoint_y);
    free(gradient);
    free(bS);
    free(bE);
    free(int_x);
    free(int_y);
    free(size);
    free(vertices);
    free(edges);
    fclose(points_file);
    fclose(outfile);
    return 0;
}