/* Author: Jack Lee, Marius Stan, Matthew Chapleau
 * A higher level API based on "StreetsDatabaseAPI.h"
 * Data structures to accelerate load and run time
 */

#include "m1.h"
#include "StreetsDatabaseAPI.h"
#include "Global.h"
#include "Global2.h"
#include "Global3.h"
#include "Global4.h"
#include <math.h>
#include <algorithm>
#include <sstream>
#include <unordered_map>
#include <chrono>
#include <fstream>

#define Infinity 999999999
//#define PROFILE_LOAD

//Refer to class descriptions
ID * mapID;
Coord * mapCoord;
kdTree * kdTrees;
OSMdata * extraMapData;
visual * mapGraphics;
interface * mapInterface;
pathfindingGraph* streetGraph;

using namespace std;

bool load_map(string map) {

    if (!loadStreetsDatabaseBIN(map)) return false;

    //std::cout << "Loading data 0% (Loading Streets Database)" << "\n";
    //Opening extra OSM data
    for (unsigned i = 0; i <= 11; i++) {
        map.pop_back();
    }

    map = map + ".osm.bin";
    //std::cout << "Loading data 20% (Loading OSM Database)" << "\n";
    if (!loadOSMDatabaseBIN(map)) return false;


#ifdef PROFILE_LOAD
    static std::ofstream perf;
    perf.open("performance_data.csv", std::ios_base::app);
    auto const start = std::chrono::high_resolution_clock::now();
#endif
    //Dynamic allocation of data structure
    std::cout << "Loading data 30% (Loading Coordinate System)" << "\n";
    mapCoord = new Coord;
    std::cout << "Loading data 40% (Loading Street Data Structures)" << "\n";
    mapID = new ID;
    std::cout << "Loading data 60% (Loading OSM data structures)" << "\n";
    extraMapData = new OSMdata;

    std::cout << "Loading data 80% (Loading pathfinding algorithm)" << "\n";
    streetGraph = new pathfindingGraph;

    std::cout << "Loading data 90% (Loading kd Trees)" << "\n";
    kdTrees = new kdTree;

    std::cout << "Loading data 100% (Opening Graphics Window)" << "\n";
    mapInterface = new interface;


#ifdef PROFILE_LOAD
    auto const end = std::chrono::high_resolution_clock::now();
    auto const delta_time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    perf << "load(no multi-threading)-London," << delta_time.count() << std::endl;
    perf.close();
#endif

    return true;
}

void close_map() {
    //Dynamic deallocation of data structure
    delete streetGraph;
    delete mapID;
    delete mapCoord;
    delete kdTrees;
    delete extraMapData;
    closeOSMDatabase();
    closeStreetDatabase();
}

//Returns street id(s) for the given street name
//If no street with this name exists, returns a 0-length vector.

vector<unsigned> find_street_ids_from_name(string street_name) {
    return mapID->get_street_ids_from_street_name(street_name);
}

//Returns the street segments for the given intersection 

vector<unsigned> find_intersection_street_segments(unsigned intersection_id) {
    return mapID->get_segment_ids_from_intersection_id(intersection_id);
}

//Returns the street names at the given intersection (includes duplicate street names in returned vector)

vector<string> find_intersection_street_names(unsigned intersection_id) {
    vector<string> street_name;
    vector<StreetSegmentIndex> seg_ids = find_intersection_street_segments(intersection_id);

    //Loop through all street segments connected to <intersection_d>
    for (vector<StreetSegmentIndex>::iterator seg_iter = seg_ids.begin(); seg_iter != seg_ids.end(); seg_iter++) {
        street_name.push_back(getStreetName(getStreetSegmentInfo(*seg_iter).streetID));
    }

    return (street_name);
}

//Returns true if you can get from intersection1 to intersection2 using a single street segment (hint: check for 1-way streets too)
//corner case: an intersection is considered to be connected to itself

bool are_directly_connected(unsigned intersection_id1, unsigned intersection_id2) {
    vector<StreetSegmentIndex> sec1 = find_intersection_street_segments(intersection_id1);
    vector<StreetSegmentIndex> sec2 = find_intersection_street_segments(intersection_id2);

    //Loop through 2 sets of street segments, return<true> if found match
    for (vector<StreetSegmentIndex>::iterator sec1_iter = sec1.begin(); sec1_iter != sec1.end(); sec1_iter++) {
        for (vector<StreetSegmentIndex>::iterator sec2_iter = sec2.begin(); sec2_iter != sec2.end(); sec2_iter++) {
            if (*sec1_iter == *sec2_iter) {
                return (true);
            }
        }
    }

    return (false);
}

//Returns all intersections reachable by traveling down one street segment 
//from given intersection (hint: you can't travel the wrong way on a 1-way street)
//the returned vector should NOT contain duplicate intersections

vector<unsigned> find_adjacent_intersections(unsigned intersection_id) {
    vector<IntersectionIndex> adj_ids;
    vector<StreetSegmentIndex> seg_ids = find_intersection_street_segments(intersection_id);

    //Loop through all street segments connected to <intersection_d>
    for (vector<StreetSegmentIndex>::iterator seg_iter = seg_ids.begin(); seg_iter != seg_ids.end(); seg_iter++) {
        StreetSegmentInfo seg_info = getStreetSegmentInfo(*seg_iter);
        //excluding wrong way
        if (intersection_id == seg_info.from) {
            adj_ids.push_back(seg_info.to);
        } else if (!seg_info.oneWay) {
            adj_ids.push_back(seg_info.from);
        }
    }

    //Remove duplicates and erase left over vectors
    sort(adj_ids.begin(), adj_ids.end());
    adj_ids.erase(unique(adj_ids.begin(), adj_ids.end()), adj_ids.end());
    return (adj_ids);
}

//Returns all street segments for the given street

vector<unsigned> find_street_street_segments(unsigned street_id) {
    return mapID->get_segment_ids_from_street_id(street_id);
}

//Returns all intersections along the a given street

vector<unsigned> find_all_street_intersections(unsigned street_id) {
    return mapID->get_intersection_ids_from_street_id(street_id);
}

//Return all intersection ids for two intersecting streets
//This function will typically return one intersection id.
//However street names are not guaranteed to be unique, so more than 1 intersection id may exist

vector<unsigned> find_intersection_ids_from_street_names(string street_name1, string street_name2) {
    vector<IntersectionIndex> inter_ids;
    vector<IntersectionIndex> str_inter; //all intersections associated with both streets(with duplicates)
    vector<StreetIndex> str1 = find_street_ids_from_name(street_name1);
    vector<StreetIndex> str2 = find_street_ids_from_name(street_name2);

    //Find all intersections associated to <street_name>
    for (vector<StreetIndex>::iterator str1_iter = str1.begin(); str1_iter != str1.end(); str1_iter++) {
        vector<IntersectionIndex> temp = find_all_street_intersections(*str1_iter);
        //concatenate
        str_inter.reserve(str_inter.size() + temp.size());
        str_inter.insert(str_inter.end(), temp.begin(), temp.end());
    }
    for (vector<StreetIndex>::iterator str2_iter = str2.begin(); str2_iter != str2.end(); str2_iter++) {
        vector<IntersectionIndex> temp = find_all_street_intersections(*str2_iter);
        //concatenate
        str_inter.reserve(str_inter.size() + temp.size());
        str_inter.insert(str_inter.end(), temp.begin(), temp.end());
    }
    sort(str_inter.begin(), str_inter.end());
    int prev = -1;
    //Extract duplicates and insert them into <inter_ids>
    for (vector<IntersectionIndex>::iterator seg_iter = str_inter.begin() + 1; seg_iter != str_inter.end(); seg_iter++) {
        if (*seg_iter == *(seg_iter - 1) && prev != static_cast<int> (*seg_iter)) {
            prev = *seg_iter;
            inter_ids.push_back(*seg_iter);
        }
    }
    return inter_ids;
}

//Returns the distance between two coordinates in meters

double find_distance_between_two_points(LatLon point1, LatLon point2) {
    double x1, y1, x2, y2;
    double latavg;
    double d = 0;

    //converting coordinates from lat/lon to (x,y)
    y1 = DEG_TO_RAD * point1.lat();
    y2 = DEG_TO_RAD * point2.lat();
    latavg = DEG_TO_RAD * (point1.lat() + point2.lat()) / 2;


    x1 = DEG_TO_RAD * point1.lon() * cos(latavg);
    x2 = DEG_TO_RAD * point2.lon() * cos(latavg);

    //compute distance using formula given
    d = EARTH_RADIUS_IN_METERS * sqrt((y2 - y1)*(y2 - y1) + (x2 - x1)*(x2 - x1));
    return d;
}

//Returns the length of the given street segment in meters

double find_street_segment_length(unsigned street_segment_id) {
    return mapID->get_length_of_street_segment(street_segment_id);
}

//Returns the length of the specified street in meters

double find_street_length(unsigned street_id) {
    double length = 0;

    vector<unsigned> streetSegments;
    streetSegments = find_street_street_segments(street_id);

    //Goes through all street segments, checks if they have an identical street id, then calculates length accordingly
    for (unsigned i = 0; i < streetSegments.size(); i++) {
        length += find_street_segment_length(streetSegments[i]);
    }

    return length;
}

//Returns the travel time to drive a street segment in seconds (time = distance/speed_limit)

double find_street_segment_travel_time(unsigned street_segment_id) {
    return mapID->get_travel_time_of_street_segment(street_segment_id);
}

//Returns the nearest point of interest to the given position

unsigned find_closest_point_of_interest(LatLon my_position) {
    float tempDistance = std::numeric_limits<float>::infinity();

    unsigned closestID = 0;
    treeNode* root = kdTrees->getPOIroot();
    kdTrees->findClosest(root, true, &closestID, &tempDistance, my_position.lat(), my_position.lon());

    return closestID;
}

//Returns the the nearest intersection to the given position

unsigned find_closest_intersection(LatLon my_position) {
    float tempDistance = Infinity;
    unsigned closestID = 0;
    treeNode* root = kdTrees->getIntersectionRoot();
    kdTrees->findClosest(root, true, &closestID, &tempDistance, my_position.lat(), my_position.lon());

    return closestID;
}
