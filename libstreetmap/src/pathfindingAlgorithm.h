/* Author: Marius Stan
 * Class pathfindingAlgorithm finds valid path between two points on the map using Dijkstra's Algorithm (A*)
 */

#ifndef PATHFINDINGALGORITHM_H
#define PATHFINDINGALGORITHM_H

#include "StreetsDatabaseAPI.h"
#include "m1.h"
#include "graphics.h"
#include "Global.h"
#include "Global2.h"
#include <string>
#include <queue>
#include <unistd.h>
#include <list>
#include <algorithm>

#define NO_EDGE -1
#define NO_ID -1
#define STREET_DOES_NOT_EXIST -1
#define POI_RETURN_AMOUNT 10

using namespace std;

typedef unordered_map<pair<IntersectionIndex, IntersectionIndex>, double> pathUnorderedMap;
typedef unordered_map<IntersectionIndex, pair<IntersectionIndex, double>> depotUnorderedMap;

//For POI lookup, stores a vector of POI ids that match a given POI string

struct POI_node {
    std::string name;
    std::vector<unsigned> POI_ids;
};

//Street Segment Node

struct streetSegment {
    unsigned segmentID;
    double travelTime;
    unsigned fromNode, toNode;
    streetSegment* next;
};

//Stores times to node depending on previous street

struct timesToNode {
    unsigned streetID; //current street being traveled on 
    //Previous intersections & street segments
    int previousSegmentID, previousNodeID;
    //Best time to node based on street
    double shortestTimeToNode;
};

//Intersection Node

struct sourceNode {
    //Linked list storing all connected street segments
    streetSegment* head;

};

//Structure for wave element used for a*

struct waveElement {
    int nodeID;
    int prevStreetsegment;
    int previousNodeID;
    double timeToNode;
    double timeToEnd; //Estimated time to end intersection, using max speed limit and geometric distance
    double combinedTime; //Combination of time to node and estimated time to destination

    //Overloaded operators for priority queue 

    bool operator<(const waveElement& rhs) const {
        return combinedTime > rhs.combinedTime;
    }

    bool operator>(const waveElement& rhs) const {
        return combinedTime < rhs.combinedTime;
    }

    //Constructor

    waveElement(int n1, int n2, int n3, double n4, double n5, double n6) {
        nodeID = n1;
        prevStreetsegment = n2;
        previousNodeID = n3;
        timeToNode = n4;
        timeToEnd = n5;
        combinedTime = n6;
    }
};

//Structure for wave element, used only for dijkstra's

struct waveElement2 {
    int nodeID;
    int prevStreetsegment;
    int previousNodeID;
    double timeToNode;

    //Overloaded operators for priority queue 

    bool operator<(const waveElement2& rhs) const {
        return timeToNode > rhs.timeToNode;
    }

    bool operator>(const waveElement2& rhs) const {
        return timeToNode < rhs.timeToNode;
    }
    //Constructor

    waveElement2(int n1, int n2, int n3, double n4) {
        nodeID = n1;
        prevStreetsegment = n2;
        previousNodeID = n3;
        timeToNode = n4;
    }
};

class pathfindingGraph {
private:
    //Vector of intersections, index matches streetDatabse id
    std::vector<sourceNode> intersections;
    unsigned totalIntersections;

    //Vector of street segments storing ids of matching street
    std::vector<unsigned> streetSegments;

    //Structure of Point of interest names, with duplicates together in a vector
    std::vector<POI_node> POI_name_data;
    void build_POI_nameData();

    //Global best should be used only for points of interest, when calculating distances to multiple POIs of the same name
    double globalBest;
    int globalBestID;

    //Vectors of bools for which end intersections are valid when building the hash tables
    std::vector<bool> validIntersections;
    std::vector<std::pair<unsigned, double>> hashIntersectionDistances;
    std::vector<bool> validDepotIntersections;

    //Private functions for building/deleting linked lists
    streetSegment* buildLinkedList(unsigned id);
    void removeLinkedList(unsigned id);

    //Private pathfinding algorithm functions, call one of public functions to use them
    bool path_found_multiple_intersections(depotUnorderedMap& theDepotMap, pathUnorderedMap& thePathMap, const unsigned startID, const double turnCost, const unsigned numEndIntersections);
    bool path_found_depot_start(const unsigned startID, const double turnCost, const unsigned numEndIntersections);
    bool path_found(const unsigned startID, const unsigned endID, const double turnCost, std::vector<std::vector<timesToNode>>&shortestTimesTemp);

public:
    pathfindingGraph();
    ~pathfindingGraph();

    //IF YOU CALL THIS OUTSIDE OF m3, MAKE SURE TO USE reset_global_best() AFTER EVERY CALL, OTHERWISE FUTURE RESULTS WILL BE WRONG
    //This is a* for one endpoint, DO NOT attempt to multi-thread calls to this function
    std::vector<unsigned> find_shortest_path(const unsigned intersect_id_start, const unsigned intersect_id_end, const double turn_penalty);

    //dijkstra's algorithm will multiple end points, do not call unless you need to rebuild hash table
    bool build_travel_time_hash_table(pathUnorderedMap& thePathMap, depotUnorderedMap& theDepotMap, depotUnorderedMap& DepotDelivMap,const std::vector<unsigned>& depots, const std::vector<unsigned>& courierIntersections, const std::vector<unsigned>& dropOffs, const float turnCost);

    //Fast geometric distance
    double fast_distance(unsigned intersection1, unsigned intersection2);

    //Note: this function does not return a streetDatabase ID, it returns an id to be used in extraOSMdata's POI list
    std::vector<unsigned> get_POI_ids_from_name(std::string POIname);
    std::vector<int> find_closest_POI(std::string POIname, const unsigned intersect_id_start);

    //Global best modifying functions (for pathfinding)
    void set_global_best(double newBest);
    double get_best_travel_time();
    void reset_global_best();

    //Testing function
    void draw_segment(unsigned id);
};



#endif /* PATHFINDINGALGORITHM_H */

