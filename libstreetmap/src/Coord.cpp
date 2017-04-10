/* Author: Jack Lee
 * This class stores constants and conversion functions for
 * the Geographic coordinates and Cartesian coordinates
 */

#include "Coord.h"

//Constructor

Coord::Coord() {
    latlon_max.first = -90;
    latlon_max.second = -180;
    latlon_min.first = 90;
    latlon_min.second = 180;

    //Loop through every feature point
    unsigned feature_num = getNumberOfFeatures();
    for (unsigned i = 0; i < feature_num; i++) {
        unsigned feature_count = getFeaturePointCount(i);
        for (unsigned n = 0; n < feature_count; n++) {
            if (latlon_max.first < getFeaturePoint(i, n).lat()) latlon_max.first = getFeaturePoint(i, n).lat();
            if (latlon_min.first > getFeaturePoint(i, n).lat()) latlon_min.first = getFeaturePoint(i, n).lat();
            if (latlon_max.second < getFeaturePoint(i, n).lon()) latlon_max.second = getFeaturePoint(i, n).lon();
            if (latlon_min.second > getFeaturePoint(i, n).lon()) latlon_min.second = getFeaturePoint(i, n).lon();
        }
    }

    LAT_AVG = (latlon_max.first + latlon_min.first) / 2;
    cosineResult = cos(DEG_TO_RAD * LAT_AVG);

    xy_max.first = lon_to_x(latlon_max.second);
    xy_max.second = lat_to_y(latlon_max.first);
    xy_min.first = lon_to_x(latlon_min.second);
    xy_min.second = lat_to_y(latlon_min.first);
}

//Destructor

Coord::~Coord() {
}

double Coord::x_to_lon(double x) {
    return x / (DEG_TO_RAD * cosineResult * SCALE_FACTOR);
}

double Coord::y_to_lat(double y) {
    return y / (DEG_TO_RAD * SCALE_FACTOR);
}

double Coord::lon_to_x(double lon) {
    return DEG_TO_RAD * lon * cosineResult * SCALE_FACTOR;
}

double Coord::lat_to_y(double lat) {
    return DEG_TO_RAD * lat * SCALE_FACTOR;
}

double Coord::get_cos_result() {
    return cosineResult;
}

