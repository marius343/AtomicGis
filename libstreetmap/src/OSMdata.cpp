/*  Author: Marius Stan
 *  Class OSMdata stores data queried directly from OSM database
 */

#include "OSMdata.h"

//Constructor, builds all the vectors in the class

OSMdata::OSMdata() {
    drawSubway = false, drawAttractions = false, drawRailway = true;

    conversionVector.resize(getNumberOfNodes());
    wayConversionVector.resize(getNumberOfWays());


    std::string key, value;
    //Looping through all nodes to find all subway stations and attractions, storing them in vectors
    for (unsigned j = 0; j < getNumberOfNodes(); j++) {

        const OSMNode* e = getNodeByIndex(j);

        //Creating temporary structure for OSMid
        OSM_id_pair tempPair;
        tempPair.id = j;
        tempPair.OSMid = e->id().operator uint64_t();
        conversionVector[j] = tempPair;

        bool validStation = false, validAttraction = false;
        std::string tempName = "NULL";

        for (unsigned i = 0; i < getTagCount(e); ++i) {
            std::tie(key, value) = getTagPair(e, i);


            if (key == "railway" && value == "station") {
                validStation = true;
            } else if (key == "name") {
                tempName = value;
            } else if (key == "tourism" && (value == "attraction" || value == "aquarium" || value == "theme_park" || value == "zoo" || value == "viewpoint")) {
                validAttraction = true;
            }

        }
        if (validStation == true && tempName != "NULL") {
            OSM_node_data temp;
            temp.name = tempName;
            temp.x = mapCoord->lon_to_x(e->coords().lon());
            temp.y = mapCoord->lat_to_y(e->coords().lat());
            subwayStations.push_back(temp);
        }
        if (validAttraction == true && tempName != "NULL") {
            OSM_node_data temp;
            temp.name = tempName;
            temp.x = mapCoord->lon_to_x(e->coords().lon());
            temp.y = mapCoord->lat_to_y(e->coords().lat());
            attractions.push_back(temp);
        }

    }

    //Sorting Id array, this may not actually be needed if all the ids are already sorted
    std::sort(conversionVector.begin(), conversionVector.end(), sortID);

    //Searches all ways and build approprate data structures
    for (unsigned j = 0; j < getNumberOfWays(); j++) {
        const OSMWay* e = getWayByIndex(j);
        bool validSubwayLine = false, validRailway = false;
        std::string tempName = "NULL";

        //Creating temporary structure for OSMid
        OSM_id_pair tempPair;
        tempPair.id = j;
        tempPair.OSMid = e->id().operator uint64_t();
        wayConversionVector[j] = tempPair;

        for (unsigned i = 0; i < getTagCount(e); ++i) {
            std::tie(key, value) = getTagPair(e, i);

            if (key == "railway" && value == "subway") {
                validSubwayLine = true;
            } else if (key == "name") {
                tempName = value;
            } else if (key == "railway" && (value == "rail")) {
                validRailway = true;
            }
        }

        //Building subway lines
        if (validSubwayLine == true && tempName != "NULL") {
            OSM_way_data tempWay;
            tempWay.name = tempName;

            //Looping through lists of node references to get subway lines

            for (unsigned i = 0; i < e->ndrefs().size(); i++) {
                unsigned id = OSMid_to_id_NODE(0, get_conversionVector_size(), e->ndrefs().at(i).operator uint64_t());
                const OSMNode* e2 = getNodeByIndex(id);
                OSM_node_data tempNode;
                tempNode.name = tempName; //This could be replaced with the actual node name later if needed, for now its the same as the subway
                tempNode.x = mapCoord->lon_to_x(e2->coords().lon());
                tempNode.y = mapCoord->lat_to_y(e2->coords().lat());
                tempWay.wayNodes.push_back(tempNode);
            }

            subwayLines.push_back(tempWay);

            //Building railway lines
        } else if (validRailway == true) {
            OSM_way_data tempWay;
            tempWay.name = tempName;

            //Looping through lists of node references to get railway lines

            for (unsigned i = 0; i < e->ndrefs().size(); i++) {
                unsigned id = OSMid_to_id_NODE(0, get_conversionVector_size(), e->ndrefs().at(i).operator uint64_t());
                const OSMNode* e2 = getNodeByIndex(id);
                OSM_node_data tempNode;
                tempNode.name = tempName; //This could be replaced with the actual node name later if needed, for now its the same as the subway
                tempNode.x = mapCoord->lon_to_x(e2->coords().lon());
                tempNode.y = mapCoord->lat_to_y(e2->coords().lat());
                tempWay.wayNodes.push_back(tempNode);
            }

            railwayLines.push_back(tempWay);
        }
    }

    //Sorting Id array, this may not actually be needed if all the ids are already sorted
    std::sort(wayConversionVector.begin(), wayConversionVector.end(), sortID);

    build_POI_vector();

}

//Converts from OSMid to an id that can be accessed directly from the layer 1 API
//If the OSMid is not found an error message will be printed out and 0 will be returned
//To use, start with OSMid_to_id_NODE(0, get_conversionVector_size(), your_id_here);

unsigned OSMdata::OSMid_to_id_NODE(unsigned left, unsigned right, unsigned OSMid) {
    //Base case 1 & 2
    if (left == right) {
        if (conversionVector[right].OSMid == OSMid) return conversionVector[right].id;
        else {
            std::cerr << "ERROR Node OSMid not found!" << std::endl;
            return 0;
        }

    } else if (right == left + 1) {
        if (conversionVector[right].OSMid == OSMid) return conversionVector[right].id;
        else if (conversionVector[left].OSMid == OSMid) return conversionVector[left].id;
        else {
            std::cerr << "ERROR Node OSMid not found!" << std::endl;
            return 0;
        }
        //If not base case, search each side depending on whether id is greater than or less than the center
    } else {
        unsigned center = (right - left) / 2 + left;
        if (conversionVector[center].OSMid == OSMid) return conversionVector[center].id;

        else if (OSMid < conversionVector[center].OSMid) return OSMid_to_id_NODE(left, center, OSMid);
        else return OSMid_to_id_NODE(center + 1, right, OSMid);
    }

}

//Similar to function above except for ways instead of nodes

unsigned OSMdata::OSMid_to_id_WAY(unsigned left, unsigned right, unsigned OSMid) {
    //Base case 1 & 2
    if (left == right) {
        if (wayConversionVector[right].OSMid == OSMid) return wayConversionVector[right].id;
        else {
            std::cerr << "ERROR Way OSMid not found!" << std::endl;
            return 0;
        }

    } else if (right == left + 1) {
        if (wayConversionVector[right].OSMid == OSMid) return wayConversionVector[right].id;
        else if (wayConversionVector[left].OSMid == OSMid) return wayConversionVector[left].id;
        else {
            std::cerr << "ERROR Way OSMid not found!" << std::endl;
            return 0;
        }
        //If not base case, search each side depending on whether id is greater than or less than the center
    } else {
        unsigned center = (right - left) / 2 + left;
        if (wayConversionVector[center].OSMid == OSMid) return wayConversionVector[center].id;

        else if (OSMid < wayConversionVector[center].OSMid) return OSMid_to_id_WAY(left, center, OSMid);
        else return OSMid_to_id_WAY(center + 1, right, OSMid);
    }
}

unsigned OSMdata::get_conversionVector_size() {
    return conversionVector.size();
}

unsigned OSMdata::get_way_conversionVector_size() {
    return wayConversionVector.size();
}


//Temporary function, just prints out Node tags

void OSMdata::getNodeTags(unsigned NodeID) {

    std::string key, value;

    const OSMNode* e = getNodeByIndex(NodeID);

    std::cout << getTagCount(e) << std::endl;
    for (unsigned i = 0; i < getTagCount(e); ++i) {
        std::tie(key, value) = getTagPair(e, i);
        std::cout << key << ": " << value << std::endl;
    }

}

//Returns the street segment classification, return NULL if there is no street segment classification
//This function is at best O(logn) and at worst O(n) so don't use it to often (ie only use it when loading the map)
//For possible return values see: http://wiki.openstreetmap.org/wiki/Key:highway#Roads

std::string OSMdata::getStreetSegmentType(OSMID &id) {

    std::string key, value, classification = "NULL";

    //Conversion from OSMid to id
    unsigned idTemp = OSMid_to_id_WAY(0, get_way_conversionVector_size(), id.operator uint64_t());
    const OSMWay* theWay = getWayByIndex(idTemp);


    for (unsigned i = 0; i < getTagCount(theWay); ++i) {
        std::tie(key, value) = getTagPair(theWay, i);
        if (key == "highway") return value;
    }


    return classification;
}



//Helper function for std::sort

bool sortID(OSM_id_pair &a, OSM_id_pair &b) {
    return (a.OSMid < b.OSMid);
}

//Builds the POI vector by searching OSM POIs and street database POIs

void OSMdata::build_POI_vector() {

    unsigned numPOI = getNumberOfPointsOfInterest();
    unsigned numStation = subwayStations.size();
    unsigned numAttractions = attractions.size();
    unsigned length = numPOI + numStation + numAttractions;
    pointsOfInterest.resize(length);
    //Putting in POIs from the street database
    for (unsigned i = 0; i < numPOI; i++) {
        pointsOfInterest[i].name = getPointOfInterestName(i);
        pointsOfInterest[i].type = getPointOfInterestType(i);
        pointsOfInterest[i].lat = getPointOfInterestPosition(i).lat();
        pointsOfInterest[i].lon = getPointOfInterestPosition(i).lon();

    }

    //Putting in subway stations
    for (unsigned i = 0; i < numStation; i++) {
        pointsOfInterest[i + numPOI].name = subwayStations[i].name;
        pointsOfInterest[i + numPOI].type = "subway";
        pointsOfInterest[i + numPOI].lon = mapCoord->x_to_lon(subwayStations[i].x);
        pointsOfInterest[i + numPOI].lat = mapCoord->y_to_lat(subwayStations[i].y);
    }

    //Putting in attractions
    for (unsigned i = 0; i < numAttractions; i++) {
        pointsOfInterest[i + numPOI + numStation].name = attractions[i].name;
        pointsOfInterest[i + numPOI + numStation].type = "attraction";
        pointsOfInterest[i + numPOI + numStation].lon = mapCoord->x_to_lon(attractions[i].x);
        pointsOfInterest[i + numPOI + numStation].lat = mapCoord->y_to_lat(attractions[i].y);
    }

}

//Functions return data from POI array

std::string OSMdata::POI_get_name(unsigned id) {
    return pointsOfInterest[id].name;
}

std::string OSMdata::POI_get_type(unsigned id) {
    return pointsOfInterest[id].type;
}

//Returns a lat, lon pair

std::pair<float, float> OSMdata::POI_get_position(unsigned id) {
    return std::make_pair(pointsOfInterest[id].lat, pointsOfInterest[id].lon);
}

unsigned OSMdata::get_number_of_POI() {

    return pointsOfInterest.size();
}


//The rest of these functions are pretty self explanatory

OSM_way_data OSMdata::get_railway(unsigned index) {
    return railwayLines[index];
}

unsigned OSMdata::get_number_of_railways() {
    return railwayLines.size();
}

OSM_way_data OSMdata::get_subway_line(unsigned index) {
    return subwayLines[index];
}

unsigned OSMdata::get_number_of_lines() {
    return subwayLines.size();
}

unsigned OSMdata::getNumberOfStations() {
    return subwayStations.size();
}

unsigned OSMdata::getNumberOfAttractions() {
    return attractions.size();
}

OSM_node_data OSMdata::getSubwayStation(unsigned index) {

    return subwayStations[index];
}

OSM_node_data OSMdata::getAttraction(unsigned index) {

    return attractions[index];
}

void OSMdata::set_draw_subway(bool valid) {
    drawSubway = valid;
}

void OSMdata::set_draw_attractions(bool valid) {
    drawAttractions = valid;
}

void OSMdata::set_draw_railway(bool valid) {
    drawRailway = valid;
}

bool OSMdata::draw_subway() {
    return drawSubway;
}

bool OSMdata::draw_attractions() {
    return drawAttractions;
}

bool OSMdata::draw_railway() {
    return drawRailway;
}

OSMdata::~OSMdata() {
    subwayStations.clear();
    pointsOfInterest.clear();
}
