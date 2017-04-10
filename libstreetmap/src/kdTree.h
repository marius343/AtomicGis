/* Author: Marius Stan
 * K-D tree class includes functions that build and access the tree
 */

#ifndef KDTREE_H
#define KDTREE_H

#include "m1.h"
#include "StreetsDatabaseAPI.h"
#include "ID.h"
#include "treeNode.h"
#include <math.h>
#include <algorithm>
#include <sstream>
#include <queue>

struct coordinate {
    unsigned id;
    float lat;
    float lon;
};

//Used for priority queue for when searching for k nearest elements in the kd tree, largest element should be stored first

struct topClosestElements {
    int nodeID;
    double distance;

    //Overloaded operators for priority queue 

    bool operator<(const topClosestElements& rhs) const {
        return distance < rhs.distance;
    }

    bool operator>(const topClosestElements& rhs) const {
        return distance > rhs.distance;
    }
    //Constructor

    topClosestElements(int n1, double n2) {
        nodeID = n1;
        distance = n2;
    }
};

class kdTree {
private:
    coordinate* tempArray; //temporary array used for building k-d trees
    treeNode* POIroot;
    treeNode* intersectionRoot;
    treeNode* depotRoot;
    std::priority_queue<topClosestElements> closestDistances;

public:
    kdTree();
    void build_POI_kdTree();
    void build_Intersection_kdTree();
    void build_Depot_KD_Tree(std::vector<coordinate> depots);
    float findDistance(float lat1, float lon1, float lat2, float lon2);
    friend bool compareLat(const coordinate &a, const coordinate &b);
    friend bool compareLon(const coordinate &a, const coordinate &b);
    treeNode* createKDtree(unsigned left, unsigned right, bool cmpLat);
    void findClosest(treeNode* newRoot, bool cmpLat, unsigned* closestID, float* distance, float lat, float lon);
    void findClosestMultiple(treeNode* newRoot, bool cmpLat, float lat, float lon);

    //Use this function to return the closest map element to a clicked point (char = i means intersection, char = p means POI, char = n means the user didn't click on anything)
    std::pair<char, unsigned> findClosestElement(float lat, float lon);

    std::vector<unsigned> find_closest_depots(float lat, float lon, unsigned number_of_return_elements);


    float findDistance_xy(float x1, float y1, float x2, float y2);

    void deleteDepotKDtree();
    treeNode* getPOIroot();
    treeNode* getIntersectionRoot();
    ~kdTree();

};

bool compareLat(const coordinate &a, const coordinate &b);
bool compareLon(const coordinate &a, const coordinate &b);

#endif /* KDTREE_H */

