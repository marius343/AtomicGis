/* Author: Marius Stan
 * K-D tree class includes functions that build and access the tree
 */

#include <queue>
#include <bits/algorithmfwd.h>

#include "graphics.h"
#include "StreetsDatabaseAPI.h"
#include "kdTree.h"
#include "Coord.h"
#include "Global.h"
#include "Global2.h"
#include "Global3.h"
#include "visual.h"

#define number_Of_Categories2 26
//Arrays for point of interest categories and symbols IF THIS ARRAY IS CHANGED IT NEEDS TO BE CHANGED IN VISUAL.CPP AS WELL
std::string POIcategories2[number_Of_Categories2] = {"EMS", "Police", "Fire", "Public", "Schools", "Community", "Library", "Cafe",
    "Transportation", "Subway", "Train", "Shop", "Church", "Fuel", "Cinema", "Money", "Bar", "Doctor", "Food",
    "Post", "Rental", "Phone", "Toilets", "Attractions", "Parking", "Other"};

kdTree::kdTree() {
    build_POI_kdTree();
    build_Intersection_kdTree();
    depotRoot = NULL;
}

//Returns the distance between two latitudes and longitudes

float kdTree::findDistance(float lat1, float lon1, float lat2, float lon2) {
    double x1, y1, x2, y2;
    double latavg;
    double d = 0;

    //converting coordinates from lat/lon to (x,y)
    y1 = DEG_TO_RAD * lat1;
    y2 = DEG_TO_RAD * lat2;
    latavg = DEG_TO_RAD * (lat1 + lat2) / 2;

    x1 = DEG_TO_RAD * lon1 * cos(latavg);
    x2 = DEG_TO_RAD * lon2 * cos(latavg);

    //compute distance using formula given
    d = EARTH_RADIUS_IN_METERS * sqrt((y2 - y1)*(y2 - y1) + (x2 - x1)*(x2 - x1));
    return d;
}

//Returns the distance between two points in world or screen coordinates 

float kdTree::findDistance_xy(float x1, float y1, float x2, float y2) {
    return sqrt((y2 - y1)*(y2 - y1) + (x2 - x1)*(x2 - x1));
}


//Helper functions for std::sort

bool compareLat(const coordinate &a, const coordinate &b) {
    return (a.lat < b.lat);
}

bool compareLon(const coordinate &a, const coordinate &b) {
    return (a.lon < b.lon);
}

//Builds the POI k-d tree by first creating a temporary array, then using the function createKDtree
//CHANGE ME

void kdTree::build_POI_kdTree() {
    unsigned length = extraMapData->get_number_of_POI();
    tempArray = new coordinate[length];


    //Creating a temporary array to use in k-d tree construction, also creates the permanent POI array (wont be sorted by distance)
    for (unsigned i = 0; i < length; i++) {
        tempArray[i].id = i;
        tempArray[i].lat = extraMapData->POI_get_position(i).first;
        tempArray[i].lon = extraMapData->POI_get_position(i).second;
        //std::cout << extraMapData->POI_get_name(i)<< std::endl;
    }

    POIroot = createKDtree(0, length - 1, true);
    delete []tempArray;

}

//Builds the intersection k-d tree by first creating a temporary array, then using the function createKDtree

void kdTree::build_Intersection_kdTree() {
    unsigned length = getNumberOfIntersections();
    tempArray = new coordinate[length];

    //Creating a temporary array to use in k-d tree construction, also creates the permanent POI array (wont be sorted by distance)
    for (unsigned i = 0; i < length; i++) {
        tempArray[i].id = i;
        tempArray[i].lat = getIntersectionPosition(i).lat();
        tempArray[i].lon = getIntersectionPosition(i).lon();
    }

    intersectionRoot = createKDtree(0, length - 1, true);
    delete []tempArray;

}

//Takes in a vector of depots and builds the k-d tree

void kdTree::build_Depot_KD_Tree(std::vector<coordinate> depots) {
    deleteDepotKDtree();
    unsigned length = depots.size();
    tempArray = new coordinate[length];

    for (unsigned i = 0; i < length; i++) {
        tempArray[i].id = depots[i].id;
        tempArray[i].lat = depots[i].lat;
        tempArray[i].lon = depots[i].lon;
    }

    depotRoot = createKDtree(0, length - 1, true);
    delete []tempArray;
}

//Creates a K-d tree using the temporary coordinate struct array, can be used for any feature, as long as its stored in the appropriate array
//For an in depth look at the k-d tree data structure search for k-d trees on Wikipedia

treeNode* kdTree::createKDtree(unsigned left, unsigned right, bool cmpLat) {
    //First statement covers any sized array partition aside from a 2 and 1 element partition
    if (left != right && right != left + 1) {
        bool new_cmpLat;
        //Following statements sorts the list by latitude or longitude depending on cmpLat flag, 
        if (cmpLat == true) {
            std::sort(tempArray + left, tempArray + right + 1, compareLat);
            new_cmpLat = false;
        } else {
            std::sort(tempArray + left, tempArray + right + 1, compareLon);
            new_cmpLat = true;
        }

        //Finding the median by locating the middle of the current section
        unsigned median = (right - left) / 2 + left;
        unsigned medianID = tempArray[median].id;
        //Creating new Node with median
        treeNode* newNode = new treeNode(medianID, tempArray[median].lat, tempArray[median].lon);
        //Recursively finding left and right subtrees
        newNode->set_left(this->createKDtree(left, median - 1, new_cmpLat));
        newNode->set_right(this->createKDtree(median + 1, right, new_cmpLat));
        return newNode;

        //Case for two elements left in array
    } else if (right == left + 1) {
        //Creating a node with the first element's id
        treeNode* newNode = new treeNode(tempArray[left].id, tempArray[left].lat, tempArray[left].lon);
        //Placing the second Node depending on its value relative to the first, also takes into account if lat or lon is being compared
        if (cmpLat == true) {
            if (tempArray[right].lat < tempArray[left].lat) {
                newNode->set_left(this->createKDtree(right, right, false));
                newNode->set_right(NULL);
            } else {
                newNode->set_left(NULL);
                newNode->set_right(this->createKDtree(right, right, false));
            }
        } else if (cmpLat == false) {
            if (tempArray[right].lon < tempArray[left].lon) {
                newNode->set_left(this->createKDtree(right, right, false));
                newNode->set_right(NULL);
            } else {
                newNode->set_left(NULL);
                newNode->set_right(this->createKDtree(right, right, false));
            }
        }
        return newNode;

        //Case for when the current section of the array has just 1 element
    } else {
        treeNode* newNode = new treeNode(tempArray[left].id, tempArray[left].lat, tempArray[right].lon);
        newNode->set_left(NULL);
        newNode->set_right(NULL);
        return newNode;
    }
}

//Recursive find closest function, can be used for any given K-d tree
//This function goes down the k-d tree, comparing lat and lon along the way to find the closest point

void kdTree::findClosest(treeNode* newRoot, bool cmpLat, unsigned* closestID, float* distance, float lat, float lon) {
    float tmpDistance;

    //Base case, if both branches are null, that node is the new best case
    if (newRoot->get_left() == NULL && newRoot->get_right() == NULL) {
        tmpDistance = findDistance(lat, lon, newRoot->get_lat(), newRoot->get_lon());
        if (tmpDistance < *distance) {
            *distance = tmpDistance;
            *closestID = newRoot->get_id();
        }
    } else {
        bool checkRightSubtree = true; //Bool for determining which subtree to potentially check on the way back up
        //Checks if given coordinate is on left/right side or top/bottom side of grid based on if cmpLat is true or false
        if (cmpLat == true && lat < newRoot->get_lat() && newRoot->get_left() != NULL) {
            findClosest(newRoot->get_left(), false, closestID, distance, lat, lon);
        } else if (cmpLat == true && lat >= newRoot->get_lat() && newRoot->get_right() != NULL) {
            findClosest(newRoot->get_right(), false, closestID, distance, lat, lon);
            checkRightSubtree = false;

        } else if (cmpLat == false && lon < newRoot->get_lon() && newRoot->get_left() != NULL) {
            findClosest(newRoot->get_left(), true, closestID, distance, lat, lon);
        } else if (cmpLat == false && lon >= newRoot->get_lon() && newRoot->get_right() != NULL) {
            findClosest(newRoot->get_right(), true, closestID, distance, lat, lon);
            checkRightSubtree = false;

        } else if (newRoot->get_left() == NULL) {
            checkRightSubtree = true;
        } else if (newRoot->get_right() == NULL) {
            checkRightSubtree = false;
        }

        //Checks if current Node is closer

        tmpDistance = findDistance(lat, lon, newRoot->get_lat(), newRoot->get_lon());

        if (tmpDistance < *distance) {
            *distance = tmpDistance;
            *closestID = newRoot->get_id();
        }

        /*Checks to see if any of the subtrees can be closer 
         *The idea behind this is that if a circle with radius r=current_closest goes into another region of the "grid"
         *There may be a point in that section (subtree) that is closer than the current one
         *This is then by first finding the point on the line containing the current node that is closest to the user inputed position
         *Then the distance is found between this point and the user's position, this distance is then compared to the current shortest
         *If the distance found is smaller, that means the circles radius goes into another section and the subtree is explored
         */
        if (cmpLat == true) tmpDistance = findDistance(lat, lon, newRoot->get_lat(), lon);
        else if (cmpLat == false) tmpDistance = findDistance(lat, lon, lat, newRoot->get_lon());

        if (tmpDistance < *distance && checkRightSubtree == true && newRoot->get_right() != NULL) {
            findClosest(newRoot->get_right(), !cmpLat, closestID, distance, lat, lon);
        } else if (tmpDistance < *distance && checkRightSubtree == false && newRoot->get_left() != NULL) {
            findClosest(newRoot->get_left(), !cmpLat, closestID, distance, lat, lon);
        }
    }
}

//Uses priority queue to find multiple closest depots

void kdTree::findClosestMultiple(treeNode* newRoot, bool cmpLat, float lat, float lon) {
    float tmpDistance;

    //Base case, if both branches are null, that node is the new best case
    if (newRoot->get_left() == NULL && newRoot->get_right() == NULL) {
        tmpDistance = findDistance(lat, lon, newRoot->get_lat(), newRoot->get_lon());
        if (tmpDistance < closestDistances.top().distance) {
            closestDistances.pop();
            closestDistances.push(topClosestElements(newRoot->get_id(), tmpDistance));
        }
    } else {
        bool checkRightSubtree = true; //Bool for determining which subtree to potentially check on the way back up
        //Checks if given coordinate is on left/right side or top/bottom side of grid based on if cmpLat is true or false
        if (cmpLat == true && lat < newRoot->get_lat() && newRoot->get_left() != NULL) {
            findClosestMultiple(newRoot->get_left(), false, lat, lon);
        } else if (cmpLat == true && lat >= newRoot->get_lat() && newRoot->get_right() != NULL) {
            findClosestMultiple(newRoot->get_right(), false, lat, lon);
            checkRightSubtree = false;

        } else if (cmpLat == false && lon < newRoot->get_lon() && newRoot->get_left() != NULL) {
            findClosestMultiple(newRoot->get_left(), true, lat, lon);
        } else if (cmpLat == false && lon >= newRoot->get_lon() && newRoot->get_right() != NULL) {
            findClosestMultiple(newRoot->get_right(), true, lat, lon);
            checkRightSubtree = false;

        } else if (newRoot->get_left() == NULL) {
            checkRightSubtree = true;
        } else if (newRoot->get_right() == NULL) {
            checkRightSubtree = false;
        }

        //Checks if current Node is closer

        tmpDistance = findDistance(lat, lon, newRoot->get_lat(), newRoot->get_lon());

        if (tmpDistance < closestDistances.top().distance) {
            closestDistances.pop();
            closestDistances.push(topClosestElements(newRoot->get_id(), tmpDistance));
        }

        /*Checks to see if any of the subtrees can be closer 
         *The idea behind this is that if a circle with radius r=current_closest goes into another region of the "grid"
         *There may be a point in that section (subtree) that is closer than the current one
         *This is then by first finding the point on the line containing the current node that is closest to the user inputed position
         *Then the distance is found between this point and the user's position, this distance is then compared to the current shortest
         *If the distance found is smaller, that means the circles radius goes into another section and the subtree is explored
         */
        if (cmpLat == true) tmpDistance = findDistance(lat, lon, newRoot->get_lat(), lon);
        else if (cmpLat == false) tmpDistance = findDistance(lat, lon, lat, newRoot->get_lon());

        if (tmpDistance < closestDistances.top().distance && checkRightSubtree == true && newRoot->get_right() != NULL) {
            findClosestMultiple(newRoot->get_right(), !cmpLat, lat, lon);
        } else if (tmpDistance < closestDistances.top().distance && checkRightSubtree == false && newRoot->get_left() != NULL) {
            findClosestMultiple(newRoot->get_left(), !cmpLat, lat, lon);
        }
    }
}

//Returns an unsigned vector of the closest depots

std::vector<unsigned> kdTree::find_closest_depots(float lat, float lon, unsigned number_of_return_elements) {
    vector<unsigned> closestPoints;
    unsigned numElements = number_of_return_elements;

    if (number_of_return_elements == 0) {
        numElements = 1;
    }

    if (depotRoot != NULL) {
        //Creating priority queue of proper size, filling it up with a bunch of infinities
        for (unsigned i = 0; i < numElements; i++) {
            closestDistances.push(topClosestElements(-1, std::numeric_limits<double>::infinity()));
        }

        findClosestMultiple(depotRoot, true, lat, lon);

        //Loops through priority queue (sorted by largest first) and adds nodes to vector
        while (closestDistances.empty() != true) {
            if (closestDistances.top().nodeID != -1) {
                closestPoints.push_back(static_cast<unsigned> (closestDistances.top().nodeID));
            }
            std::cout << closestDistances.top().nodeID << ": " << closestDistances.top().distance << std::endl;
            closestDistances.pop();
        }

        //Reversing so order is from closest to furthest 
        std::reverse(closestPoints.begin(), closestPoints.end());
    } else {
        std::cout << "ERROR: Depot k-d tree is empty, cannot find closest point(s)" << std::endl;
    }


    return closestPoints;
}


//Returns a pair with the closest intersection/POI id 

std::pair<char, unsigned> kdTree::findClosestElement(float lat, float lon) {

    float temp_POI_Distance = std::numeric_limits<float>::infinity();
    float temp_Intersection_Distance = std::numeric_limits<float>::infinity();
    float x1, y1, x2, y2;
    unsigned closest_POI_id = 0;
    unsigned closest_Intersection_ID = 0;

    findClosest(POIroot, true, &closest_POI_id, &temp_POI_Distance, lat, lon);
    findClosest(intersectionRoot, true, &closest_Intersection_ID, &temp_Intersection_Distance, lat, lon);

    if (temp_POI_Distance < temp_Intersection_Distance) {

        //Getting coordinates of closest point and converting to x y world coordinates
        x2 = mapCoord->lon_to_x(extraMapData->POI_get_position(closest_POI_id).second);
        y2 = mapCoord->lat_to_y(extraMapData->POI_get_position(closest_POI_id).first);
        //Converting to screen coordinates
        x1 = xworld_to_scrn(mapCoord->lon_to_x(lon));
        y1 = yworld_to_scrn(mapCoord->lat_to_y(lat));
        x2 = xworld_to_scrn(x2);
        y2 = yworld_to_scrn(y2);


        if (findDistance_xy(x1, y1, x2, y2) <= 20 && mapGraphics->is_POI_on_screen(closest_POI_id) == true) { //Click should be less than 20 pixels away
            return std::make_pair('p', closest_POI_id);

        }
    } else {
        //Getting coordinates of closest point and converting to x y world coordinates
        x2 = mapCoord->lon_to_x(getIntersectionPosition(closest_Intersection_ID).lon());
        y2 = mapCoord->lat_to_y(getIntersectionPosition(closest_Intersection_ID).lat());
        //Converting to screen coordinates
        x1 = xworld_to_scrn(mapCoord->lon_to_x(lon));
        y1 = yworld_to_scrn(mapCoord->lat_to_y(lat));
        x2 = xworld_to_scrn(x2);
        y2 = yworld_to_scrn(y2);

        if (findDistance_xy(x1, y1, x2, y2) <= 20) //Click should be less than 20 pixels away
            return std::make_pair('i', closest_Intersection_ID);
    }

    //Returns this if there is no close intersection/POI (i.e. its too far from where the user clicks)
    return std::make_pair('n', 0);
}

treeNode* kdTree::getPOIroot() {
    return POIroot;
}

treeNode* kdTree::getIntersectionRoot() {
    return intersectionRoot;
}

void kdTree::deleteDepotKDtree() {
    if (depotRoot != NULL) {
        delete depotRoot;
        depotRoot = NULL;
    }
}

kdTree::~kdTree() {
    delete POIroot;
    delete intersectionRoot;
    if (depotRoot != NULL) delete depotRoot;
}