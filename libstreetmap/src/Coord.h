/* Author: Jack Lee
 * This class stores constants and conversion functions for
 * the Geographic coordinates and Cartesian coordinates
 * <ATTENTION> Map(X,Y) coordinates has been scaled by <SCALE_FACTOR>
 * <ATTENTION> For Toronto the coordinate range from <-133747, 75886.9> to <-132423, 76654.8>
 * <ATTENTION> X axis incremented <1324> steps, Y axis incremented <767.9> steps
 * <ATTENTION> Please decide drawing size accordingly
 */

#include <math.h>
#include "m1.h"
#include "StreetsDatabaseAPI.h"

#ifndef COORDINATE_H
#define COORDINATE_H

#define SCALE_FACTOR 100000

using namespace std;

//lat and long
typedef pair<double, double> world_coord;
//x and y
typedef pair<double, double> map_coord;

class Coord {
private:
    pair<double, double> latlon_max, latlon_min, xy_max, xy_min;
    double LAT_AVG;
    double cosineResult;

public:
    //Constructor & Destructor
    Coord();
    ~Coord();

    //Coordinate Conversion
    double x_to_lon(double x);
    double y_to_lat(double y);
    double lon_to_x(double lon);
    double lat_to_y(double lat);

    //Accessor
    double get_cos_result();

    //Allowing map class to access private members
    friend class visual;
};

#endif /* COORDINATE_H */

