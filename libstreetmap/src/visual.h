/* Author: Jack Lee, Marius Stan, Matthew Chapleau
 * Class "map" stores x,y coordinates for all drawable object
 * Includes drawing functions
 */

#ifndef MAP_H
#define MAP_H

#include "m1.h"
#include "graphics.h"
#include "StreetsDatabaseAPI.h"
#include "Global.h"
#include "Global2.h"
#include <math.h>
#include <algorithm>
#include "m4.h"

struct intersection_data {
    map_coord xy;
    string data;
};

//t_point = xy coord

struct strSegGraphicInfo {
    std::string name;
    t_point from;
    t_point to;
    t_color roadClassColour;
    int width;
};

struct cpSeg {
    t_point from;
    t_point to;
};

struct pathSSeg {
    unsigned id;
    string name;
    string classification;
    //t_point from;
    //t_point to;
    vector<cpSeg> curvepts;
};

struct dirnInstruct {
    vector<pathSSeg*> streetSec; //if segments belong to same street put them in 1 instruct
    std::string instructFollow;
    std::string instructTurn;
    double dist;
    double roundedDist;
    string distStr;
    pair<float, float> startOfSeg;
};

struct feature_data {
    std::string name;
    vector<t_point> points;
    t_color colour;
    //t_color textColour;
    //t_bound_box textBox;
    bool isClosed;
    float area;
    int drawAtLowZoom; //0 - always draw, 1 - draw at moderate zoom, 2 - draw at close zoom

    //used for sorting features from largest to smallest

    bool operator()(feature_data i, feature_data j) {
        return (i.area > j.area);
    }
};

struct POI_data {
    unsigned id;
    std::string name;
    t_point location;
};

struct POI_type {
    std::string type;
    vector<POI_data> poi_list;

};


//Moved definition to graphics.h (needed to create a new function in there that took a labels parameter)

/*
struct labels {
    t_bound_box textBox;
    int angle;
    t_color textColour;
    std::string name;
    char type;
    t_point centre;
    float streetlength; //only for streets
    string directionIcon;
};*/

class visual {
private:
    //Streets
    vector<strSegGraphicInfo> minorStreets;
    vector<strSegGraphicInfo> majorStreets;
    vector<strSegGraphicInfo> highways;
    //Intersections
    vector<intersection_data> intersection;
    //Points of interests
    vector<POI_type> points_Of_Interest;
    //features sorted by area (large to small)
    vector<feature_data> features;
    //labels
    vector<labels> itemLabels;
    //NOTE: Bottom left corner of street label box is actually top-left, top-right corner is actually bottom-right
    vector<labels> drawnTBoxes;
    //Note, this vector only stores label positions and thus is only for checking for overlap, not for directly drawing
    vector<t_bound_box> POILabels;

    //vector that stores shortest path (as pathSSeg struct)
    vector<pathSSeg> path;
    //Stores the id's of the current POIs on screen
    vector<unsigned> POIidOnScreen;

    vector<dirnInstruct> instructions;


public:
    //Constructor & Destructor
    visual();
    ~visual();

    //Streets (names) / (major , minor, one-way roads) <Matthew>
    void draw_streets();
    void build_street();

    //Intersections / (clickable, highlight and displays info) <Jack>
    void build_intersection();
    void draw_intersection();

    //Points of interests (names) <Marius>
    void build_POI();
    friend std::string sort_POI(std::string type);
    bool draw_POI_text(float x, float y, std::string name, unsigned id);
    bool POI_zoom_valid(int index, float area);
    void draw_POI();
    bool is_POI_on_screen(unsigned id);

    //Railways and Subways <Marius>
    void draw_subways();
    void draw_railways();

    //Features <Matthew>
    void build_features();
    void draw_features();

    void draw_labels();

    void checkTBoxesOnScreen();

    float polyArea(vector<t_point>& points, unsigned numPoints);

    void createPath(vector<unsigned int> srcPath);
    void drawPath();

    void createInstructions();

    float checkTurn(pathSSeg& currentS, pathSSeg& nextS);

    string instructForSSegsWithSameName(vector<unsigned>& srcPath, unsigned pos);

    void drawHighlightedStreet(dirnInstruct highlightedSeg);

    //void test();

    void drawM4Info(const std::vector<DeliveryInfo>& deliveries, const std::vector<unsigned>& depots);

    void updatePathToDraw(const vector<unsigned int> &srcPath);

    //void drawM4Path(const vector<unsigned> srcPath);

    friend class interface;
};

void draw_screen();
std::string sort_POI(std::string type);

#endif /* MAP_H */

