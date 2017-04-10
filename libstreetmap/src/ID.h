/* Author: Jack Lee
 * This class stores all data ids and their conversion functions
 */

#ifndef ID_H
#define ID_H

#include "m1.h"
#include <algorithm>
#include <unordered_map>
#include "StreetsDatabaseAPI.h"

using namespace std;


typedef unordered_multimap<string, unsigned> streetDB;

class ID {
    // friend class map;

private:
    //Data structures
    //vector of street ids each holding a vector of street segments
    vector<vector<StreetSegmentIndex>> StrSegmatrix;
    //vector of intersection ids holding vector of connected street segments
    vector<vector<StreetSegmentIndex>> InterSegmatrix;
    //vector of street ids holding vector of intersection ids along the street
    vector<vector<IntersectionIndex>> StrIntermatrix;
    //vector of street ids holding street segment lengths
    vector<double> SegLenvec;
    //vector of street ids holding street seg travel times
    vector<double> SegTimvec;
    //unordered multmap matching street name with street id
    streetDB streetInfo;
    //Street Name vector for search
    vector<pair<string, string>> SearchData;
    vector<pair<string, string>> POI_data;
    vector<pair<string, string>> Highway_data;
    vector<pair<string, string>> Major_data;
    vector<pair<string, string>> Minor_data;

    //Maximum speed limit on a given map in m/s, used for pathfinding algorithm estimation
    double maxSpeedLimit;


    //Build functions called by constructor
    void build_StrSegmatrix();
    void build_InterSegmatrix();
    void build_StrIntermatrix();
    void build_SegLenTimvecs();
    void build_streetInfo();
    void build_SearchData();


public:
    //Constructor & Destructor
    ID();
    ~ID();

    //Accessors
    vector<StreetIndex> get_street_ids_from_street_name(string street_name);
    vector<StreetSegmentIndex> get_segment_ids_from_intersection_id(unsigned intersection_id);
    vector<StreetSegmentIndex> get_segment_ids_from_street_id(unsigned street_id);
    vector<IntersectionIndex> get_intersection_ids_from_street_id(unsigned street_id);
    double get_length_of_street_segment(unsigned street_segment_id);
    double get_travel_time_of_street_segment(unsigned street_segment_id);
    double get_max_speed_limit();

    //Size function
    unsigned get_size_of_StrIntermatrix();

    //Sort POI
    string sort_POI(string type);

    //Search Function
    string search_input_type(string input);
    string search_autocomplete(string typed);
};

#endif /* ID_H */

