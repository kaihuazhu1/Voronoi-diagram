// Function prototypes for intersection functions
#ifndef INTERSECT_H
#define INTERSECT_H

#include "dcel.h"

enum intersectType;

/* Returns -1, 0 or 1, based on the area enclosed by the three points. 0 corresponds
    to no area enclosed.
*/
int areaSign(double sx, double sy, double ex, double ey, double x, double y);

/* Returns 1 if the point (x, y) is in the line from s(x, y) to e(x, y), 0 otherwise. */
int collinear(double sx, double sy, double ex, double ey, double x, double y);

/* Returns 1 if point (x, y) is between (sx, sy) and (se, se) */
int between(double sx, double sy, double ex, double ey, double x, double y);

enum intersectType parallelIntersects(double heSx, double heSy, double heEx, double heEy,
    double bSx, double bSy, double bEx, double bEy, double *x, double *y);

enum intersectType intersects(vertex_t heS, vertex_t heE, vertex_t *bS, vertex_t *b, double *x, double *y);

int getRelativeDir(double x, double y, vertex_t *v1, vertex_t *v2);
#endif