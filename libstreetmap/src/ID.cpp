/* Author: Jack Lee
 * This class stores all data ids and their conversion functions
 */

#include "ID.h"
#include "Global2.h"
#include <chrono>
//Constructor
//#define PROFILE_IDLOAD

ID::ID() {
    maxSpeedLimit = 0;

#ifdef PROFILE_IDLOAD
    static std::ofstream perf;
    perf.open("performance_data.csv", std::ios_base::app);
    auto const start = std::chrono::high_resolution_clock::now();
#endif
    // std::cout << "  Building street segments from streets..." << std::endl;
    build_StrSegmatrix();

#pragma omp parallel for num_threads(5)
    for (int i = 0; i < 5; ++i) {
        //std::cout << "  Building street segments from intersections..." << std::endl;
        if (i == 0)
            build_InterSegmatrix();
            //std::cout << "  Building intersections from streets..." << std::endl;
        else if (i == 1)
            build_StrIntermatrix();
            //std::cout << "  Building distance-time data structures..." << std::endl;
        else if (i == 2)
            build_SegLenTimvecs();
            //std::cout << "  Building street name multimap..." << std::endl;
        else if (i == 3)
            build_streetInfo();
            //std::cout << "  Building search data..." << std::endl;
        else if (i == 4)
            build_SearchData();
    }

#ifdef PROFILE_IDLOAD
    auto const end = std::chrono::high_resolution_clock::now();
    auto const delta_time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "id load took: " << delta_time.count() << "ns\n";
    perf << "idload(no multi-threading)-London," << delta_time.count() << std::endl;
    perf.close();
#endif
}

//Destructor

ID::~ID() {
    StrSegmatrix.clear();
    InterSegmatrix.clear();
    StrIntermatrix.clear();
    SegLenvec.clear();
    SegTimvec.clear();
    streetInfo.clear();
    SearchData.clear();
}

//Vector of street segment ids sorted by associated street ids

void ID::build_StrSegmatrix() {
    //Sort Street/Street Segment pairs
    vector<pair<StreetIndex, StreetSegmentIndex>> StrSeg_plist;
    unsigned seg_num = getNumberOfStreetSegments();

    for (unsigned n = 0; n < seg_num; n++) {
        StreetSegmentInfo seg_info = getStreetSegmentInfo(n);
        pair<StreetIndex, StreetSegmentIndex> StrSeg_pair(seg_info.streetID, n);
        StrSeg_plist.push_back(StrSeg_pair);
    }
    sort(StrSeg_plist.begin(), StrSeg_plist.end());

    //Build StrSegmatrix
    vector<StreetSegmentIndex> Seg_list;
    unsigned street = StrSeg_plist[0].first;

    for (vector<pair < StreetIndex, StreetSegmentIndex>>::iterator list_iter = StrSeg_plist.begin(); list_iter != StrSeg_plist.end(); list_iter++) {
        if ((*list_iter).first == street) {
            Seg_list.push_back((*list_iter).second);
            //corner case: reaching end without push_back
            if ((list_iter + 1) == StrSeg_plist.end()) StrSegmatrix.push_back(Seg_list);
        } else {
            StrSegmatrix.push_back(Seg_list);
            Seg_list.clear();
            Seg_list.push_back((*list_iter).second);
            street = (*list_iter).first;
        }
    }
}

//Vector of street segment ids sorted by associated intersection ids

void ID::build_InterSegmatrix() {
    unsigned inter_num = getNumberOfIntersections();

    for (unsigned i = 0; i < inter_num; i++) {
        vector<StreetSegmentIndex> seg_id;
        unsigned seg_num = getIntersectionStreetSegmentCount(i);
        for (unsigned n = 0; n < seg_num; n++) {
            seg_id.push_back(getIntersectionStreetSegment(i, n));
        }
        InterSegmatrix.push_back(seg_id);
    }
}

//Vector of intersection ids sorted by associated street ids

void ID::build_StrIntermatrix() {
    for (vector<vector < StreetSegmentIndex>>::iterator str_iter = StrSegmatrix.begin(); str_iter != StrSegmatrix.end(); str_iter++) {
        vector<IntersectionIndex>inter_id;
        //Loop through all street segments and extract associated intersection ids
        for (vector<StreetSegmentIndex>::iterator seg_iter = (*str_iter).begin(); seg_iter != (*str_iter).end(); seg_iter++) {
            StreetSegmentInfo seg_info = getStreetSegmentInfo(*seg_iter);
            inter_id.push_back(seg_info.from);
            inter_id.push_back(seg_info.to);
        }
        //Sort and remove duplicates in <inter_ids>
        sort(inter_id.begin(), inter_id.end());
        inter_id.erase(unique(inter_id.begin(), inter_id.end()), inter_id.end());
        //Build
        StrIntermatrix.push_back(inter_id);
    }
}

//Vector of street segment length sorted by associated street ids

void ID::build_SegLenTimvecs() {
    unsigned seg_num = getNumberOfStreetSegments();
    double speedLimit;
    for (unsigned street_segment_id = 0; street_segment_id < seg_num; street_segment_id++) {
        IntersectionIndex start, end;
        double distance = 0;
        unsigned curvePoints = getStreetSegmentInfo(street_segment_id).curvePointCount;

        //Getting the start and end point ids
        start = getStreetSegmentInfo(street_segment_id).from;
        end = getStreetSegmentInfo(street_segment_id).to;

        //Finding distance by retrieving the coordinates and using the distance function
        //If there are no curve points, distance is just that between the start and end coordinate
        if (curvePoints == 0) {
            distance = find_distance_between_two_points(getIntersectionPosition(start), getIntersectionPosition(end));
        } else {
            //Otherwise, distance between startpoint, endpoint, and all curve points must be found
            distance += find_distance_between_two_points(getIntersectionPosition(start), getStreetSegmentCurvePoint(street_segment_id, 0)); //Distance between start & point 1
            for (unsigned i = 0; i < curvePoints; i++) {
                if (i == curvePoints - 1) {
                    //distance between last curve point and end point
                    distance += find_distance_between_two_points(getIntersectionPosition(end), getStreetSegmentCurvePoint(street_segment_id, curvePoints - 1));
                } else {
                    //Distance between two consecutive curve points
                    distance += find_distance_between_two_points(getStreetSegmentCurvePoint(street_segment_id, i), getStreetSegmentCurvePoint(street_segment_id, i + 1));
                }
            }
        }

        //Build SegLenvec
        SegLenvec.push_back(distance);

        speedLimit = (getStreetSegmentInfo(street_segment_id).speedLimit) * 5 / 18;

        if (speedLimit > maxSpeedLimit) maxSpeedLimit = speedLimit;

        //Build SegTimevec: Street Segment travel time
        SegTimvec.push_back(distance / (speedLimit));
    }
}

//Create unordered multi-map Key: street name Value: street id

void ID::build_streetInfo() {
    unsigned street_num = getNumberOfStreets();
    std::string strtName;
    for (StreetIndex strtIndex = 0; strtIndex < street_num; strtIndex++) {
        strtName = getStreetName(strtIndex);
        streetInfo.insert(std::pair<std::string, unsigned>(strtName, strtIndex));
    }
}

//Vector of all street names sorted by importance

void ID::build_SearchData() {
    /* Search Data is a vector of pairs <name, type>
     * it is organized by search priority: Major roads, Minor roads, POI, highways
     */
    unsigned POI_num = getNumberOfPointsOfInterest();
    for (unsigned n = 0; n < POI_num; n++) {
        pair<string, string> temp(getPointOfInterestName(n), "P" + to_string(n));
        POI_data.push_back(temp);
    }

    unsigned i = 0;
    for (vector<vector < StreetSegmentIndex>>::iterator iter = StrSegmatrix.begin(); iter != StrSegmatrix.end(); iter++) {
        float speedlimit = getStreetSegmentInfo((*iter)[(*iter).size() / 2]).speedLimit;

        if (speedlimit >= 88) {
            //Highway
            pair<string, string> temp(getStreetName(i), "S" + to_string(i));
            Highway_data.push_back(temp);
        } else if (speedlimit >= 60) {
            //Major
            pair<string, string> temp(getStreetName(i), "S" + to_string(i));
            Major_data.push_back(temp);
        } else {
            //Minor
            pair<string, string> temp(getStreetName(i), "S" + to_string(i));
            Minor_data.push_back(temp);
        }

        i++;
    }

    //Sort and remove duplicates for all vectors
    sort(POI_data.begin(), POI_data.end());
    POI_data.erase(unique(POI_data.begin(), POI_data.end()), POI_data.end());
    sort(Highway_data.begin(), Highway_data.end());
    Highway_data.erase(unique(Highway_data.begin(), Highway_data.end()), Highway_data.end());
    sort(Major_data.begin(), Major_data.end());
    Major_data.erase(unique(Major_data.begin(), Major_data.end()), Major_data.end());
    sort(Minor_data.begin(), Minor_data.end());
    Minor_data.erase(unique(Minor_data.begin(), Minor_data.end()), Minor_data.end());

    //Concatenate in order
    SearchData.clear();
    SearchData.insert(SearchData.end(), Major_data.begin(), Major_data.end());
    SearchData.insert(SearchData.end(), Minor_data.begin(), Minor_data.end());
    SearchData.insert(SearchData.end(), POI_data.begin(), POI_data.end());
    SearchData.insert(SearchData.end(), Highway_data.begin(), Highway_data.end());
}

vector<StreetIndex> ID::get_street_ids_from_street_name(string street_name) {
    vector<StreetIndex> street_ids;
    //retrieve all elements with the key street_name and add to vector
    auto range = streetInfo.equal_range(street_name);
    for (auto iter = range.first; iter != range.second; ++iter) {
        street_ids.push_back(iter->second);
    }

    return (street_ids);
}

vector<StreetSegmentIndex> ID::get_segment_ids_from_intersection_id(unsigned intersection_id) {
    return InterSegmatrix[intersection_id];
}

vector<StreetSegmentIndex> ID::get_segment_ids_from_street_id(unsigned street_id) {
    return StrSegmatrix[street_id];
}

vector<IntersectionIndex> ID::get_intersection_ids_from_street_id(unsigned street_id) {
    return StrIntermatrix[street_id];
}

double ID::get_length_of_street_segment(unsigned street_segment_id) {
    return SegLenvec[street_segment_id];
}

double ID::get_travel_time_of_street_segment(unsigned street_segment_id) {
    return SegTimvec[street_segment_id];
}

unsigned ID::get_size_of_StrIntermatrix() {
    return StrIntermatrix.size();
}

double ID::get_max_speed_limit() {
    return maxSpeedLimit;
}

string ID::sort_POI(string type) {
    if (type == "community_centre" || type == "swimming_pool" || type == "arts_centre" || type == "social_facility")
        return "Community Centre";
    else if (type == "kindergarten" || type == "school" || type == "childcare" || type == "college" || type == "university" || type == "driving_school" || type == "music_school" || type == "tutoring")
        return "School";
    else if (type == "parking" || type == "bicycle_parking" || type == "motorcycle_parking")
        return "Parking Lot";
    else if (type == "bank" || type == "bureau_de_change" || type == "atm")
        return "Bank";
    else if (type == "fast_food" || type == "restaurant" || type == "food_court" || type == "ice_cream" || type == "bbq")
        return "Restaurant";
    else if (type == "cafe" || type == "internet_cafe")
        return "Cafe";
    else if (type == "hospital" || type == "ambulance_station")
        return "Hospital";
    else if (type == "police")
        return "Police Station";
    else if (type == "fire_station")
        return "Fire Station";
    else if (type == "place_of_worship")
        return "Church";
    else if (type == "library")
        return "Library";
    else if (type == "attraction")
        return "Attraction";
    else if (type == "bus_station" || type == "taxi" || type == "subway")
        return "Transportation";
    else if (type == "fuel" || type == "car_wash")
        return "Gas Station";
    else if (type == "toilets")
        return "Washroom";
    else if (type == "nightclub" || type == "casino" || type == "pub" || type == "bar" || type == "stripclub")
        return "Bar";
    else if (type == "theatre" || type == "cinema")
        return "Cinema";
    else if (type == "marketplace" || type == "shopping")
        return "Store";
    else if (type == "public_building" || type == "townhall")
        return "Public Property";
    else if (type == "post_office")
        return "Post Station";
    else if (type == "dentist" || type == "optometrist" || type == "veterinary" || type == "chiropractor" || type == "doctors" || type == "medical_center" || type == "pharmacy" || type == "health_centre" || type == "clinic")
        return "Health Care";
    else if (type == "bicycle_rental" || type == "car_rental")
        return "Rental Shop";
    else if (type == "telephone")
        return "Phone Booth";
    else return "<unknown>";
}

string ID::search_input_type(string input) {
    for (vector<pair < string, string>>::iterator iter = SearchData.begin(); iter != SearchData.end(); iter++) {
        if (input == iter->first) {
            return (iter->second);
        }
    }

    return "\0";
}

string ID::search_autocomplete(string typed) {
    for (vector<pair < string, string>>::iterator iter = SearchData.begin(); iter != SearchData.end(); iter++) {
        if (!iter->first.find(typed, 0)) {
            return (iter->first);
        }
    }

    return "";
}
