// Definitions for intersection functions
#include "intersect.h"
#include "dcel.h"

#define DIR_UNDECIDED (0)
#define INSIDE (1)
#define OUTSIDE (-1)
#define NODIAMETER (-1)

enum intersectType {
    DOESNT_INTERSECT  = 0, // Doesn't intersect
    INTERSECT         = 1, // Intersects
    SAME_LINE_OVERLAP = 2, // Lines are the same
    ENDS_OVERLAP      = 3  // Intersects at exactly one point (endpoint)
};

/* 
This intersection is based on code by Joseph O'Rourke and is provided for use in 
COMP20003 Assignment 2.

The approach for intersections is:
- Use the bisector to construct a finite segment and test it against the half-edge.
- Use O'Rourke's segseg intersection (https://hydra.smith.edu/~jorourke/books/ftp.html)
    to check if the values overlap.
*/
/* 
    Generates a segment with each end at least minLength away in each direction 
    from the bisector midpoint. Returns 1 if b intersects the given half-edge
    on this segment, 0 otherwise. Sets the intersection point to the given x, y
    positions.
*/
    
int collinear(double sx, double sy, double ex, double ey, double x, double y){
    /* Work out area of parallelogram - if it's 0, points are in the same line. */
    if (areaSign(sx, sy, ex, ey, x, y) == 0){
        return 1;
    } else {
        return 0;
    }
}

int areaSign(double sx, double sy, double ex, double ey, double x, double y){
    double areaSq;
    /* |AB x AC|^2, squared area */
    /* See https://mathworld.wolfram.com/CrossProduct.html */
    areaSq = (ex - sx) * (y  - sy) -
             (x  - sx) * (ey - sy);
    
    if(areaSq > 0.0){
        return 1;
    } else if(areaSq == 0.0){
        return 0;
    } else {
        return -1;
    }
}


int between(double sx, double sy, double ex, double ey, double x, double y){
    if(sx != ex){
        /* If not vertical, check whether between x. */
        if((sx <= x && x <= ex) || (sx >= x && x >= ex)){
            return 1;
        } else {
            return 0;
        }
    } else {
        /* Vertical, so can't check _between_ x-values. Check y-axis. */
        if((sy <= y && y <= ey) || (sy >= y && y >= ey)){
            return 1;
        } else {
            return 0;
        }
    }
}

enum intersectType parallelIntersects(double heSx, double heSy, double heEx, double heEy,
    double bSx, double bSy, double bEx, double bEy, double *x, double *y){
    if(!collinear(heSx, heSy, heEx, heEy, bSx, bSy)){
        /* Parallel, no intersection so don't set (x, y) */
        return DOESNT_INTERSECT;
    }
    /* bS between heS and heE */
    if(between(heSx, heSy, heEx, heEy, bSx, bSy)){
        *x = bSx; 
        *y = bSy;
        return SAME_LINE_OVERLAP;
    }
    /* bE between heS and heE */
    if(between(heSx, heSy, heEx, heEy, bEx, bEy)){
        *x = bEx;
        *y = bEy;
        return SAME_LINE_OVERLAP;
    }
    /* heS between bS and bE */
    if(between(bSx, bSy, bEx, bEy, heSx, heSy)){
        *x = heSx;
        *y = heSy;
        return SAME_LINE_OVERLAP;
    }
    /* heE between bS and bE */
    if(between(bSx, bSy, bEx, bEy, heEx, heEy)){
        *x = heEx; 
        *y = heEy;
        return SAME_LINE_OVERLAP;
    }
    
    return DOESNT_INTERSECT;
}

enum intersectType intersects(vertex_t heS, vertex_t heE, vertex_t *bS, vertex_t *bE, double *x, double *y){
    /* Half-edge x, y pair */
    double heSx = heS.x;
    double heSy = heS.y;
    double heEx = heE.x;
    double heEy = heE.y;
    
    /* Bisector x, y pair */
    double bSx  = bS->x;
    double bSy  = bS->y;
    double bEx  = bE->x;
    double bEy  = bE->y;
    
    /* Parametric equation parameters */
    double t1;
    double t2;
    /* Numerators for X and Y coordinate of intersection. */
    double numeratorX;
    double numeratorY;
    /* Denominators of intersection coordinates. */
    double denominator;
    
    /*
    See http://www.cs.jhu.edu/~misha/Spring20/15.pdf
    for explanation and intuition of the algorithm here.
    x_1 = heSx, y_1 = heSy    |    p_1 = heS
    x_2 = heEx, y_2 = heEy    |    q_1 = heE
    x_3 = bSx , y_3 = bSy     |    p_2 =  bS
    x_4 = bEx , y_4 = bEy     |    q_2 =  bE
    ----------------------------------------
    So the parameters t1 and t2 are given by:
    | t1 |   | heEx - heSx  bSx - bEx | -1  | bSx - heSx |
    |    | = |                        |     |            |
    | t2 |   | heEy - heSy  bSy - bEy |     | bSy - heSy |
    
    Hence:
    | t1 |       1     | bSy - bEy        bEx - bSx |  | bSx - heSx |
    |    | = --------- |                            |  |            |
    | t2 |    ad - bc  | heSy - heEy    heEx - heSx |  | bSy - heSy |
    
        where 
        a = heEx - heSx
        b = bSx  -  bEx
        c = heEy - heSy
        d = bSy  -  bEy
    */
    
    /* Here we calculate ad - bc */
    denominator = heSx * (bEy  -  bSy) +
                  heEx * (bSy  -  bEy) +
                  bEx  * (heEy - heSy) +
                  bSx  * (heSy - heEy);
    
    if(denominator == 0){
        /* In this case the two are parallel */
        return parallelIntersects(heSx, heSy, heEx, heEy, bSx, bSy, bEx, bEy, x, y);
    }
    
    /*
    Here we calculate the top row.
    | bSy - bEy        bEx - bSx |  | bSx - heSx |
    |                            |  |            |
    |                            |  | bSy - heSy |
    */
    numeratorX = heSx * (bEy  -  bSy) +
                 bSx  * (heSy -  bEy) +
                 bEx  * (bSy  - heSy);
    
    /*
    Here we calculate the bottom row.
    |                            |  | bSx - heSx |
    |                            |  |            |
    | heSy - heEy    heEx - heSx |  | bSy - heSy |
    */
    numeratorY = -(heSx * (bSy  -  heEy) +
                   heEx * (heSy -  bSy) +
                   bSx  * (heEy  - heSy));
    
    /* Use parameters to convert to the intersection point */
    t1 = numeratorX/denominator;
    t2 = numeratorY/denominator;
    *x = heSx + t1 * (heEx - heSx);
    *y = heSy + t1 * (heEy - heSy);
    
    /* Make final decision - if point is on segments, parameter values will be
    between 0, the start of the line segment, and 1, the end of the line segment.
    */
    if (0.0 < t1 && t1 < 1.0 && 0.0 < t2 && t2 < 1.0){
        return INTERSECT;
    } else if(t1 < 0.0 || 1.0 < t1 || t2 < 0.0 || 1.0 < t2){
        /* s or t outside of line segment. */
        return DOESNT_INTERSECT;
    } else {
        /* 
        ((numeratorX == 0) || (numeratorY == 0) || 
         (numeratorX == denominator) || (numeratorY == denominator))
        */
        return ENDS_OVERLAP;
    }
}

int getRelativeDir(double x, double y, vertex_t *v1, vertex_t *v2){
    /* Here we're doing a simple half-plane check against the vector v1->v2. */
    double x1 = v1->x;
    double x2 = v2->x;
    double y1 = v1->y;
    double y2 = v2->y;
    if(x1 == x2 && y1 == y2){
        /* Same point. */
        return DIR_UNDECIDED;
    } else if(x1 == x2){
        /* y = c line */
        /* Work out whether line is going up or down. */
        if(y2 > y1){
            if(x > x1){
                return INSIDE;
            } else if(x < x1){
                return OUTSIDE;
            } else {
                return DIR_UNDECIDED;
            }
        } else {
            if(x < x1){
                return INSIDE;
            } else if(x > x1){
                return OUTSIDE;
            } else {
                return DIR_UNDECIDED;
            }
        }
    } else if(y1 == y2){
        /* x = c line */
        /* Work out whether line is going left or right. */
        if(x2 > x1){
            if(y < y1){
                return INSIDE;
            } else if(y > y1){
                return OUTSIDE;
            } else {
                return DIR_UNDECIDED;
            }
        } else {
            if(y > y1){
                return INSIDE;
            } else if(y < y1){
                return OUTSIDE;
            } else {
                return DIR_UNDECIDED;
            }
        }
    }
    
    /* 
        x1, x2, y1, y2 distinct, so see whether point being tested is
        above or below gradient line.
    */
    double m = (y2 - y1)/(x2 - x1);
    double c = y1 - m*x1;
    
    double predictedY = x * m + c;
    double residual = y - predictedY;
    
    /*
        Being inside or outside the polygon depends on the direction
        the half-edge is going.
    */
    if(x2 > x1){
        if(residual < 0){
            return INSIDE;
        } else if(residual > 0){
            return OUTSIDE;
        } else {
            return DIR_UNDECIDED;
        }
    } else {
        if(residual > 0){
            return INSIDE;
        } else if(residual < 0){
            return OUTSIDE;
        } else {
            return DIR_UNDECIDED;
        }
    }
};