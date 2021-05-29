#ifndef CSG_H
#define CSG_H
#include "shape.h"
#include "primitive.h"

enum CSGNodeType{
    UNION, DIFFERENCE, INTERSECTION, GEOMETRY
};

// Follow Dr.Badler's slids to store the intersection information
// for geometry boolean operation
// A Tree structure to store all the CSG components
struct CSGSegment
{
    Intersection leftInsect;
    float leftT;
    Intersection rightInsect;
    float rightT;
};

class CSGNode : public Shape
{
public:
    CSGNode* leftChild;
    CSGNode* rightChild;
    Shape* mGeomertry;
    CSGNodeType mType;

    // These functions are virtual that needs some defination
    // Follow the written in cube/sphere
    virtual Point2f GetUVCoordinates(const Point3f &point) const {return Point2f(point.x + 0.5f, point.y + 0.5f);}
    virtual void ComputeTBN(const Point3f& P, Normal3f* nor, Vector3f* tan, Vector3f* bit) const;
    virtual float Area() const {return 1.f;}
    virtual Intersection Sample(const Point2f &xi, Float *pdf) const {return Intersection();}
    void create(){}

    virtual bool Intersect(const Ray &ray, Intersection *isect) const;
    // Need to use std::list to better work with shared ptr
    void IntersectHelp(const Ray &ray, std::list<CSGSegment>& segments);

};

#endif // CSG_H
