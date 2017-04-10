/* Author: Jack Lee, Marius Stan, Matthew Chapleau
 * Algorithm for finding shortest path
 */

#include "m3.h"
#include "m2.h"
#include "OSMDatabaseAPI.h"
#include "StreetsDatabaseAPI.h"
#include "pathfindingAlgorithm.h"
#include "Global2.h"
#include "Global3.h"
#include "Global4.h"
#include "visual.h"

using namespace std;



// Returns the time required to travel along the path specified, in seconds. 
// The path is given as a vector of street segment ids, and this function 
// can assume the vector either forms a legal path or has size == 0.
// The travel time is the sum of the length/speed-limit of each street 
// segment, plus the given turn_penalty (in seconds) per turn implied by the path. 
// A turn occurs when two consecutive street segments have different street IDs.

double compute_path_travel_time(const std::vector<unsigned>& path, const double turn_penalty) {
    double travelTime = 0, currentTurnDelay;
    unsigned previousStreet = 0, currentStreet;
    for (unsigned i = 0; i < path.size(); i++) {
        currentStreet = getStreetSegmentInfo(path[i]).streetID;
        //Calculating turn delay, if the loop has just begun there is no delay
        if (i == 0) {
            currentTurnDelay = 0;
        } else if (previousStreet == currentStreet) {
            currentTurnDelay = 0;
        } else {
            currentTurnDelay = turn_penalty;
        }

        previousStreet = currentStreet;

        travelTime += find_street_segment_travel_time(path[i]) + currentTurnDelay;
    }
    return travelTime;
}


// Returns a path (route) between the start intersection and the end 
// intersection, if one exists. This routine should return the shortest path
// between the given intersections when the time penalty to turn (change
// street IDs) is given by turn_penalty (in seconds).
// If no path exists, this routine returns an empty (size == 0) vector. 
// If more than one path exists, the path with the shortest travel time is 
// returned. The path is returned as a vector of street segment ids; traversing 
// these street segments, in the returned order, would take one from the start 
// to the end intersection.

std::vector<unsigned> find_path_between_intersections(const unsigned intersect_id_start, const unsigned intersect_id_end, const double turn_penalty) {
    vector<unsigned> path = streetGraph->find_shortest_path(intersect_id_start, intersect_id_end, turn_penalty);
    streetGraph->reset_global_best();
    return path;
}


// Returns the shortest travel time path (vector of street segments) from 
// the start intersection to a point of interest with the specified name.
// The path will begin at the specified intersection, and end on the 
// intersection that is closest (in Euclidean distance) to the point of 
// interest.
// If no such path exists, returns an empty (size == 0) vector.

std::vector<unsigned> find_path_to_point_of_interest(const unsigned intersect_id_start, const std::string point_of_interest_name, const double turn_penalty) {
    //Getting closest intersection

    unsigned shortestID = 0;
    bool valid = false;
    vector<int> closestPOIs = streetGraph->find_closest_POI(point_of_interest_name, intersect_id_start);
    vector<unsigned> shortestPath, tempPath;
    double shortestLength = std::numeric_limits<double>::infinity(), tempLength;

    //Loops through all returned points of interest and calculates the distance to all of them, it then compares their distances and picks the one with the smallest distance
    for (unsigned i = 0; i < closestPOIs.size(); i++) {
        if (closestPOIs[i] != -1) {
            //Convert to LatLon and find the closest intersections to all closest POIs
            LatLon temp(extraMapData->POI_get_position(closestPOIs[i]).first, extraMapData->POI_get_position(closestPOIs[i]).second);
            unsigned intersect_id_end = find_closest_intersection(temp);

            tempPath = streetGraph->find_shortest_path(intersect_id_start, intersect_id_end, turn_penalty);
            if (tempPath.size() != 0) {
                tempLength = compute_path_travel_time(tempPath, turn_penalty);

                //If new path is shorter, update all current bests
                if (tempLength < shortestLength) {
                    shortestPath = tempPath;
                    shortestLength = tempLength;
                    shortestID = closestPOIs[i];
                    valid = true;
                }
            }
        }
    }

    //If valid, call the interface to re-center the view around a potentially closer POI
    if (valid == true) {
        float world_x = mapCoord->lon_to_x(extraMapData->POI_get_position(shortestID).second);
        float world_y = mapCoord->lat_to_y(extraMapData->POI_get_position(shortestID).first);
        mapInterface->adjust_poi_panel_to_closest(world_x, world_y);
    }

    //Reseting global best to infinity
    streetGraph->reset_global_best();

    return shortestPath;
}
