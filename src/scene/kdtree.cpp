#include "kdtree.h"

KDTree::KDTree()
    : root(nullptr)
{}

KDTree::~KDTree()
{
    delete mAllPhotons;
    delete root;
}

// Comparator functions you can use with std::sort to sort vec3s along the cardinal axes
bool xSort(const Photon* a, const Photon* b) { return a->pos.x < b->pos.x; }
bool ySort(const Photon* a, const Photon* b) { return a->pos.y < b->pos.y; }
bool zSort(const Photon* a, const Photon* b) { return a->pos.z < b->pos.z; }

bool xTrisSort(Triangle* a, Triangle* b) { return a->middlePos.x < b->middlePos.x; }
bool yTrisSort(Triangle* a, Triangle* b) { return a->middlePos.y < b->middlePos.y; }
bool zTrisSort(Triangle* a, Triangle* b) { return a->middlePos.z < b->middlePos.z; }


void findCorner(const std::vector<const Photon*> &points, glm::vec3& minCorner, glm::vec3& maxCorner)
{
    minCorner = Point3f(FLT_MAX);
    maxCorner = Point3f(FLT_MIN);
    for(auto itr : points)
    {
        const Photon* tempPtr = itr;
        float x = tempPtr->pos.x;
        float y = tempPtr->pos.y;
        float z = tempPtr->pos.z;
        // Find the min and max value for x,y,z
        if (x > maxCorner.x) {
            maxCorner.x = x;
        }
        if (x < minCorner.x) {
            minCorner.x = x;
        }
        if (y > maxCorner.y) {
            maxCorner.y = y;
        }
        if (y < minCorner.y) {
            minCorner.y = y;
        }
        if (z > maxCorner.z) {
            maxCorner.z = z;
        }
        if (z < minCorner.z) {
            minCorner.z = z;
        }
    }
}

KDNode* KDTree::buildKdTree(const std::vector<const Photon *> &points,int depth)
{
    // If there is only one point to build the k-d tree
    if (points.size() == 1) {
        // Directly add this point as the root of the tree
        KDNode *curNode = new KDNode();
        findCorner(points, curNode->minCorner, curNode->maxCorner);
        curNode->particles.push_back(points.at(0));
        // curAxis indicates along which axis to partition
        // Axis Mapping: X - 0, Y - 1, Z - 2
        int curAxis = depth % 3;
        curNode->axis = curAxis;
        return curNode;
    } else {
        // For more than one point, we need to find the median point then partition
        std::vector<const Photon *> sortedPoints = points;
        int curAxis = depth % 3;
        // find which axis to partition with
        if (curAxis == 0) {
            std::sort(sortedPoints.begin(), sortedPoints.end(), xSort);
        } else if (curAxis == 1) {
            std::sort(sortedPoints.begin(), sortedPoints.end(), ySort);
        } else {
            std::sort(sortedPoints.begin(), sortedPoints.end(), zSort);
        }
        // Find the median point and partition into 2 parts
        int medianIndex = sortedPoints.size() / 2;
        std::vector<const Photon *> Part1(sortedPoints.begin(), sortedPoints.begin() + medianIndex);
        std::vector<const Photon *> Part2(sortedPoints.begin() + medianIndex, sortedPoints.end());
        // Recursively call the partition function until no point left in certain part
        KDNode *curNode = new KDNode();
        findCorner(points, curNode->minCorner, curNode->maxCorner);
        curNode->axis = curAxis;
        if (!Part1.empty()) {
            curNode->leftChild = buildKdTree(Part1, depth + 1);
        }
        if (!Part2.empty()) {
            curNode->rightChild = buildKdTree(Part2, depth + 1);
        }
        return curNode;
    }
}

void KDTree::build(const std::vector<Photon> *points)
{
    mAllPhotons = points;
    if (points->empty()) {
        return;
    }

    // Build pointers vector:
    std::vector<const Photon*> photonPtrVec(points->size());
    const std::vector<Photon>& photonVec = *mAllPhotons;
    for (unsigned int i = 0; i < points->size(); ++i) {
        photonPtrVec[i] = &photonVec[i];
    }
    findCorner(photonPtrVec, minCorner, maxCorner);
    this->root = buildKdTree(photonPtrVec, 1);
    std::cout << "KDTree Build Finished!" << std::endl;
}

inline float square(float n) {return n * n;}
// Check if the bounding box has intersection with the sphere
// For origin of the sphere outside the bounding box
bool boxSphereIntersectionTest(Point3f boxLeft, Point3f boxRight, Point3f sphereOrigin, float r) {
    float disRemain = square(r);
    // Calculate X
    if (sphereOrigin.x < boxLeft.x) {
        disRemain -= square(boxLeft.x - sphereOrigin.x);
    } else if (sphereOrigin.x > boxRight.x) {
        disRemain -= square(sphereOrigin.x - boxRight.x);
    }
    // Calculate Y
    if (sphereOrigin.y < boxLeft.y) {
        disRemain -= square(boxLeft.y - sphereOrigin.y);
    } else if (sphereOrigin.y > boxRight.y) {
        disRemain -= square(sphereOrigin.y - boxRight.y);
    }
    // Calculate Z
    if (sphereOrigin.z < boxLeft.z) {
        disRemain -= square(boxLeft.z - sphereOrigin.z);
    } else if (sphereOrigin.z > boxRight.z) {
        disRemain -= square(sphereOrigin.z - boxRight.z);
    }
    return disRemain > 0.f;
}

void KDTree::searchKdTree(KDNode *node, std::vector<const Photon*> &resultPoints, const glm::vec3 &origin, const float &r) const{
    // If there are points in the list and not both children are null
    // std::cout << "Search Current Node size: " << node->particles.size() << std::endl;
    if (node->particles.size() != 0 &&
            (node->leftChild == nullptr && node->rightChild == nullptr)) {
        // If the point is inside the sphere
        if (glm::distance(node->particles.at(0)->pos, origin) <= r) {
            // std::cout << "Check in Box Range" << std::endl;
            resultPoints.push_back(node->particles.at(0));
        }
    } else {
        if (boxSphereIntersectionTest(node->leftChild->minCorner, node->leftChild->maxCorner, origin, r)) {
            // std::cout << "Check left Box Edge" << std::endl;
            searchKdTree(node->leftChild, resultPoints, origin, r);
        }
        if (boxSphereIntersectionTest(node->rightChild->minCorner, node->rightChild->maxCorner, origin, r)) {
            // std::cout << "Check Right Box Edge" << std::endl;
            searchKdTree(node->rightChild, resultPoints, origin, r);
        }
    }
}

std::vector<const Photon*> KDTree::particlesInSphere(const glm::vec3& c, const float& r) const
{
    std::vector<const Photon*> resultsPoints;
    searchKdTree(this->root, resultsPoints, c, r);
    //std::cout << "Reslut size: " << resultsPoints.size() << std::endl;
    return resultsPoints;
}

void KDTree::clear()
{
    delete root;
    root = nullptr;
}
