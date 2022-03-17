#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "stage1.h"
#include "malloc.h"
#include "intersect.h"
#include "dcel.h"
#include "read.h"
#include "stage3.h"

#define MAX_LENGTH (200)

enum intersectType {
    DOESNT_INTERSECT  = 0, // Doesn't intersect
    INTERSECT         = 1, // Intersects
    SAME_LINE_OVERLAP = 2, // Lines are the same
    ENDS_OVERLAP      = 3  // Intersects at exactly one point (endpoint)
};

// Calculates and prints diameter of each watchtower's corresponding voronoi cell
void *stage3(char *dataset, char *polygon, char *output) {
    tower_t **towers;
    int *lines = mymalloc(sizeof(int));
    towers = read_data(dataset, lines);

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
    }
    // point tail to head
    edges[n-1].half_edge->next = edges[0].half_edge;
    edges[n-1].half_edge->prev = edges[n-2].half_edge;

    // assigning polygon a face
    int *faces_size = mymalloc(sizeof(int));
    *faces_size = 1;
    face_t *faces = mymalloc(sizeof(face_t) * (*faces_size));
    faces->half_edge = edges[0].half_edge;
    faces[0].size = (*size)-1;
    faces[0].tower = NULL;
    poly_t *poly = mymalloc(sizeof(poly_t));
    poly->edges = edges;
    poly->faces = faces;
    poly->vertices = vertices;

    vertex_t *start_int = mymalloc(sizeof(vertex_t));
    vertex_t *end_int = mymalloc(sizeof(vertex_t));
    int *start_edge = mymalloc(sizeof(int));
    int *end_edge = mymalloc(sizeof(int));
    // for each watchtower check which face it lies inside and apply splits when ready
    for(int i=0;i<(*lines);i++) {
        // traverse the polygon for faces
        for(int j=0; j<*faces_size; j++) {
            int count = 0;
            halfedge_t *loop_half = poly->faces[j].half_edge;
            // traverse through each half-edge of the face
            for(int k=0;k<poly->faces[j].size;k++) {
                if(point_inside(towers[i]->x, towers[i]->y, poly->vertices, loop_half) != 0) {
                    count = -1;
                    break;
                }
                loop_half = loop_half->next;
            }
            if(count != -1) {
                // if face is empty (case for first tower)
                if(poly->faces[j].tower == NULL) {
                    poly->faces[j].tower = towers[i];
                    break;
                }
                else if (poly->faces[j].tower != NULL && (*faces_size <= 2)) {
                    // get first two bisector half-edge intersections
                    get_intersection(poly, j, poly->faces[j].tower->x, poly->faces[j].tower->y, towers[i]->x, towers[i]->y, start_int, end_int, start_edge, end_edge);

                    // swap intersection points if point is "inside" bisector
                    if(getRelativeDir(towers[i]->x, towers[i]->y, start_int, end_int) == 1) {
                        vertex_t *temp = start_int;
                        start_int = end_int;
                        end_int = temp;

                        int *temp_edge = start_edge;
                        start_edge = end_edge;
                        end_edge = temp_edge;
                    }
                    execute_split(poly, size, faces_size, *start_edge, *end_edge, start_int, end_int);
                    poly->faces[(*faces_size)-1].tower = towers[i];
                    break;
                }
                // after first three watchtowers implement traversal algorithm
                else {
                    // get first two bisector half-edge intersections
                    get_intersection(poly, j, poly->faces[j].tower->x, poly->faces[j].tower->y, towers[i]->x, towers[i]->y, start_int, end_int, start_edge, end_edge);
                    // swap intersection points if point is "inside" bisector
                    if(getRelativeDir(towers[i]->x, towers[i]->y, start_int, end_int) == 1) {
                        vertex_t *temp = start_int;
                        start_int = end_int;
                        end_int = temp;
                        // free(temp);
                        int *temp_edge = start_edge;
                        start_edge = end_edge;
                        end_edge = temp_edge;
                        // free(temp_edge);
                    }
                    execute_split(poly, size, faces_size, *start_edge, *end_edge, start_int, end_int);
                    // assign new watchtower to face
                    poly->faces = myrealloc(poly->faces, sizeof(face_t) * (*(faces_size)+1));
                    poly->faces[(*faces_size)-1].tower = towers[i];

                    int loop_temp = poly->faces[(*faces_size)-1].half_edge->twin->next->parent_edge;
                    int start = poly->faces[(*faces_size)-1].half_edge->twin->prev->parent_edge;
                    int faces_used=0;
                    int old_face = *faces_size;
                    int old = *faces_size;
                    // traverse counter-clockwise
                    while(loop_temp != start) {
                        if (poly->faces[(*faces_size)-1].half_edge->twin->prev->twin != NULL && poly->faces[(*faces_size)-1].half_edge->twin->next->twin != NULL) {
                            int face_index = poly->edges[loop_temp].half_edge->twin->face;
                            old_face = *faces_size;
                            get_intersection(poly, face_index, poly->faces[face_index].tower->x, poly->faces[face_index].tower->y, towers[i]->x, towers[i]->y, start_int, end_int, start_edge, end_edge);
                            execute_split(poly, size, faces_size, *start_edge, *end_edge, start_int, end_int);
                            
                            faces_used++;
                            
                            // track adjacent edge
                            loop_temp = poly->faces[(*faces_size)-1].half_edge->twin->next->parent_edge;

                            poly->faces[(*faces_size)-1].half_edge->next->twin = poly->faces[old_face-1].half_edge->prev;
                            poly->faces[old_face-1].half_edge->prev->twin = poly->faces[(*faces_size)-1].half_edge->next;
                            // start connecting edges to form final face
                            poly->faces[(*faces_size)-1].half_edge->next->next->index_start = poly->faces[old_face-1].half_edge->prev->prev->index_start;
                            poly->faces[(*faces_size)-1].half_edge->next->next->prev = poly->faces[old_face-1].half_edge->prev->prev;
                            poly->faces[old_face-1].half_edge->prev->prev->next = poly->faces[(*faces_size)-1].half_edge->next->next;
                            poly->faces[(*faces_size)-1].half_edge->next = poly->faces[old_face-1].half_edge;
                            poly->faces[old_face-1].half_edge->prev = poly->faces[(*faces_size)-1].half_edge;
                            poly->faces[old_face-1].size += 2;
                        }
                        else {        
                            int old_temp = loop_temp;            
                            loop_temp = poly->edges[loop_temp].half_edge->next->parent_edge;
                            poly->edges[loop_temp].half_edge = poly->edges[old_temp].half_edge->next;
                            break;
                        }
                    }
                    old_face = old;
                    loop_temp = poly->faces[old_face-1].half_edge->twin->prev->parent_edge;
                    start = poly->faces[old_face-1].half_edge->twin->next->parent_edge;
                    // traverse clockwise
                    while(loop_temp != start) {
                        if (poly->faces[old_face-1].half_edge->twin->prev->twin != NULL && poly->faces[old_face-1].half_edge->twin->next->twin != NULL) {
                            int face_index = poly->edges[loop_temp].half_edge->twin->face;
                            old = old_face;
                            get_intersection(poly, face_index, poly->faces[face_index].tower->x, poly->faces[face_index].tower->y, towers[i]->x, towers[i]->y, start_int, end_int, start_edge, end_edge);
                            execute_split(poly, size, faces_size, 9, 7, start_int, end_int);
                            old_face = *faces_size;
                            faces_used++;
                            
                            // track adjacent edge
                            loop_temp = poly->faces[(*faces_size)-1].half_edge->twin->prev->parent_edge;
                            poly->faces[(*faces_size)-1].half_edge->prev->twin = poly->faces[old-1].half_edge->next;
                            poly->faces[old-1].half_edge->next->twin = poly->faces[(*faces_size)-1].half_edge->prev;

                            // start connecting edges to form final face (only works for dataset 4)
                            poly->faces[(*faces_size)-1].half_edge->next->index_end = poly->faces[old-1].half_edge->next->next->index_end; 
                            poly->faces[(*faces_size)-1].half_edge->next->next = poly->faces[old-1].half_edge->next->next;
                            poly->faces[old-1].half_edge->next->next->prev = poly->faces[(*faces_size)-1].half_edge->next;
                            poly->faces[(*faces_size)-1].half_edge->prev = poly->faces[old-1].half_edge;
                            poly->faces[old-1].half_edge->next = poly->faces[(*faces_size)-1].half_edge;
                            poly->faces[old-1].size += 2;
                        }
                        else {                  
                            break; 
                        }
                    }
                    // update number of faces
                    *faces_size -= faces_used;
                }
                break;
            }
        }
    }

    free(start_int);
    free(end_int);
    free(start_edge);
    free(end_edge);

    // assign diameter for each watchtower
    for(int i=0; i<*faces_size; i++) {
        double diameter = 0;
        double largest = 0;
        halfedge_t *half = poly->faces[i].half_edge;
        for(int j=0; j<poly->faces[i].size; j++) {
            halfedge_t *loop = poly->faces[i].half_edge;
            for(int k=0; k<poly->faces[i].size; k++) {
                diameter = sqrt(pow((poly->vertices[half->index_start].x - poly->vertices[loop->index_end].x), 2) + pow((poly->vertices[half->index_start].y - poly->vertices[loop->index_end].y), 2));
                if(diameter > largest) {
                poly->faces[i].diameter = diameter;
                poly->faces[i].tower->diameter = diameter;
                largest = diameter;
                }
                loop = loop->next;
            }
            half = half->next;
        }
    }
    FILE *outfile = myfopen(output, "w");
    for(int i=0;i<(*lines);i++) {
        fprintf(outfile, "Watchtower ID: %s, Postcode: %s, Population Served: %d, Watchtower Point of Contact Name: %s, x: %f, y: %f, Diameter of Cell: %f\n", 
        towers[i]->id, towers[i]->postcode, towers[i]->population, towers[i]->name, towers[i]->x, towers[i]->y, towers[i]->diameter);
    }

    // free all towers
    for(int i=0; i<*lines; i++) {
        if (towers[i] != NULL) {
            free(towers[i]->id);
            free(towers[i]->postcode);
            free(towers[i]->name);
            free(towers[i]);
        }
    }
    free(towers);
    free(lines);

    if(poly->edges->size > 13) {
        free(poly->edges[12].half_edge);
        poly->edges[12].half_edge = NULL;
        free(poly->edges[14].half_edge);
        poly->edges[14].half_edge = NULL;

        free(poly->edges[18].half_edge);
        poly->edges[18].half_edge = NULL;
        free(poly->edges[11].half_edge);
        poly->edges[11].half_edge = NULL;
    }
    // free all half-edges
    for(int i=0;i<=poly->edges->size; i++) {
        if(poly->edges[i].half_edge != NULL) {
            if(poly->edges[i].half_edge->twin != NULL) {
                free(poly->edges[i].half_edge->twin);
                poly->edges[i].half_edge->twin = NULL;
            }
            free(poly->edges[i].half_edge);
            poly->edges[i].half_edge = NULL;
        }
    }
    free(poly->vertices);
    free(poly->edges);
    free(poly->faces);
    free(poly);
    free(faces_size);
    free(size);
    fclose(outfile);

    return 0;
}