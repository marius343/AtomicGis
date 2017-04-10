/*  Author: Marius Stan
 *  Class OSMdata stores data queried directly from OSM database
 */

#ifndef OSMDATA_H
#define OSMDATA_H

#include "Global.h"
#include "OSMDatabaseAPI.h"

using namespace std;

//Node structure for use in vectors

struct OSM_node_data {
    float x;
    float y;
    std::string name;
};

struct OSM_way_data {
    std::string name;
    std::vector<OSM_node_data> wayNodes;
};

struct OSM_id_pair {
    unsigned id;
    unsigned OSMid;
};

struct OSM_POI {
    std::string name;
    std::string type;
    float lat, lon;
};

class OSMdata {
private:
    bool drawSubway, drawAttractions, drawRailway; //Bools to check when drawing
    std::vector<OSM_node_data> subwayStations;
    std::vector<OSM_node_data> attractions;
    std::vector<OSM_way_data> subwayLines;

    //std::vector<OSM_node_data> railwayStations;
    std::vector<OSM_way_data> railwayLines;


    //These vectors are used for (relativly) fast conversion from OSMid to regular id
    std::vector<OSM_id_pair> conversionVector;
    std::vector<OSM_id_pair> wayConversionVector;

    //Stores all the points of interests
    std::vector<OSM_POI> pointsOfInterest;

public:
    //Constructor and destructor    
    OSMdata();
    ~OSMdata();

    //Subway related functions
    OSM_node_data getSubwayStation(unsigned index);
    unsigned getNumberOfStations();
    void set_draw_subway(bool valid);
    bool draw_subway();
    OSM_way_data get_subway_line(unsigned index);
    unsigned get_number_of_lines();


    //Attraction related functions
    OSM_node_data getAttraction(unsigned index);
    void set_draw_attractions(bool valid);
    bool draw_attractions();
    unsigned getNumberOfAttractions();

    //OSMID conversion related functions
    unsigned OSMid_to_id_NODE(unsigned left, unsigned right, unsigned OSMid);
    unsigned OSMid_to_id_WAY(unsigned left, unsigned right, unsigned OSMid);
    unsigned get_conversionVector_size();
    unsigned get_way_conversionVector_size();

    //Railway related functions
    void set_draw_railway(bool valid);
    bool draw_railway();
    OSM_way_data get_railway(unsigned index);
    unsigned get_number_of_railways();

    //POI related functions
    void build_POI_vector();
    std::string POI_get_name(unsigned id);
    std::string POI_get_type(unsigned id);
    //Returns a lat, lon pair
    std::pair<float, float> POI_get_position(unsigned id);
    unsigned get_number_of_POI();

    //Other functions
    void getNodeTags(unsigned NodeID);
    friend bool sortID(OSM_id_pair &a, OSM_id_pair &b);
    std::string getStreetSegmentType(OSMID &id);

    friend class interface;
};

//Friend functions
bool sortID(OSM_id_pair &a, OSM_id_pair &b);

#endif /* OSMDATA_H */

