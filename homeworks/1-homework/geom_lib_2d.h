//UMN CSCI 5607 2D Geometry Library Homework [HW0]
//TODO: For the 18 functions below, replace their sub function with a working version that matches the desciption.

#ifndef GEOM_LIB_H
#define GEOM_LIB_H

#include "pga.h"

//Displace a point p on the direction d
//The result is a point
Point2D move(Point2D p, Dir2D d){
    return p + d;
}

//Compute the displacement vector between points p1 and p2
//The result is a direction 
Dir2D displacement(Point2D p1, Point2D p2){
    return p2 - p1;
}

//Compute the distance between points p1 and p2
//The result is a scalar 
float dist(Point2D p1, Point2D p2){
    return vee(p1, p2).magnitude();
}

//Compute the perpendicular distance from the point p the the line l
//The result is a scalar 
float dist(Line2D l, Point2D p){
    return vee(p, l.normalized());
}

//Compute the perpendicular distance from the point p the the line l
//The result is a scalar 
float dist(Point2D p, Line2D l){
    return dist(l, p);  // TODO seems sketchy
}

//Compute the intersection point between lines l1 and l2
//You may assume the lines are not parallel
//The results is a a point that lies on both lines
Point2D intersect(Line2D l1, Line2D l2){
    return wedge(l1, l2);
}

//Compute the line that goes through the points p1 and p2
//The result is a line 
Line2D join(Point2D p1, Point2D p2){
    return vee(p1, p2);
}

//Compute the projection of the point p onto line l
//The result is the closest point to p that lies on line l
Point2D project(Point2D p, Line2D l){
    Line2D d = dot(l, p);
    return dot(d, l) + wedge(d, l);
}

//Compute the projection of the line l onto point p
//The result is a line that lies on point p in the same direction of l
Line2D project(Line2D l, Point2D p){
    Line2D d = dot(l, p);
    return dot(d, p) + wedge(d, p);
}

//Compute the angle point between lines l1 and l2
//You may assume the lines are not parallel
//The results is a scalar
float angle(Line2D l1, Line2D l2){
    return acos(dot(l1.normalized(), l2.normalized()));
}

//Compute if the line segment p1->p2 intersects the line segment a->b
//The result is a boolean
bool segmentSegmentIntersect(Point2D p1, Point2D p2, Point2D a, Point2D b){
    // Compute normalized line segments
    Line2D l1 = join(p1, p2).normalized();
    Line2D l2 = join(a, b).normalized();
    
    // Compute signs for each point in regards to the opposite line
    // Signs should be opposite for each pair of points
    int sp1 = sign(vee(p1, l2));
    int sp2 = sign(vee(p2, l2));
    if (sp1 == sp2) return false;
    
    int sa = sign(vee(a, l1));
    int sb = sign(vee(b, l1));
    if (sa == sb) return false;
    
    return true;
    
}

//Compute if the point p lies inside the triangle t1,t2,t3
//Your code should work for both clockwise and counterclockwise windings
//The result is a bool
bool pointInTriangle(Point2D p, Point2D t1, Point2D t2, Point2D t3){
    // Compute lines between vertices of triangle
    Line2D e1 = join(t1, t2).normalized();
    Line2D e2 = join(t2, t3).normalized();
    Line2D e3 = join(t3, t1).normalized();
    
    // Compute signs for each point/edge
    int s1 = sign(vee(p, e1));
    int s2 = sign(vee(p, e2));
    int s3 = sign(vee(p, e3));
    
    // Point is in triangle if all three signs match
    if (s1 == s2 && s2 == s3) return true;
    
    return false;
}

//Compute the area of the triangle t1,t2,t3
//The result is a scalar
float areaTriangle(Point2D t1, Point2D t2, Point2D t3){
    return 0.5 * (vee(t1, vee(t2, t3)));
}

//Compute the distance from the point p to the triangle t1,t2,t3 as defined 
//by it's distance from the edge closest to p.
//The result is a scalar
float pointTriangleEdgeDist(Point2D p, Point2D t1, Point2D t2, Point2D t3){  // TODO maybe compare through tolerance for floats
    // Compute lines between vertices of triangle
    Line2D e1 = join(t1, t2);
    Line2D e2 = join(t2, t3);
    Line2D e3 = join(t3, t1);
    
    // Project point onto each edge line
    Point2D p1 = project(p, e1);
    Point2D p2 = project(p, e2);
    Point2D p3 = project(p, e3);
    
    // Clamp point to nearest endpoint if not on segment
    if (!((dist(p1, t1) + dist(p1, t2)) == dist(t1, t2))) {  // test if point is already on segment
        if (dist(p1, t1) < dist(p1, t2)) {  // if not, clamp to endpoints
            p1 = t1;
        } else {
            p1 = t2;
        }
    }
    if (!((dist(p2, t2) + dist(p2, t3)) == dist(t2, t3))) {  // same for other two projected points and edges
        if (dist(p2, t2) < dist(p2, t3)) {
            p2 = t2;
        } else {
            p2 = t3;
        }
    }
    if (!((dist(p3, t3) + dist(p3, t1)) == dist(t3, t1))) {  // same for other two projected points and edges
        if (dist(p3, t3) < dist(p3, t1)) {
            p3 = t3;
        } else {
            p3 = t1;
        }
    }
    
    // Return smallest distance
    float d1 = dist(p, p1);
    float d2 = dist(p, p2);
    float d3 = dist(p, p3);
    
    if (d1 <= d2 && d1 <= d3) {
        return d1;
    } else if (d2 <= d1 && d2 <= d3) {
        return d2;
    }
    
    return d3;
    
}

//Compute the distance from the point p to the closest of three corners of
// the triangle t1,t2,t3
//The result is a scalar
float pointTriangleCornerDist(Point2D p, Point2D t1, Point2D t2, Point2D t3){
    float pt1 = vee(p, t1).magnitude();
    float pt2 = vee(p, t2).magnitude();
    float pt3 = vee(p, t3).magnitude();
    
    if (pt1 <= pt2 && pt1 <= pt3) {
        return pt1;
    } else if (pt2 <= pt1 && pt2 <= pt3) {
        return pt2;
    }
    
    return pt3;
}

//Compute if the quad (p1,p2,p3,p4) is convex.
//Your code should work for both clockwise and counterclockwise windings
//The result is a boolean
bool isConvex_Quad(Point2D p1, Point2D p2, Point2D p3, Point2D p4){
    Line2D e1 = join(p1, p2);
    Line2D e2 = join(p2, p3);
    Line2D e3 = join(p3, p4);
    Line2D e4 = join(p4, p1);
    
    return sign(vee(p3, e1)) == sign(vee(p4, e1)) &&
            sign(vee(p1, e2)) == sign(vee(p4, e2)) &&
            sign(vee(p1, e3)) == sign(vee(p2, e3)) &&
            sign(vee(p3, e4)) == sign(vee(p3, e4));
}

//Compute the reflection of the point p about the line l
//The result is a point
Point2D reflect(Point2D p, Line2D l){
    // sandwich - lpl
    return l.normalized() * p * l.normalized();
}

//Compute the reflection of the line d about the line l
//The result is a line
Line2D reflect(Line2D d, Line2D l){
    // sandwich
    return (l.normalized() * d * l.normalized()).normalized();
}

#endif
