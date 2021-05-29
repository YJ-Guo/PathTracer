#pragma once
#include <la.h>
#include <scene/geometry/mesh.h>

class Triangle;

struct KDNode {
    KDNode* leftChild;
    KDNode* rightChild;
    unsigned int axis; // Which axis split this node represents
    glm::vec3 minCorner, maxCorner; // The world-space bounds of this node
    std::vector<const Photon*> particles; // A collection of pointers to the particles contained in this node.
};


class KDTree {
public:
    KDTree();
    ~KDTree();

    void build(const std::vector<Photon> *points);
    KDNode* buildKdTree(const std::vector<const Photon*> &points, int depth);
    void searchKdTree(KDNode *node, std::vector<const Photon*> &resultPoints, const glm::vec3 &origin, const float &r) const;
    std::vector<const Photon*> particlesInSphere(const glm::vec3& c, const float& r) const; // Returns all the points contained within a sphere with center c and radius r
    void clear();

    KDNode* root;
    glm::vec3 minCorner, maxCorner; // For visualization purposes
    const std::vector<Photon> *mAllPhotons;
};
