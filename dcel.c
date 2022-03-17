#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "dcel.h"
#include "malloc.h"
#include "intersect.h"

#define MAX_BUFFER_SIZE (512+1)
#define MAX_FIELD_SIZE (128+1)
#define MAX_LENGTH (200)

enum intersectType {
    DOESNT_INTERSECT  = 0, // Doesn't intersect
    INTERSECT         = 1, // Intersects
    SAME_LINE_OVERLAP = 2, // Lines are the same
    ENDS_OVERLAP      = 3  // Intersects at exactly one point (endpoint)
};

// Stores vertices in an array from given polygon file
vertex_t *store_vertices(char* polygon, int *size){
    FILE *polyfile;
    if ((polyfile = fopen(polygon, "r")) == NULL){
        printf("Can't open file.\n");
        exit(1);
    }
    *size = 10;
    vertex_t *vertices = mymalloc(sizeof(vertex_t) * (*size));

    int i=0;
    while(fscanf(polyfile, "%lf%lf", &vertices[i].x, &vertices[i].y) == 2) {
        if(i == *size) {
            *size *= 2;
            vertices = myrealloc(vertices, sizeof(vertex_t*) * (*size));
        }
        i++;
    }
    fclose(polyfile);
    *size = i;
    return vertices;
}

// Returns 0 if point is "inside" half-edge and returns 1 otherwise
int point_inside(double x, double y, vertex_t *vertices, halfedge_t *halfedge) {
    double start_x = vertices[halfedge->index_start].x;
    double end_x = vertices[halfedge->index_end].x;
    double start_y = vertices[halfedge->index_start].y;
    double end_y = vertices[halfedge->index_end].y;
    // case for vertical line across the y-axis
    if (start_x == end_x && start_y < end_y) {
        if (x>start_x) {
            return 0;
        }
    }
    else if (start_x == end_x && start_y > end_y) {
        if (x<start_x) {
            return 0;
        }
    }
    // case for horizontal line across x-axis
    else if (start_y == end_y && start_x > end_x) {
        if (y>start_y) {
            return 0;
        }
    }
    else if (start_y == end_y && start_x < end_x) {
        if (y<start_y) {
            return 0;
        }
    }
    // case for half-edge with non-zero gradient
    double gradient = (end_y - start_y)/(end_x - start_x);
    double y_int = end_y - gradient*end_x;
    double predicted = gradient*x + y_int;
    if (start_x<end_x && y-predicted<=0) {
        return 0;
    }
    else if (start_x>end_x && y-predicted>=0) {
        return 0;
    }
    return 1;
}

// Calculates midpoint of an edge with given start index
void *midpoint(double *x, double*y, int index, edge_t *edges, vertex_t *vertices) {
    *x = (vertices[edges[index].half_edge->index_start].x
     + vertices[edges[index].half_edge->index_end].x)/2;
    *y = (vertices[edges[index].half_edge->index_start].y
     + vertices[edges[index].half_edge->index_end].y)/2;
    return 0;
}

// Creates a half-edge and assigns co-ordinates, face number and index of parent edge
halfedge_t *create_halfedge(int index_start, int index_end, int face_num, int parent_edge) {
    halfedge_t *halfedge = mymalloc(sizeof(halfedge_t));
    halfedge->index_start = index_start;
    halfedge->index_end = index_end;
    halfedge->face = face_num;
    halfedge->parent_edge = parent_edge;
    halfedge->twin = NULL;
    return halfedge;
}

// Executes a given split with start and end edge on the given polygon 
void *execute_split(poly_t *poly, int *size, int *faces_size, int start_edge, int end_edge, vertex_t *start_int, vertex_t *end_int) {
    *size = (*size)+2;
    // allocate enough memory for two more vertices
    poly->vertices = myrealloc(poly->vertices, sizeof(vertex_t) * (*size));
    double *new_start_x, *new_start_y, *new_end_x, *new_end_y;
    new_start_x = mymalloc(sizeof(double));
    new_start_y = mymalloc(sizeof(double));
    new_end_x = mymalloc(sizeof(double));
    new_end_y = mymalloc(sizeof(double));

    *new_start_x = start_int->x;
    *new_start_y = start_int->y;
    *new_end_x = end_int->x;
    *new_end_y = end_int->y;

    // next two indices available in the vertices array   
    int new_start = (*size)-2;
    int new_end = (*size)-1;
    // inserts new vertices into array
    poly->vertices[new_start].x = *new_start_x;
    poly->vertices[new_start].y = *new_start_y;
    poly->vertices[new_end].x = *new_end_x;
    poly->vertices[new_end].y = *new_end_y;

    // check if edge is pointing to correct half-edge
    vertex_t *g = mymalloc(sizeof(vertex_t));
    g->x = (poly->vertices[new_start].x + poly->vertices[new_end].x)/2;
    g->y = (poly->vertices[new_start].y + poly->vertices[new_end].y)/2;
    // check if point g is within both half-edges
    if (point_inside(g->x, g->y, poly->vertices, poly->edges[start_edge].half_edge) == 1) {
        poly->edges[start_edge].half_edge = poly->edges[start_edge].half_edge->twin;
    }
    if (point_inside(g->x, g->y, poly->vertices, poly->edges[end_edge].half_edge) == 1) {
        poly->edges[end_edge].half_edge = poly->edges[end_edge].half_edge->twin;
    }
    free(g);

    //stores old start and end edges before fixing 
    int old_edge_end = poly->edges[start_edge].half_edge->index_end;
    int old_edge_start = poly->edges[end_edge].half_edge->index_start;
    halfedge_t *temp_1 = poly->edges[start_edge].half_edge->next;
    halfedge_t *temp_2 = poly->edges[end_edge].half_edge->prev;

    // fixes half-edges of start and end edge
    poly->edges[start_edge].half_edge->index_end = new_start;
    poly->edges[end_edge].half_edge->index_start = new_end;
    
    // create new half-edge with twin and assign mid-points as indices
    halfedge_t *halfedge = create_halfedge(new_start, new_end, poly->edges[start_edge].half_edge->face, poly->edges->size + 1);
    halfedge_t *halfedge_twin = create_halfedge(new_end, new_start, 0, poly->edges->size + 1);

    halfedge->twin = halfedge_twin;
    halfedge_twin->twin = halfedge;

    (*faces_size)++;
    poly->faces = myrealloc(poly->faces, sizeof(face_t) * (*faces_size));
    // update face to new joining half-edge
    poly->faces[(*faces_size)-1].half_edge = halfedge_twin;
    
    
    // insert new half-edge into dcel between fixed edges
    poly->edges->size++;
    poly->edges = myrealloc(poly->edges, sizeof(edge_t) * (poly->edges->size+1));
    poly->edges[poly->edges->size].half_edge = halfedge;
    poly->edges[start_edge].half_edge->next = halfedge;
    halfedge->prev = poly->edges[start_edge].half_edge;
    poly->edges[end_edge].half_edge->prev = halfedge;
    halfedge->next = poly->edges[end_edge].half_edge;
    
    // create new dcel for other face
    poly->edges->size += 2;
    poly->edges = myrealloc(poly->edges, sizeof(edge_t) * (poly->edges->size+1));
    halfedge_t *new_half_1 = create_halfedge(halfedge_twin->index_end, old_edge_end, 0, poly->edges->size - 1); 
    new_half_1->twin = NULL;
    halfedge_t *new_half_2 = create_halfedge(old_edge_start, halfedge_twin->index_start, 0, poly->edges->size); 
    new_half_2->twin = NULL;

    // insert dcel (2 other half-edges) into edges array
    poly->edges[poly->edges->size - 1].half_edge = new_half_1;
    poly->edges[poly->edges->size].half_edge = new_half_2;
    
    
    // create new half-edges for new face
    new_half_1->next = temp_1;
    temp_1->prev = new_half_1;
    new_half_2->prev = temp_2;
    temp_2->next = new_half_2;
    
    new_half_1->prev = halfedge_twin; 
    new_half_2->next = halfedge_twin; 
    halfedge_twin->next = new_half_1; 
    halfedge_twin->prev = new_half_2;
    
    // create new face
    poly->faces[(*faces_size)-1].half_edge = halfedge_twin;
    halfedge_twin->face = (*faces_size)-1;
    new_half_1->face = (*faces_size)-1;
    new_half_2->face = (*faces_size)-1;
    
    int i=0;
    halfedge_t *loop_half = halfedge_twin;
    while (loop_half->index_end != halfedge_twin->index_start) {
        loop_half->face = (*faces_size)-1;
        i++;
        loop_half = loop_half->next;
    }
    loop_half->face = (*faces_size)-1;
    poly->faces[*(faces_size)-1].size = i+2;

    poly->faces[halfedge->face].half_edge = halfedge;
    poly->faces[halfedge_twin->face].half_edge = halfedge_twin;
    
    free(new_start_x);
    free(new_start_y);
    free(new_end_x);
    free(new_end_y);
    return 0;
}

void *get_intersection(poly_t *poly, int face_index, double x1, double y1, double x2, double y2, vertex_t *start_int, vertex_t *end_int, int *start_edge, int *end_edge) {
    int i=0;
    double *int_x = mymalloc(sizeof(double));
    double *int_y = mymalloc(sizeof(double));
    double midpoint_x = (x1 + x2)/2;
    double midpoint_y = (y1 + y2)/2;
    
    halfedge_t *start = poly->faces[face_index].half_edge;
    halfedge_t *loop_half = poly->faces[face_index].half_edge;
    vertex_t *bS = mymalloc(sizeof(vertex_t));
    vertex_t *bE = mymalloc(sizeof(vertex_t));

    // checks if line is non-vertical
    if(isfinite(-((x1-x2)/(y1-y2))) == 1) {
        double gradient  = -((x1-x2)/(y1-y2));
        bS->x = midpoint_x - MAX_LENGTH;
        bS->y = gradient * (bS->x - midpoint_x) + midpoint_y;

        bE->x = midpoint_x + MAX_LENGTH;
        bE->y = gradient * (bE->x - midpoint_x) + midpoint_y;
        // checks if each half-edge in the face intersects with the line
        while(loop_half->index_end != start->index_start) {
            printf("Loop edge: %d\n", loop_half->parent_edge);
            if(intersects(poly->vertices[loop_half->index_start], poly->vertices[loop_half->index_end], bS, bE, int_x, int_y) != DOESNT_INTERSECT) {
                if(i==0) {
                    start_int->x = *int_x;
                    start_int->y = *int_y;
                    *start_edge = loop_half->parent_edge;
                    i++;
                }
                else {
                    end_int->x = *int_x;
                    end_int->y = *int_y;
                    *end_edge = loop_half->parent_edge;
                }
            }
            loop_half = loop_half->next;
        }
        printf("Ending loop edge: %d\n", loop_half->parent_edge);
        // ensure last half-edge in loop is included 
        if(intersects(poly->vertices[loop_half->index_start], poly->vertices[loop_half->index_end], bS, bE, int_x, int_y) != DOESNT_INTERSECT) {
            if(i==0) {
                start_int->x = *int_x;
                start_int->y = *int_y;
                *start_edge = loop_half->parent_edge;
                i++;
            }
            else {
                end_int->x = *int_x;
                end_int->y = *int_y;
                *end_edge = loop_half->parent_edge;
            }
        }   
    }

    // case for vertical line
    else {
        bS->x = bE->x = midpoint_x;
        bS->y = midpoint_y - MAX_LENGTH;
        bE->y = midpoint_y + MAX_LENGTH;
        while(loop_half->index_end != start->index_start) {
            if(intersects(poly->vertices[loop_half->index_start], poly->vertices[loop_half->index_end], bS, bE, int_x, int_y) != DOESNT_INTERSECT) {
                if(i==0) {
                    start_int->x = *int_x;
                    start_int->y = *int_y;
                    *start_edge = loop_half->parent_edge;
                    i++;
                }
                else {
                    end_int->x = *int_x;
                    end_int->y = *int_y;
                    *end_edge = loop_half->parent_edge;
                }
            }
            loop_half = loop_half->next;
        }
        if(intersects(poly->vertices[loop_half->index_start], poly->vertices[loop_half->index_end], bS, bE, int_x, int_y) != DOESNT_INTERSECT) {
            if(i==0) {
                start_int->x = *int_x;
                start_int->y = *int_y;
                *start_edge = loop_half->parent_edge;
                i++;
            }
            else {
                end_int->x = *int_x;
                end_int->y = *int_y;
                *end_edge = loop_half->parent_edge;
            }
        }
    }
    free(bS);
    free(bE);
    free(int_x);
    free(int_y);
    return 0;
}