/* Author: Marius Stan
 * Class pathfindingAlgorithm finds valid path between two points on the map using Dijkstra's Algorithm (A*)
 */

#include "pathfindingAlgorithm.h"
#include "Global4.h"

pathfindingGraph::pathfindingGraph() {
    totalIntersections = getNumberOfIntersections();
    intersections.resize(getNumberOfIntersections());
    streetSegments.resize(getNumberOfStreetSegments());
    //Loops through all intersections and adds them as nodes in the vector
    for (unsigned i = 0; i < getNumberOfIntersections(); i++) {
        intersections[i].head = buildLinkedList(i);
    }

    for (unsigned i = 0; i < getNumberOfStreetSegments(); i++) {
        streetSegments[i] = getStreetSegmentInfo(i).streetID;
    }

    hashIntersectionDistances.assign(totalIntersections, std::make_pair(0, std::numeric_limits<double>::infinity()));


    validIntersections.assign(totalIntersections, false);
    validDepotIntersections.assign(totalIntersections, false);

    build_POI_nameData();
    reset_global_best();
}

//Builds a linked list of connected street segments from a given id, returns the head

streetSegment* pathfindingGraph::buildLinkedList(unsigned id) {
    std::vector<unsigned> tempSegments;
    tempSegments = find_intersection_street_segments(id);
    streetSegment *currentNode, *newNode, *head;
    bool dontAddSegment = false;
    if (tempSegments.size() == 0) return NULL; //If there are no connected street segments return NULL

    //Building the head
    head = NULL;

    //Loops through all street segments and adds them to linked list
    for (unsigned i = 0; i < tempSegments.size(); i++) {
        newNode = new streetSegment();
        newNode->fromNode = id;
        newNode->segmentID = tempSegments[i];
        newNode->travelTime = find_street_segment_travel_time(tempSegments[i]);
        newNode->next = NULL;
        //Checking to make sure to and from aren't the same id
        if (getStreetSegmentInfo(tempSegments[i]).from == id) {
            newNode->toNode = getStreetSegmentInfo(tempSegments[i]).to;
        } else if (getStreetSegmentInfo(tempSegments[i]).oneWay != true) {
            newNode->toNode = getStreetSegmentInfo(tempSegments[i]).from;
        } else {
            dontAddSegment = true;
        }

        //Only add the segment if the street is not one way or if its one-way in the right direction
        if (dontAddSegment != true) {
            if (head == NULL) {
                head = newNode;
            } else {
                currentNode->next = newNode;
            }
            currentNode = newNode;
        } else {
            delete newNode;
        }

        dontAddSegment = false;
    }


    return head;
}

/*
 * Takes in two intersection ids and returns the shortest path between them
 * This works by checking the times stored in a vector at each node starting at the ending node
 * The times stored in the node are stored independently for each street used to get to the intersection
 * This is used to ensure the best path is returned when there are turn delays, as the shortest time to 
 * a node is not necessarily the best path to take to the destination if a turn must be made to get to
 * the next node.
 */
std::vector<unsigned> pathfindingGraph::find_shortest_path(const unsigned intersect_id_start, const unsigned intersect_id_end, const double turn_penalty) {
    //If start ID is the same as the end ID, return an empty vector
    std::vector<unsigned> thePath;
    if (intersect_id_start == intersect_id_end) return thePath;
    std::vector<std::vector < timesToNode>> shortestTimesTemp;
    shortestTimesTemp.resize(totalIntersections);
    unsigned nextStreet, tempIndex = 0; //Next street to go on, used for calculating potential turn delay
    double currentTurnDelay;
    //Checking if a path exists
    bool found = path_found(intersect_id_start, intersect_id_end, turn_penalty, shortestTimesTemp);

    if (found == true) {
        //Finding shortest travel time to the end node and picking that as the starting point
        for (unsigned i = 0; i < shortestTimesTemp[intersect_id_end].size(); i++) {
            if (shortestTimesTemp[intersect_id_end][i].shortestTimeToNode < shortestTimesTemp[intersect_id_end][tempIndex].shortestTimeToNode) tempIndex = i;
        }

        //Getting initial data
        int previousEdge = shortestTimesTemp[intersect_id_end][tempIndex].previousSegmentID;
        int currentNode = shortestTimesTemp[intersect_id_end][tempIndex].previousNodeID;
        nextStreet = shortestTimesTemp[intersect_id_end][tempIndex].streetID;
        thePath.push_back(previousEdge);
        //Keeps going through nodes until no more edges are found
        while (static_cast<unsigned> (currentNode) != intersect_id_start) {
            double currentBest = std::numeric_limits<double>::infinity();
            //Goes through the shortest times stored at a node and sees which one is actually the shortest after adding a potential turn delay
            for (unsigned i = 0; i < shortestTimesTemp[currentNode].size(); i++) {
                //Calculating potential turn delay
                if (nextStreet != shortestTimesTemp[currentNode][i].streetID) currentTurnDelay = turn_penalty;
                else currentTurnDelay = 0;

                //Seeing if the path plus the potential turn delay is the shortest
                if (shortestTimesTemp[currentNode][i].shortestTimeToNode + currentTurnDelay < currentBest) {
                    currentBest = shortestTimesTemp[currentNode][i].shortestTimeToNode + currentTurnDelay;
                    tempIndex = i;
                }
            }
            //Updating everything    
            previousEdge = shortestTimesTemp[currentNode][tempIndex].previousSegmentID;
            nextStreet = shortestTimesTemp[currentNode][tempIndex].streetID;
            currentNode = shortestTimesTemp[currentNode][tempIndex].previousNodeID;
            thePath.push_back(previousEdge);
        }

        //Reversing the vector (its currently backwards)
        std::reverse(thePath.begin(), thePath.end());
    }

    return thePath;
}

//Checks if the path exists, this uses a modified version of a* algorithm with current bests stored at each node

bool pathfindingGraph::path_found(const unsigned startID, const unsigned endID, const double turnCost, std::vector<std::vector<timesToNode>>&shortestTimesTemp) {
    //Current best time (infinite upon initialization)
    double currentBest = std::numeric_limits<double>::infinity();
    double currentTurnDelay, tempTimeToEnd, combinedTime;
    int previousStreet, currentStreet;
    bool found = false, valid = false;
    //Creating a priority queue, node with shortest travel time 
    std::priority_queue<waveElement> wavefront;
    tempTimeToEnd = fast_distance(startID, endID) / mapID->get_max_speed_limit();
    waveElement tempElement(startID, NO_EDGE, NO_EDGE, 0, tempTimeToEnd, 0);
    wavefront.push(tempElement);



    while (wavefront.empty() == false) {
        valid = false;
        waveElement currentE = wavefront.top();
        wavefront.pop();

        int tempArrayLocation = STREET_DOES_NOT_EXIST;

        //cout<<intersections[currentE.nodeID].shortestTimes.size()<<endl;
        //Searching to see if the street exists in the intersections array already
        for (unsigned i = 0; i < shortestTimesTemp[currentE.nodeID].size(); i++) {
            if (shortestTimesTemp[currentE.nodeID][i].streetID == streetSegments[currentE.prevStreetsegment]) {
                tempArrayLocation = i;
                break;
            }
        }

        //If the street does not exist yet and the travel time is less than the current best, proceed
        if (tempArrayLocation == STREET_DOES_NOT_EXIST) {
            if (currentE.timeToNode < currentBest && currentE.timeToNode < globalBest && currentE.combinedTime < currentBest) {
                valid = true;
                timesToNode temporaryStructure;
                if (currentE.prevStreetsegment != NO_EDGE) temporaryStructure.streetID = streetSegments[currentE.prevStreetsegment];
                else temporaryStructure.streetID = NO_EDGE;
                temporaryStructure.previousSegmentID = currentE.prevStreetsegment;
                temporaryStructure.previousNodeID = currentE.previousNodeID;
                temporaryStructure.shortestTimeToNode = currentE.timeToNode;
                shortestTimesTemp[currentE.nodeID].push_back(temporaryStructure);

            }
        }//If the street does exist, check that the time to the current node is less than the previous time stored at that location    
        else if (tempArrayLocation != STREET_DOES_NOT_EXIST && currentE.timeToNode < shortestTimesTemp[currentE.nodeID][tempArrayLocation].shortestTimeToNode) {
            if (currentE.timeToNode < currentBest && currentE.timeToNode < globalBest && currentE.combinedTime < currentBest) {
                shortestTimesTemp[currentE.nodeID][tempArrayLocation].streetID = streetSegments[currentE.prevStreetsegment];
                shortestTimesTemp[currentE.nodeID][tempArrayLocation].previousSegmentID = currentE.prevStreetsegment;
                shortestTimesTemp[currentE.nodeID][tempArrayLocation].previousNodeID = currentE.previousNodeID;
                shortestTimesTemp[currentE.nodeID][tempArrayLocation].shortestTimeToNode = currentE.timeToNode;
                valid = true;
            }
        }

        //If the current node is valid, proceed with adding more elements to the wavefront
        if (valid == true) {
            //if(currentE.prevStreetsegment > 0) draw_segment(currentE.prevStreetsegment);

            //If the current node's id matches the destination id, return true
            if (static_cast<unsigned> (currentE.nodeID) == endID) {
                found = true;
                currentBest = currentE.timeToNode;
                //std::cout << "NEW BEST: " << std::setprecision(20) << currentBest << std::endl;
            }


            streetSegment *currentSegment;
            double tempShortestTime;
            currentSegment = intersections[currentE.nodeID].head;

            //Looping through all connected segments and pushing back any connected nodes
            while (currentSegment != NULL) {
                //Getting streets, if there is no previous street segment, the previous street is the same as the current street
                currentStreet = streetSegments[currentSegment->segmentID];

                if (currentE.prevStreetsegment == NO_EDGE) previousStreet = currentStreet;
                else previousStreet = streetSegments[currentE.prevStreetsegment];

                //Checking what the turn delay should be
                if (currentStreet == previousStreet) currentTurnDelay = 0;
                else currentTurnDelay = turnCost;

                //Setting time to current node
                int nextNode = currentSegment->toNode;
                tempShortestTime = currentE.timeToNode + currentSegment->travelTime + currentTurnDelay;

                //Calculating fastest possible time to reach destination by using max speed limit and geometric distance
                tempTimeToEnd = fast_distance(nextNode, endID) / mapID->get_max_speed_limit();

                //Calculating combined time
                combinedTime = tempShortestTime + tempTimeToEnd;
                // std::cout<<tempTimeToEnd << ", " << combinedTime << std::endl;

                wavefront.push(waveElement(nextNode, currentSegment->segmentID, currentSegment->fromNode, tempShortestTime, tempTimeToEnd, combinedTime));
                //std::cout << wavefront.top().timeToNode << std::endl;
                currentSegment = currentSegment->next;
            }
        }
    }
    //Sets a global best. Used for limiting expansion when finding paths for multiple points of interest
    //Global best should be reset to infinity after every intersection-intersection path
    globalBest = currentBest;
    return found;
}

//Finds the distance between two intersections faster by not computing cosines, less accurate compared to the m1 function

double pathfindingGraph::fast_distance(unsigned intersection1, unsigned intersection2) {
    LatLon point1, point2;
    point1 = getIntersectionPosition(intersection1);
    point2 = getIntersectionPosition(intersection2);

    double x1, y1, x2, y2, d;

    //converting coordinates from lat/lon to (x,y)
    y1 = DEG_TO_RAD * point1.lat();
    y2 = DEG_TO_RAD * point2.lat();
    x1 = DEG_TO_RAD * point1.lon() * mapCoord->get_cos_result();
    x2 = DEG_TO_RAD * point2.lon() * mapCoord->get_cos_result();



    //Square root is slow, I might implement my own faster square root function
    d = EARTH_RADIUS_IN_METERS * sqrt((y2 - y1)*(y2 - y1) + (x2 - x1)*(x2 - x1));
    return d;
}


//Draws a segment, ignoring any cure points. Used for quickly checking the path finding algorithm

void pathfindingGraph::draw_segment(unsigned id) {

    float x1, y1, x2, y2;
    x1 = mapCoord->lon_to_x(getIntersectionPosition(getStreetSegmentInfo(id).from).lon());
    x2 = mapCoord->lon_to_x(getIntersectionPosition(getStreetSegmentInfo(id).to).lon());
    y1 = mapCoord->lat_to_y(getIntersectionPosition(getStreetSegmentInfo(id).from).lat());
    y2 = mapCoord->lat_to_y(getIntersectionPosition(getStreetSegmentInfo(id).to).lat());
    setcolor(RED);
    drawline(x1, y1, x2, y2);
    flushinput();
    //Delay in microseconds, increase for slower algorithm drawing
    usleep(1);

}


//Builds the poi name data to be used in returning all ids related to a POI name

void pathfindingGraph::build_POI_nameData() {
    unsigned POI_num = extraMapData->get_number_of_POI();
    bool found = false;

    for (unsigned n = 0; n < POI_num; n++) {
        for (vector<POI_node>::iterator iter = POI_name_data.begin(); iter != POI_name_data.end(); iter++) {
            //If the name already exists 
            if (iter->name == extraMapData->POI_get_name(n)) {
                found = true;
                iter->POI_ids.push_back(n);
            }
        }
        if (found == false) {
            POI_node temp;
            temp.name = extraMapData->POI_get_name(n);
            temp.POI_ids.push_back(n);
            POI_name_data.push_back(temp);

        }
        found = false;
    }
}

//Returns any POI ids (including duplicates) for a given POI name
//Note: this function does not return a streetDatabase ID, it returns an id to be used in extraOSMdata's POI list

vector<unsigned> pathfindingGraph::get_POI_ids_from_name(string POIname) {
    for (vector<POI_node>::iterator iter = POI_name_data.begin(); iter != POI_name_data.end(); iter++) {
        if (iter->name == POIname) return iter->POI_ids;
    }

    //return nothing if the string isn't found
    vector<unsigned> nothing;
    return nothing;


}

//Returns the closest POI of a given name to an intersection
//If there are multiple POIs with matching names, it takes the 10 (or less) closest POIs and returns them

std::vector<int> pathfindingGraph::find_closest_POI(std::string POIname, const unsigned intersect_id_start) {
    float shortestDistance = std::numeric_limits<float>::infinity();
    float tempDistance, tempHighest;
    unsigned highestID;
    bool valid = true;
    vector<unsigned> points = get_POI_ids_from_name(POIname);
    vector<int> finalPoints;
    finalPoints.resize(POI_RETURN_AMOUNT);
    double distances[POI_RETURN_AMOUNT];

    //Initializing arrays to default values, NO_ID means there is no id in that spot yet, shortest distance set to infinity
    for (unsigned i = 0; i < POI_RETURN_AMOUNT; i++) {
        finalPoints[i] = NO_ID;
        distances[i] = shortestDistance;
    }


    //If the vector is empty, there are no POIs with a matching name
    if (points.size() == 0) {
        std::cout << "Error: no matching POI" << std::endl;
        return finalPoints;
    }

    //Comparing distances to all returned ids to find shortest one
    for (vector<unsigned>::iterator iter = points.begin(); iter != points.end(); iter++) {
        valid = true;
        LatLon temp(extraMapData->POI_get_position(*iter).first, extraMapData->POI_get_position(*iter).second);
        tempDistance = find_distance_between_two_points(temp, getIntersectionPosition(intersect_id_start));
        tempHighest = distances[0];
        highestID = 0;
        //Looping through current distances and checking to see if there are any higher than the current one
        for (unsigned i = 0; i < POI_RETURN_AMOUNT; i++) {
            if (finalPoints[i] == NO_ID) {
                distances[i] = tempDistance;
                finalPoints[i] = *iter;
                valid = false;
                break;
            } else if (distances[i] > tempHighest) {
                tempHighest = distances[i];
                highestID = i;
            }

        }

        if (valid != false && tempDistance < tempHighest) {
            distances[highestID] = tempDistance;
            finalPoints[highestID] = *iter;
        }

    }


    return finalPoints;
}

//Takes a vector of end intersections as the input and finds the closest intersection by travel time. Returned are the vector of street segment ids and the closest end intersection
//Use get_best_travel_time to get the travel time of the path

bool pathfindingGraph::build_travel_time_hash_table(pathUnorderedMap& thePathMap, depotUnorderedMap& theDepotMap, depotUnorderedMap& DepotDelivMap, const std::vector<unsigned>& depots, const std::vector<unsigned>& courierIntersections, const std::vector<unsigned>& dropOffs, const float turnCost) {
    bool foundGlobal = true;
    unsigned numPickupIntersections = courierIntersections.size();
    unsigned numDropOffLocations = dropOffs.size();
    unsigned numberOfDepots = depots.size();
    unsigned totalCourierIntersections = numDropOffLocations + numPickupIntersections;


    //Creating initial list to send to each function
#pragma omp parallel for
    for (unsigned i = 0; i < numPickupIntersections; i++) {
        validIntersections[courierIntersections[i]] = true;
    }

#pragma omp parallel for
    for (unsigned i = 0; i < numDropOffLocations; i++) {
        validIntersections[dropOffs[i]] = true;
    }


    for (vector<unsigned>::const_iterator iter = depots.begin(); iter != depots.end(); iter++) {
        validDepotIntersections[*iter] = true;
    }

    //Loop through all courier Intersections and find distance from each intersection to every other intersection, store in hash table
    //This is done with multithreading, note that any changes made to algorithm MUST NOT WRITE to any global variables
#pragma omp parallel for num_threads(8)
    for (unsigned i = 0; i < numPickupIntersections; i++) {
        bool found = false;
        found = path_found_multiple_intersections(theDepotMap, thePathMap, courierIntersections[i], turnCost, totalCourierIntersections);
        //std::cout << *iter<< std::endl;

        if (found == false) foundGlobal = false;
    }

#pragma omp parallel for num_threads(8)
    for (unsigned i = 0; i < numDropOffLocations; i++) {
        bool found = false;
        found = path_found_multiple_intersections(theDepotMap, thePathMap, dropOffs[i], turnCost, totalCourierIntersections);
        //std::cout << *iter<< std::endl;

        if (found == false) foundGlobal = false;
    }


    //clearing endpoints, reinitializing start points (in case of some identical start+end points)
#pragma omp parallel for
    for (unsigned i = 0; i < numDropOffLocations; i++) {
        validIntersections[dropOffs[i]] = false;
    }

#pragma omp parallel for
    for (unsigned i = 0; i < numPickupIntersections; i++) {
        validIntersections[courierIntersections[i]] = true;
    }

    //Loops through all depots and finds distances to all pickups
#pragma omp parallel for num_threads(8)
    for (unsigned i = 0; i < numberOfDepots; i++) {
        path_found_depot_start(depots[i], turnCost, numPickupIntersections);
    }


    //Clearing pickup intersections and pushing distance final values into hash table
    for (unsigned i = 0; i < numPickupIntersections; i++) {
        validIntersections[courierIntersections[i]] = false;

        DepotDelivMap.emplace(courierIntersections[i], std::make_pair(hashIntersectionDistances[courierIntersections[i]].first, hashIntersectionDistances[courierIntersections[i]].second));
        hashIntersectionDistances[courierIntersections[i]] = std::make_pair(0, std::numeric_limits<double>::infinity());
        // std::cout << courierIntersections[i] << ": " << hashIntersectionDistances[courierIntersections[i]].first << ", " << hashIntersectionDistances[courierIntersections[i]].second << std::endl;
    }



    for (vector<unsigned>::const_iterator iter = depots.begin(); iter != depots.end(); iter++) {
        validDepotIntersections[*iter] = false;
    }

    return foundGlobal; // change me
}

//Finds paths from starting node to all ending nodes and stores it in a hash table (dijkstra's algorithm)

bool pathfindingGraph::path_found_multiple_intersections(depotUnorderedMap& theDepotMap, pathUnorderedMap& thePathMap, const unsigned startID, const double turnCost, const unsigned numEndIntersections) {
    //Current best time (infinite upon initialization)
    std::vector<std::vector < timesToNode>> shortestTimesTemp;

    std::vector<bool> visitedIDs;
    visitedIDs.assign(totalIntersections, false);

    shortestTimesTemp.resize(totalIntersections);

    for (unsigned i = 0; i < totalIntersections; i++) {
        shortestTimesTemp[i].reserve(8);
    }

    double currentTurnDelay;
    int previousStreet, currentStreet;
    bool pathFound = false, valid = false;
    bool depotFound = false;
    unsigned currentVisitedEndIntersections = 0;
    //Creating wave
    std::priority_queue<waveElement2> wavefront;
    waveElement2 tempElement(startID, NO_EDGE, NO_EDGE, 0);
    wavefront.push(tempElement);

    while (wavefront.empty() == false && (currentVisitedEndIntersections != numEndIntersections || depotFound == false)) {
        waveElement2 currentE = wavefront.top();
        wavefront.pop();

        int tempArrayLocation = NO_ID;
        if (currentE.prevStreetsegment != NO_EDGE) {
            //Searching to see if the street exists in the intersections array already
            for (unsigned i = 0; i < shortestTimesTemp[currentE.nodeID].size(); i++) {
                if (shortestTimesTemp[currentE.nodeID][i].streetID == streetSegments[currentE.prevStreetsegment]) {
                    tempArrayLocation = i;
                    break;
                }
            }
        }
        //std::cout << currentE.previousNodeID << std::endl;
        //If the street does not exist yet and the travel time is less than the current best, proceed
        if (tempArrayLocation == NO_ID) {
            valid = true;
            timesToNode temporaryStructure;
            if (currentE.prevStreetsegment != NO_EDGE) temporaryStructure.streetID = streetSegments[currentE.prevStreetsegment];
            else temporaryStructure.streetID = NO_EDGE;
            temporaryStructure.previousSegmentID = currentE.prevStreetsegment;
            temporaryStructure.previousNodeID = currentE.previousNodeID;
            temporaryStructure.shortestTimeToNode = currentE.timeToNode;  
            shortestTimesTemp[currentE.nodeID].push_back(temporaryStructure);
            //If the street does exist, check that the time to the current node is less than the previous time stored at that location    

        } else if (tempArrayLocation != NO_ID && currentE.timeToNode < shortestTimesTemp[currentE.nodeID][tempArrayLocation].shortestTimeToNode) {
            shortestTimesTemp[currentE.nodeID][tempArrayLocation].streetID = streetSegments[currentE.prevStreetsegment];
            shortestTimesTemp[currentE.nodeID][tempArrayLocation].previousSegmentID = currentE.prevStreetsegment;
            shortestTimesTemp[currentE.nodeID][tempArrayLocation].previousNodeID = currentE.previousNodeID;
            shortestTimesTemp[currentE.nodeID][tempArrayLocation].shortestTimeToNode = currentE.timeToNode;
            valid = true;
        } else {
            valid = false;
        }

        if (valid == true) {
            //if(currentE.prevStreetsegment > 0) draw_segment(currentE.prevStreetsegment);

            //Checking if the current node is a valid end intersection and writing to the hash table
            if (currentE.nodeID >= 0) {
                //Vector of bools stores whether or not each intersection is valid for O(1) access at each node)
                if (validIntersections[currentE.nodeID] == true) {
                    if (visitedIDs[currentE.nodeID] == false) {
                        visitedIDs[currentE.nodeID] = true;
                         if(currentE.nodeID != static_cast<int>(startID)) pathFound = true;
                        currentVisitedEndIntersections++;
                        //Any emplace calls are critical and cannot be done through multithreading, they must be done one at a time
#pragma omp critical 
                            thePathMap.emplace(std::make_pair(startID, currentE.nodeID), currentE.timeToNode);

                    }
                }

                //The first depot found (closest) is the only one stored in the depot hash table
                if (depotFound == false && validDepotIntersections[currentE.nodeID] == true) {
                    depotFound = true;
#pragma omp critical 
                    theDepotMap.emplace(startID, std::make_pair(currentE.nodeID, currentE.timeToNode));
                    // std::cout<< "DEPOT: " <<currentE.nodeID << ": " << currentE.timeToNode << std::endl;
                }
            }


            streetSegment *currentSegment;
            double tempShortestTime;
            currentSegment = intersections[currentE.nodeID].head;

            //Looping through all connected segments and pushing back any connected nodes
            while (currentSegment != NULL) {
                //Getting streets, if there is no previous street segment, the previous street is the same as the current street
                currentStreet = streetSegments[currentSegment->segmentID];

                if (currentE.prevStreetsegment == NO_EDGE) previousStreet = currentStreet;
                else previousStreet = streetSegments[currentE.prevStreetsegment];

                //Checking what the turn delay should be
                if (currentStreet == previousStreet) currentTurnDelay = 0;
                else currentTurnDelay = turnCost;

                int nextNode = currentSegment->toNode;
                tempShortestTime = currentE.timeToNode + currentSegment->travelTime + currentTurnDelay;

                wavefront.push(waveElement2(nextNode, currentSegment->segmentID, currentSegment->fromNode, tempShortestTime));

                currentSegment = currentSegment->next;
            }
        }
    }

    //If at least one path has not been found, the current node (pickup/drop-off) is disconnected and no path will be valid  
    //Similarly if there are no connected depots found, it means that all the depots are probably disconnected 
    if (pathFound == false || depotFound == false) {
        return false;
    }

    //Setting remaining distances (if any) as infinity
    if (currentVisitedEndIntersections != numEndIntersections) {
        for (unsigned i = 0; (i < totalIntersections) && (currentVisitedEndIntersections != numEndIntersections); i++) {
            if (validIntersections[i] != visitedIDs[i]) {
                currentVisitedEndIntersections++;
#pragma omp critical 
                thePathMap.emplace(std::make_pair(startID, i), std::numeric_limits<double>::infinity());
            }
        }
    }

    return true;
}


//Similar to algorithm above but stores results in a temporary vector, results are replaced if a shorter travel time is found in a future call

bool pathfindingGraph::path_found_depot_start(const unsigned startID, const double turnCost,  const unsigned numEndIntersections) {
    //Current best time (infinite upon initialization)
    std::vector<std::vector < timesToNode>> shortestTimesTemp;

    std::vector<bool> visitedIDs;
    visitedIDs.assign(totalIntersections, false);

    shortestTimesTemp.resize(totalIntersections);

    for (unsigned i = 0; i < totalIntersections; i++) {
        shortestTimesTemp[i].reserve(8);
    }

    double currentTurnDelay;
    int previousStreet, currentStreet;
    bool pathFound = false, valid = false;
    unsigned currentVisitedEndIntersections = 0;
    //Creating wave
    std::priority_queue<waveElement2> wavefront;
    waveElement2 tempElement(startID, NO_EDGE, NO_EDGE, 0);
    wavefront.push(tempElement);

    while (wavefront.empty() == false && currentVisitedEndIntersections != numEndIntersections) {
        waveElement2 currentE = wavefront.top();
        wavefront.pop();

        int tempArrayLocation = NO_ID;
        if (currentE.prevStreetsegment != NO_EDGE) {
            //Searching to see if the street exists in the intersections array already
            for (unsigned i = 0; i < shortestTimesTemp[currentE.nodeID].size(); i++) {
                if (shortestTimesTemp[currentE.nodeID][i].streetID == streetSegments[currentE.prevStreetsegment]) {
                    tempArrayLocation = i;
                    break;
                }
            }
        }
        //std::cout << currentE.previousNodeID << std::endl;
        //If the street does not exist yet and the travel time is less than the current best, proceed
        if (tempArrayLocation == NO_ID) {
            valid = true;
            timesToNode temporaryStructure;
            if (currentE.prevStreetsegment != NO_EDGE) temporaryStructure.streetID = streetSegments[currentE.prevStreetsegment];
            else temporaryStructure.streetID = NO_EDGE;
            temporaryStructure.previousSegmentID = currentE.prevStreetsegment;
            temporaryStructure.previousNodeID = currentE.previousNodeID;
            temporaryStructure.shortestTimeToNode = currentE.timeToNode;           
            shortestTimesTemp[currentE.nodeID].push_back(temporaryStructure);           
            //If the street does exist, check that the time to the current node is less than the previous time stored at that location    

        } else if (tempArrayLocation != NO_ID && currentE.timeToNode < shortestTimesTemp[currentE.nodeID][tempArrayLocation].shortestTimeToNode) {
            shortestTimesTemp[currentE.nodeID][tempArrayLocation].streetID = streetSegments[currentE.prevStreetsegment];
            shortestTimesTemp[currentE.nodeID][tempArrayLocation].previousSegmentID = currentE.prevStreetsegment;
            shortestTimesTemp[currentE.nodeID][tempArrayLocation].previousNodeID = currentE.previousNodeID;
            shortestTimesTemp[currentE.nodeID][tempArrayLocation].shortestTimeToNode = currentE.timeToNode;
            valid = true;
        } else {
            valid = false;
        }

        if (valid == true) {
            //if(currentE.prevStreetsegment > 0) draw_segment(currentE.prevStreetsegment);

            //Checking if the current node is a valid end intersection and writing to the hash table
            if (currentE.nodeID >= 0) {
                //Vector of bools stores whether or not each intersection is valid for O(1) access at each node)
                if (validIntersections[currentE.nodeID] == true) {
                    if (visitedIDs[currentE.nodeID] == false) {
                        visitedIDs[currentE.nodeID] = true;
                        if(currentE.nodeID != static_cast<int>(startID)) pathFound = true;
                        currentVisitedEndIntersections++;
                        //Replacing shortest distances in vector cannot be done in multi-threading, must be critical
#pragma omp critical 
                        if (currentE.timeToNode < hashIntersectionDistances[currentE.nodeID].second) {
                            hashIntersectionDistances[currentE.nodeID] = std::make_pair(startID, currentE.timeToNode);            
                        }
                    }
                }
            }


            streetSegment *currentSegment;
            double tempShortestTime;
            currentSegment = intersections[currentE.nodeID].head;

            //Looping through all connected segments and pushing back any connected nodes
            while (currentSegment != NULL) {
                //Getting streets, if there is no previous street segment, the previous street is the same as the current street
                currentStreet = streetSegments[currentSegment->segmentID];

                if (currentE.prevStreetsegment == NO_EDGE) previousStreet = currentStreet;
                else previousStreet = streetSegments[currentE.prevStreetsegment];

                //Checking what the turn delay should be
                if (currentStreet == previousStreet) currentTurnDelay = 0;
                else currentTurnDelay = turnCost;

                int nextNode = currentSegment->toNode;
                tempShortestTime = currentE.timeToNode + currentSegment->travelTime + currentTurnDelay;

                wavefront.push(waveElement2(nextNode, currentSegment->segmentID, currentSegment->fromNode, tempShortestTime));

                currentSegment = currentSegment->next;
            }
        }
    }

    //If at least one path has not been found, the current node (pickup/drop-off) is disconnected and no path will be valid  
    if (pathFound == false) {
        return false;
    }

    return true;
}

//Sets the global best for use in pathfinding algorithm, should be set back to infinity when done using reset best

void pathfindingGraph::set_global_best(double newBest) {
    globalBest = newBest;
}

//Resets the global best back to infinity

void pathfindingGraph::reset_global_best() {
    globalBest = std::numeric_limits<double>::infinity();
}

//Returns global best travel time, make sure to reset global best travel time after using

double pathfindingGraph::get_best_travel_time() {
    return globalBest;
}

//Removes a linked list at a given id

void pathfindingGraph::removeLinkedList(unsigned id) {
    streetSegment *nextNode, *currentNode;
    currentNode = intersections[id].head;
    while (currentNode != NULL) {
        nextNode = currentNode->next;
        delete currentNode;
        currentNode = nextNode;
    }


}

//Removes all linked lists at given nodes

pathfindingGraph::~pathfindingGraph() {
    for (unsigned i = 0; i < intersections.size(); i++) {
        if (intersections[i].head != NULL) removeLinkedList(i);
    }
    intersections.clear();
}
