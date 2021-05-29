#include "csg.h"
void CSGNode::ComputeTBN(const Point3f &P, Normal3f *nor, Vector3f *tan, Vector3f *bit) const {
    *nor = glm::normalize(transform.invTransT() * Normal3f(0,0,1));
    CoordinateSystem(*nor, tan, bit);
}

bool segConnected(CSGSegment& s1, CSGSegment& s2) {
    // If the 2 geometry doesn't have overlap
    if ((s1.rightT < s2.leftT) || (s2.rightT < s1.leftT)) {
        return false;
    }
    return true;
}

CSGSegment CSGSegMerge(CSGSegment& seg1, CSGSegment& seg2, CSGNodeType Opt) {
    // Convert the table in Dr.Badler's slides to code
    if (Opt == UNION) {
        CSGSegment resSegment;
        // Find the most left intersection point
        if (seg1.leftT < seg2.leftT) {
            resSegment.leftT = seg1.leftT;
            resSegment.leftInsect = seg1.leftInsect;
        }
        else {
            resSegment.leftT = seg2.leftT;
            resSegment.leftInsect = seg2.leftInsect;
        }
        // Find the most right intersection point
        if (seg1.rightT > seg2.rightT) {
            resSegment.rightT = seg1.rightT;
            resSegment.rightInsect = seg1.rightInsect;
        }
        else {
            resSegment.rightT = seg2.rightT;
            resSegment.rightInsect = seg2.rightInsect;
        }
        // Finish Union set up
        return resSegment;
    }
    else if (Opt == DIFFERENCE) {
        // If we use seg1 - seg2
        // There are 4 cases
        // Case 1: S1 - S2 = 0, nothing left
        if (seg1.leftT == seg1.rightT) {
            CSGSegment resSegment;
            resSegment = seg1;
            return resSegment;
        }

        // Case 2: S1 - S2 != 0 and S1, S2 are connected
        if ((seg1.leftT > seg2.leftT) && (seg1.rightT < seg2.rightT)) {
            CSGSegment resSegment;
            resSegment.leftT = seg1.leftT;
            resSegment.rightT = seg1.leftT;
            return resSegment;
        }
        // S2 at the left part of S1, S1 right remains
        else if ((seg2.rightT > seg1.leftT) && (seg2.rightT < seg1.rightT)) {
            CSGSegment resSegment;
            resSegment.leftT = seg2.rightT;
            resSegment.leftInsect = seg2.rightInsect;
            return resSegment;
        }
        // S2 at the right part of S1, S1 left remains
        else if ((seg2.leftT > seg1.leftT) && (seg2.leftT < seg1.rightT)) {
            CSGSegment resSegment;
            resSegment.leftT = seg1.leftT;
            resSegment.leftInsect = seg1.leftInsect;
            resSegment.rightT = seg2.leftT;
            resSegment.rightInsect = seg2.leftInsect;
            return resSegment;
        }
    }
    else if (Opt == INTERSECTION) {
        // This part is similar to UNION
        CSGSegment resSegment;
        // Get largest leftT:
        if (seg1.leftT < seg2.leftT) {
            resSegment.leftT = seg2.leftT;
            resSegment.leftInsect = seg2.leftInsect;
        }
        else {
            resSegment.leftT = seg1.leftT;
            resSegment.leftInsect = seg1.leftInsect;
        }
        // Get smallest rightT:
        if (seg1.rightT > seg2.rightT) {
            resSegment.rightT = seg2.rightT;
            resSegment.rightInsect = seg2.rightInsect;
        }
        else {
            resSegment.rightT = seg1.rightT;
            resSegment.rightInsect = seg1.rightInsect;
        }
        return resSegment;
    }

    return CSGSegment();
}

void UnionCombineHelp(std::list<CSGSegment>& resSegList, CSGSegment& s, CSGNodeType Opt)
{
    if (resSegList.size() == 0) {
        resSegList.push_back(s);
    }
    else {
        if (!segConnected(resSegList.back(), s)) {
            // If the input two geometries are not connected:
            // No operation needs to be done
            resSegList.push_back(s);
        }
        else {
            // If the input two geometrise are connected
            // Call the cooresponding Operation type
            CSGSegment mergedSeg = CSGSegMerge(resSegList.back(), s, Opt);
            // Remove the operated segment
            resSegList.pop_back();
            // Add the operated result segement
            resSegList.push_back(mergedSeg);
        }
    }
}

void CombineSegments(std::list<CSGSegment>& LL, std::list<CSGSegment>& RL, std::list<CSGSegment>& segments, CSGNodeType Opt)
{
    if (Opt == UNION) {
        // For all the segments in the scene, sort them by the first
        // intersetion's distance
        // First check for potential empty branch
        if (LL.empty() && RL.empty()) {
            return;
        }
        else if (LL.empty() && !RL.empty()) {
            segments = RL;
            return;
        }
        else if (!LL.empty() && RL.empty()) {
            segments = LL;
            return;
        }
        auto LIterator = LL.begin();
        auto RIterator = RL.begin();
        while (LIterator != LL.end() && RIterator != RL.end()) {
            if (LIterator == LL.end()) {
                // Combine all the Right Children
                for (;RIterator != RL.end(); ++RIterator) {
                    UnionCombineHelp(segments, *RIterator, UNION);
                }
            }
            if (RIterator == RL.end()) {
                // Combine all the Left Children
                for (;LIterator != LL.end(); ++LIterator) {
                    UnionCombineHelp(segments, *LIterator, UNION);
                }
            }
            if (LIterator->leftT < RIterator->leftT) {
                UnionCombineHelp(segments, *LIterator, UNION);
                ++LIterator;
            }
            else {
                UnionCombineHelp(segments, *RIterator, UNION);
                ++RIterator;
            }
        }
    }
    else if (Opt == INTERSECTION) {
        // First check for potential empty branch
        if (LL.empty() || RL.empty()) {
            return;
        }
        // Iterate over all the segments in Left List and Right List
        for (auto LIterator = LL.begin(); LIterator != LL.end(); ++LIterator) {
            for(auto RIterator = RL.begin(); RIterator != RL.end(); ++RIterator) {
                if (segConnected(*LIterator, *RIterator)) {
                    CSGSegment tempSeg = CSGSegMerge(*LIterator, *RIterator, INTERSECTION);
                    segments.push_back(tempSeg);
                }
            }
        }
    }
    else if (Opt == DIFFERENCE) {
        // First check for potential empty branch
        if (LL.empty()) {
            return;
        }
        if (RL.empty()) {
            segments = LL;
            return;
        }
        for (auto LIterator = LL.begin(); LIterator != LL.end(); ++LIterator) {
            CSGSegment tempSeg = *LIterator;
            for (auto RIterator = RL.begin(); RIterator != RL.end(); ++RIterator) {
                if ((RIterator->leftT > tempSeg.rightT) || (tempSeg.leftT == tempSeg.rightT)) {
                    // If there is nothing left after difference or no overlap
                    break;
                }
                if (segConnected(tempSeg, *RIterator)) {
                    if ((tempSeg.leftT < RIterator->leftT) && (tempSeg.rightT > RIterator->rightT)) {
                        // The overlaping area is on the right side
                        CSGSegment rightTempSeg;
                        rightTempSeg.leftT = RIterator->rightT;
                        rightTempSeg.rightT = tempSeg.rightT;
                        rightTempSeg.leftInsect = RIterator->rightInsect;
                        rightTempSeg.rightInsect = tempSeg.rightInsect;
                        ++LIterator;
                        LL.insert(LIterator, rightTempSeg);
                        // After differencing between two geometries
                        // reset the position of the iterator
                        --LIterator;
                        --LIterator;
                        tempSeg.rightT = RIterator->leftT;
                        tempSeg.rightInsect = RIterator->leftInsect;
                    }
                    else {
                        tempSeg = CSGSegMerge(tempSeg, *RIterator, DIFFERENCE);
                    }
                }
            }
            // Check for geometry left
            if (tempSeg.leftT != tempSeg.rightT) {
                segments.push_back(tempSeg);
            }
        }
    }
}

void CSGNode::IntersectHelp(const Ray &ray, std::list<CSGSegment>& segments)
{
    // If the segement is a geometry rather than an operation
    if (mType == GEOMETRY) {
        Intersection tempIntersection;
        // Find the intersection infromation by calling
        // the geometry's intersect function and check the t value
        if (mGeomertry->Intersect(ray, &tempIntersection) &&
                tempIntersection.CSGT1 >= 0 &&
                tempIntersection.CSGT2 >= 0) {
            // Construct Segment for valid intersection info
            CSGSegment tempSeg;
            tempSeg.leftInsect = tempIntersection;
            tempSeg.rightInsect = tempIntersection;
            tempSeg.leftT = tempIntersection.CSGT1;
            tempSeg.rightT = tempIntersection.CSGT2;
            segments.push_back(tempSeg);
        }
        return;
    }
    else {
        // If current Node is not geometry
        // goto next child on left and right
        std::list<CSGSegment> LL;
        this->leftChild->IntersectHelp(ray, LL);
        std::list<CSGSegment> RL;
        this->rightChild->IntersectHelp(ray, RL);
        CombineSegments(LL, RL, segments, mType);
        return;
    }
}

// This function is from shape function to properly set the Intersect
bool CSGNode::Intersect(const Ray &ray, Intersection *isect) const
{
    // Construct the left children list
    std::list<CSGSegment> LL;
    this->leftChild->IntersectHelp(ray, LL);
    // Construct the right children list
    std::list<CSGSegment> RL;
    this->rightChild->IntersectHelp(ray, RL);
    std::list<CSGSegment> tempSegList;
    CombineSegments(LL, RL, tempSegList, mType);
    if(!tempSegList.empty()) {
        *isect = tempSegList.begin()->leftInsect;
        return true;
    }
    return false;
}
