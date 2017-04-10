/* Author: Jack Lee, Marius Stan, Matthew Chapleau
 * Class "map" stores x,y coordinates for all drawable object
 * Includes drawing functions
 */

#include "visual.h"
#include "m1.h"
#include "interface.h"
#include <math.h>
#include "Global3.h"
#include <iterator>

//#define SHOW_TEXT_BBOX
#define number_Of_Categories 26
//Arrays for point of interest categories and symbols IF THIS ARRAY IS CHANGED IT NEEDS TO BE CHANGED IN KD TREE CLASS AS WELL
std::string POIcategories[number_Of_Categories] = {"EMS", "Police", "Fire", "Public", "Schools", "Community", "Library", "Cafe",
    "Transportation", "Subway", "Train", "Shop", "Church", "Fuel", "Cinema", "Money", "Bar", "Doctor", "Food",
    "Post", "Rental", "Phone", "Toilets", "Attractions", "Parking", "Other"};


//Initializes map & set coordinate systems

visual::visual() {

    //Name and background for the map
    t_color background(242, 229, 212, 255);
    init_graphics("ATOMIC GIS", background);

    //(min_longitude * cos(avg_lat), min_latitude, max_longitude * cos(avg_lat), max_latitude)
    set_visible_world(mapCoord->xy_min.first, mapCoord->xy_min.second, mapCoord->xy_max.first, mapCoord->xy_max.second);

    //Turn on mouse move and keypress callback functions
    set_mouse_move_input(true);
    set_keypress_input(true);

    build_street();
    build_intersection();
    build_POI();
    build_features();
}

//Closes map

visual::~visual() {
    close_graphics();
}


//builds street vector for graphics (xy positions)

void visual::build_street() {
    unsigned street_num = mapID->get_size_of_StrIntermatrix();
    //loop through streets to build vector storing name and 2 xy positions for segments
    for (unsigned i = 0; i < street_num; i++) {

        strSegGraphicInfo temp;

        labels tempLabel;
        temp.name = getStreetName(i);

        vector<StreetSegmentIndex> tempSeg = mapID->get_segment_ids_from_street_id(i);

        //loop through segments to set the 2 xy positions
        for (auto iter = tempSeg.begin(); iter < tempSeg.end(); iter++) {

            //get endpoints of street seg and convert to xy coords
            StreetSegmentInfo sInfo = getStreetSegmentInfo(*iter);

            LatLon start = getIntersectionPosition(sInfo.from);
            LatLon end = getIntersectionPosition(sInfo.to);

            t_point tempStart;
            t_point tempEnd;

            tempStart.x = mapCoord->lon_to_x(start.lon());
            tempStart.y = mapCoord->lat_to_y(start.lat());
            tempEnd.x = mapCoord->lon_to_x(end.lon());
            tempEnd.y = mapCoord->lat_to_y(end.lat());

            unsigned cpCount = sInfo.curvePointCount;
            //loop through curve points
            for (unsigned cp = 0; cp <= cpCount; cp++) {
                if (cpCount == 0) {
                    temp.from.x = tempStart.x;
                    temp.from.y = tempStart.y;
                    temp.to.x = tempEnd.x;
                    temp.to.y = tempEnd.y;

                } else {
                    //connect to start
                    if (cp == 0) {
                        temp.from.x = mapCoord->lon_to_x(start.lon());
                        temp.from.y = mapCoord->lat_to_y(start.lat());
                        LatLon point = getStreetSegmentCurvePoint(*iter, cp);
                        temp.to.x = mapCoord->lon_to_x(point.lon());
                        temp.to.y = mapCoord->lat_to_y(point.lat());
                    }//connect to end
                    else if (cp == cpCount) {
                        LatLon point = getStreetSegmentCurvePoint(*iter, cp - 1);
                        temp.from.x = mapCoord->lon_to_x(point.lon());
                        temp.from.y = mapCoord->lat_to_y(point.lat());
                        temp.to.x = mapCoord->lon_to_x(end.lon());
                        temp.to.y = mapCoord->lat_to_y(end.lat());
                    }//connect middle points
                    else {
                        LatLon pointFrom = getStreetSegmentCurvePoint(*iter, (cp - 1));
                        temp.from.x = mapCoord->lon_to_x(pointFrom.lon());
                        temp.from.y = mapCoord->lat_to_y(pointFrom.lat());

                        LatLon pointTo = getStreetSegmentCurvePoint(*iter, cp);
                        temp.to.x = mapCoord->lon_to_x(pointTo.lon());
                        temp.to.y = mapCoord->lat_to_y(pointTo.lat());
                    }
                }
                //add to street vector and
                //determine if highway, major or minor based on osm classification
                string classification = extraMapData->getStreetSegmentType(sInfo.wayOSMID);
                
                if (classification == "motorway" || classification == "motorway_link") {
                    temp.roadClassColour = t_color(239, 209, 81, 255);
                    temp.width = 2;

                    highways.push_back(temp);
                }
                else if (classification == "trunk" || classification == "trunk_link" ||
                        classification == "primary" || classification == "primary_link" ||
                        classification == "secondary" || classification == "secondary_link") {
                    temp.roadClassColour = WHITE;
                    temp.width = 1;

                    majorStreets.push_back(temp);
                } else {
                    temp.roadClassColour = t_color(228, 215, 198, 255);
                    temp.width = 1;

                    minorStreets.push_back(temp);
                }

                //only label every other cp segment

                if (cp % 2 == 0) {
                    //create label
                    tempLabel.id = *iter;
                    //check 1 way
                    //and draw arrow in direction
                    //and set textbox to size of street;
                    if (sInfo.oneWay) {
                        if (temp.from.x < temp.to.x) {
                            tempLabel.name = temp.name + "  >";
                            if (temp.from.y < temp.to.y) {
                                tempLabel.textBox = t_bound_box(temp.from.x, temp.from.y, temp.to.x, temp.to.y);
                            } else {
                                tempLabel.textBox = t_bound_box(temp.from.x, temp.to.y, temp.to.x, temp.from.y);
                            }
                        } else {
                            tempLabel.name = "<  " + temp.name;
                            if (temp.from.y < temp.to.y) {
                                tempLabel.textBox = t_bound_box(temp.to.x, temp.from.y, temp.from.x, temp.to.y);
                            } else {
                                tempLabel.textBox = t_bound_box(temp.to.x, temp.to.y, temp.from.x, temp.from.y);
                            }
                        }
                    } else {
                        tempLabel.name = temp.name;
                    }

                    //determine angle
                    tempLabel.angle = static_cast<int> ((atan((temp.to.y - temp.from.y) / (temp.to.x - temp.from.x)))*180 / PI);

                    tempLabel.textColour = t_color(52, 73, 94, 255);
                    tempLabel.centre.x = (temp.to.x + temp.from.x) / 2;
                    tempLabel.centre.y = (temp.to.y + temp.from.y) / 2;

                    tempLabel.streetlength = sqrt(pow((temp.to.x - temp.from.x), 2.0) + pow((temp.to.y - temp.from.y), 2.0));

                    itemLabels.push_back(tempLabel);
                }
            }
        }

    }

}

//Build vector intersection and draw all intersections
//Build vector intersection

void visual::build_intersection() {
    /*
    unsigned inter_num = getNumberOfIntersections();
    intersection.resize(inter_num);

    for (unsigned i = 0; i < inter_num; i++) {
        //Build data structure
        intersection_data temp;
        temp.xy.first = mapCoord->lon_to_x(getIntersectionPosition(i).lon());
        temp.xy.second = mapCoord->lat_to_y(getIntersectionPosition(i).lat());
        temp.data = getIntersectionName(i);
        intersection.push_back(temp);
    }*/
}

//Builds the points of interest by looping through the ID vector then sorting them in the appropriate category with the appropriate icon

void visual::build_POI() {
    unsigned POI_num = extraMapData->get_number_of_POI();
    unsigned POI_type_num = number_Of_Categories;
    points_Of_Interest.resize(POI_type_num);

    for (unsigned j = 0; j < POI_type_num; j++) {
        points_Of_Interest[j].type = POIcategories[j];
    }

    //Loops through all points of interest and sorts them into the appropriate category
    for (unsigned i = 0; i < POI_num; i++) {
        POI_data temp;
        temp.location.x = mapCoord->lon_to_x(extraMapData->POI_get_position(i).second);
        temp.location.y = mapCoord->lat_to_y(extraMapData->POI_get_position(i).first);
        temp.name = extraMapData->POI_get_name(i);
        temp.id = i;
        std::string temp_category = sort_POI(extraMapData->POI_get_type(i));

        for (unsigned j = 0; j < POI_type_num; j++) {
            if (temp_category == points_Of_Interest[j].type) {
                points_Of_Interest[j].poi_list.push_back(temp);
            }
        }
    }




}

//This function takes in a POI type and spits out the category it belongs to (category meaning the one of 23 defined in the POIcategory array)

std::string sort_POI(std::string type) {
    if (type == "community_centre" || type == "swimming_pool" || type == "arts_centre" || type == "social_facility")
        return "Community";
    else if (type == "kindergarten" || type == "school" || type == "childcare" || type == "college" || type == "university" || type == "driving_school" || type == "music_school" || type == "tutoring")
        return "Schools";
    else if (type == "parking" || type == "bicycle_parking" || type == "motorcycle_parking")
        return "Parking";
    else if (type == "bank" || type == "bureau_de_change" || type == "atm")
        return "Money";
    else if (type == "fast_food" || type == "restaurant" || type == "food_court" || type == "ice_cream" || type == "bbq")
        return "Food";
    else if (type == "cafe" || type == "internet_cafe")
        return "Cafe";
    else if (type == "hospital" || type == "ambulance_station")
        return "EMS";
    else if (type == "police")
        return "Police";
    else if (type == "fire_station")
        return "Fire";
    else if (type == "place_of_worship")
        return "Church";
    else if (type == "library")
        return "Library";
    else if (type == "subway")
        return "Subway";
    else if (type == "attraction")
        return "Attractions";
    else if (type == "bus_station" || type == "taxi")
        return "Transportation";
    else if (type == "fuel" || type == "car_wash")
        return "Fuel";
    else if (type == "toilets")
        return "Toilets";
    else if (type == "nightclub" || type == "casino" || type == "pub" || type == "bar" || type == "stripclub")
        return "Bar";
    else if (type == "theatre" || type == "cinema")
        return "Cinema";
    else if (type == "marketplace" || type == "shopping")
        return "Shop";
    else if (type == "public_building" || type == "townhall")
        return "Public";
    else if (type == "post_office")
        return "Post";
    else if (type == "dentist" || type == "optometrist" || type == "veterinary" || type == "chiropractor" || type == "doctors" || type == "medical_center" || type == "pharmacy" || type == "health_centre" || type == "clinic")
        return "Doctor";
    else if (type == "bicycle_rental" || type == "car_rental")
        return "Rental";
    else if (type == "telephone")
        return "Phone";
    else return "Other";

}

void visual::draw_POI() {
    POILabels.clear();
    POIidOnScreen.clear();
    t_bound_box currentScreen = get_visible_world();
    float screenArea = currentScreen.area();

    bool overLap = true;
    
    Surface icon;
    std::string fileTemp;
    //Goes through POI vector and draws each POI with the appropriate symbol if the zoom level is correct
    for (unsigned j = 0; j < number_Of_Categories; j++) {

        if (POI_zoom_valid(j, screenArea) == true) {
            //Checks if current POI is in bounds

            //creates a string for the file location, converts string to const char * then loads the surface to be used for drawing
            fileTemp = "Icons/" + points_Of_Interest[j].type + ".png";
            const char * fileLocation = fileTemp.c_str();
            icon = load_png_from_file(fileLocation);

            for (vector<POI_data>::iterator iter = points_Of_Interest[j].poi_list.begin(); iter != points_Of_Interest[j].poi_list.end(); iter++) {
                //Checks if current POI is in bounds
                if (currentScreen.intersects(iter->location.x, iter->location.y)) {
                    float x_center = xscrn_to_world(xworld_to_scrn(iter->location.x) - 10);
                    float y_center = yscrn_to_world(yworld_to_scrn(iter->location.y) - 10);
                    overLap = draw_POI_text(x_center, y_center, iter->name, iter->id);
                    if (overLap == false) draw_surface(icon, x_center, y_center);
                }
            }
        }
    }
}

//Draws the Point of Interest Text by converting to screen coordinates then checking for overlap with other POI text, if no overlap

bool visual::draw_POI_text(float x, float y, std::string name, unsigned id) {

    settextrotation(0);
    setfontsize(8);
    bool valid = true;
    float boxWidth = 100, boxHeight = 16;
    t_bound_box * textBox;
    setcolor(111, 36, 20);

    //Centering the text
    int new_x = xworld_to_scrn(x) + 10;
    int new_y = yworld_to_scrn(y) - 8;

    float xLeft = xscrn_to_world(new_x - boxWidth / 2);
    float xRight = xscrn_to_world(new_x + boxWidth / 2);
    float yTop = yscrn_to_world(new_y - boxHeight / 2);
    float yBottom = yscrn_to_world(new_y + boxHeight / 2 + 19);

    //Inserting text box position into array
    textBox = new t_bound_box(xLeft, yBottom, xRight, yTop);


    //Loops through all current labels and checks for collision, if collision occurs text is inserted below, if collision still occurs, icon is not drawn
    for (vector<t_bound_box>::iterator iter = POILabels.begin(); iter != POILabels.end(); iter++) {
        if (iter->intersects(*textBox, 0) == true) {
            valid = false;
            break;
        }
    }

    //Checks all street labels for collision
    //NOTE Bottom left corner of street label box is actually top-left, top-right corner is actually bottom-right
    for (vector<labels>::iterator iter = drawnTBoxes.begin(); iter != drawnTBoxes.end(); iter++) {
        if (iter->textBox.intersects(*textBox, 1) == true) {
            valid = false;
            break;
        }
    }

    if (valid == true) {
        drawtext(xscrn_to_world(new_x), yscrn_to_world(new_y), name, 50, 10);
        POILabels.push_back(*textBox);
        POIidOnScreen.push_back(id);
    } else {
        delete textBox;

    }

    //This function is not yet complete
    return !valid;

}

//Checks if the POI should be drawn depending on zoom level, index is the index of the POI type array

bool visual::POI_zoom_valid(int index, float area) {
    //Nothing should be drawn at this zoom level
    if (area >= 17100.0) return false;
        //Case for when subway Stations should be drawn higher up if user enables them
    else if (index == 9 && extraMapData->draw_subway() == true && area <= 6200.0) return true;
        //If attractions are enables, they're drawn at a higher zoom
    else if (extraMapData->draw_attractions() == true && index == 23 && area <= 17100.0) return true;
        //Only hospitals building at this zoom level
    else if (area <= 6200.0 && index == 0) return true;
        //Add in EMS, Fire, and police 
    else if (area <= 2300.0 && index <= 2.0) return true;
        //Add in churches, schools, community centers and libraries
    else if (area <= 800.0 && index <= 6.0) return true;
        //Add in cafe, shops, Cinemas and fuel/transport, 
    else if (area <= 300.0 && index <= 14.0) return true;
        //Add in bank, bars, doctors
    else if (area <= 110.0 && index <= 17.0) return true;
        //Add in phones, toilets, rentals, post, and parking and attractions
    else if (area <= 40.0 && index <= 24.0) return true;
        //Add in other POIs
    else if (area <= 20.0) return true;
    else return false;
}

//Draws subways, using OSMdata class, extraMapData->set_draw_subway(bool valid) must be set to true before subway lines are drawn

void visual::draw_subways() {
    if (extraMapData->draw_subway()) { //change to extraMapData->draw_subway()
        int scale = 0;

        float area = get_visible_world().area();

        //determine subway width based on zoom level
        if (area > 17300)
            scale = 1;
        else if (area > 2240)
            scale = 3;
        else if (area > 105)
            scale = 6;
        else if (area > 14)
            scale = 12;
        else if (area > 5)
            scale = 25;
        else
            scale = 40;

        //the subway lines
        for (unsigned i = 0; i < extraMapData->get_number_of_lines(); i++) {

            for (unsigned j = 0; j < extraMapData->get_subway_line(i).wayNodes.size() - 1; j++) {
                setcolor(72, 93, 114, 255);
                int width = scale * 2;
                setlinewidth(width);
                drawline(extraMapData->get_subway_line(i).wayNodes[j].x, extraMapData->get_subway_line(i).wayNodes[j].y, extraMapData->get_subway_line(i).wayNodes[j + 1].x, extraMapData->get_subway_line(i).wayNodes[j + 1].y);
            }
        }

    }
}

//Checks if the current POI is on screen by searching through id vector 

bool visual::is_POI_on_screen(unsigned id) {

    for (unsigned i = 0; i < POIidOnScreen.size(); i++) {
        if (POIidOnScreen[i] == id) return true;
    }

    return false;
}

//Draws railways, using OSMdata class, extraMapData->set_draw_railway(bool valid) must be set to true before railway lines are drawn   

void visual::draw_railways() {
    if (extraMapData->draw_railway()) { //change to extraMapData->draw_subway()
        int scale = 0;

        float area = get_visible_world().area();

        //determine railway width based on zoom level
        if (area > 17300)
            scale = 1;
        else if (area > 2240)
            scale = 3;
        else if (area > 105)
            scale = 6;
        else if (area > 14)
            scale = 12;
        else if (area > 5)
            scale = 25;
        else
            scale = 40;


        //the subway lines
        for (unsigned i = 0; i < extraMapData->get_number_of_railways(); i++) {

            for (unsigned j = 0; j < extraMapData->get_railway(i).wayNodes.size() - 1; j++) {
                setlinestyle(DASHED, BUTT);
                setcolor(222, 204, 162, 255 * 0.9); //Railway colour
                int width = scale * 0.5;
                setlinewidth(width);
                drawline(extraMapData->get_railway(i).wayNodes[j].x, extraMapData->get_railway(i).wayNodes[j].y, extraMapData->get_railway(i).wayNodes[j + 1].x, extraMapData->get_railway(i).wayNodes[j + 1].y);
            }
        }

    }


}

void visual::build_features() {
    unsigned feature_num = getNumberOfFeatures();

    for (unsigned currentFeature = 0; currentFeature < feature_num; currentFeature++) {
        feature_data temp;
        temp.name = getFeatureName(currentFeature);

        unsigned ptCount = getFeaturePointCount(currentFeature);

        //find all points of the current feature and add to vector
        for (unsigned point = 0; point < ptCount; point++) {
            LatLon currentPointLL = getFeaturePoint(currentFeature, point);
            t_point currentPointXY;
            currentPointXY.x = mapCoord->lon_to_x(currentPointLL.lon());
            currentPointXY.y = mapCoord->lat_to_y(currentPointLL.lat());
            temp.points.push_back(currentPointXY);
        }

        //store whether feature is an open feature or closed feature and store area
        //area is used to determine what to draw first
        //larger features drawn first and smaller ones are drawn on top
        if ((temp.points[0].x == temp.points[ptCount - 1].x)
                &&(temp.points[0].y == temp.points[ptCount - 1].y)) {
            temp.isClosed = true;
            temp.area = fabs(polyArea(temp.points, ptCount));
            temp.drawAtLowZoom = 0;
        } else {
            if (temp.name.find("ocean") != std::string::npos) {
                temp.isClosed = true;
                temp.area = 10000000000; // arbitrarily large area so that it is drawn first
                temp.drawAtLowZoom = 0;
            } else {
                temp.isClosed = false;
                temp.area = 0.0;
                temp.drawAtLowZoom = 1;
            }
        }



        FeatureType type = getFeatureType(currentFeature);
        if (type == Park || type == Greenspace || type == Golfcourse) {
            temp.colour = t_color(197, 218, 198, 255); // a pale green colour
            //temp.textColour = DARKGREEN;
            //tempLabel.textColour = DARKGREEN;
        } else if (type == Beach || type == Shoreline) {
            temp.colour = KHAKI;
            //temp.textColour = FIREBRICK;
            //tempLabel.textColour = FIREBRICK; 
        } else if (type == River || type == Stream) {
            temp.colour = t_color(172, 188, 201, 255); // a light blue colour
            //temp.textColour = BLUE;
            //tempLabel.textColour = BLUE;
        } else if (type == Lake) {
            temp.colour = t_color(172, 188, 201, 255); // a light blue colour
            //temp.textColour = BLUE;
            //tempLabel.textColour = BLUE;
        } else if (type == Building) {
            temp.colour = LIGHTGREY;
            temp.drawAtLowZoom = 2;
            //temp.textColour = DARKGREY;
            //tempLabel.textColour = DARKGREY;
        } else if (type == Island) {
            temp.colour = t_color(242, 229, 212, 255); // light brownish
            //tempLabel.textColour = PURPLE;
        } else {

            temp.colour = PURPLE;
        }
        features.push_back(temp);
    }

    //sort by area (greatest to least)
    std::sort(features.begin(), features.end(), features[0]);
}

float visual::polyArea(vector<t_point>& points, unsigned numPoints) {
    float area = 0.0;
    unsigned j = numPoints - 1;

    for (unsigned i = 0; i < numPoints; i++) {

        area = area + (points[j].x + points[i].x) * (points[j].y - points[i].y);
        j = i;
    }
    return area / 2.0;
}

void visual::draw_intersection() {
    //    setcolor(WHITE);
    //    for (vector<intersection_data>::iterator iter = intersection.begin(); iter != intersection.end(); iter++) {
    //        fillarc(iter->xy.first, iter->xy.second, 0.1, 0, 360);
    //    }
}

void visual::draw_streets() {

    setlinestyle(SOLID, ROUND); //set line style to solid and round capstyle
    int scale = 0;

    float area = get_visible_world().area();

    //determine street width based on zoom level
    if (area > 17300)
        scale = 1;
    else if (area > 2240)
        scale = 3;
    else if (area > 105)
        scale = 6;
    else if (area > 14)
        scale = 12;
    else if (area > 5)
        scale = 25;
    else
        scale = 40;

    //draw minor streets
    if (area < 47000) {
        for (auto strIter = minorStreets.begin(); strIter < minorStreets.end(); strIter++) {
            setcolor(strIter->roadClassColour);
            int width = scale * strIter->width;
            setlinewidth(width);
            drawline(strIter->from, strIter->to);
        }
    }

    //draw major streets
    for (auto strIter = majorStreets.begin(); strIter < majorStreets.end(); strIter++) {
        setcolor(strIter->roadClassColour);
        int width = scale * strIter->width;
        setlinewidth(width);
        drawline(strIter->from, strIter->to);
    }

    //draw highways
    for (auto strIter = highways.begin(); strIter < highways.end(); strIter++) {

        setcolor(strIter->roadClassColour);
        int width = scale * strIter->width;
        setlinewidth(width);
        drawline(strIter->from, strIter->to);
    }
}

void visual::draw_features() {

    //line attributes for non closed features
    setlinestyle(SOLID, ROUND);
    setlinewidth(2);
    float area = get_visible_world().area();
    for (auto iter = features.begin(); iter < features.end(); iter++) {
        //only draw buildings at a higher zoom
        if (iter->drawAtLowZoom != 1 || area < 120000.0) {
            if (iter->drawAtLowZoom != 2 || area < 47000.0) {
                setcolor(iter->colour);
                if (iter->isClosed == true) {
                    t_point* fPoints;
                    fPoints = &(iter->points[0]);
                    fillpoly(fPoints, iter->points.size());
                } else {
                    for (unsigned i = 0; i < iter->points.size() - 1; i++) {

                        drawline(iter->points[i], iter->points[i + 1]);
                    }
                }
            }
        }
    }
}

//determine which street labels will be drawn for the current zoom
//needed for checking overlap with other street names and POI's

void visual::checkTBoxesOnScreen() {
    drawnTBoxes.clear();
    for (auto iter = itemLabels.begin(); iter < itemLabels.end(); iter++) {
        if (iter->name.find("unknown") == std::string::npos) {
            labels* sLabel = &(*iter);
            settextattrs(8, iter->angle);

            bool valid = true;

            if (willDrawText(iter->centre.x, iter->centre.y, iter->name, iter->streetlength, iter->streetlength, sLabel) == true && valid == true) {

                drawnTBoxes.push_back(*iter);
            }
            
            for (vector<labels>::iterator iter2 = drawnTBoxes.begin(); iter2 != drawnTBoxes.end(); iter2++) {
                if (iter2->textBox.intersects(iter->textBox, 2) == true) {
                    valid = false;
                    break;
                }
            }


            
        }
    }
    settextrotation(0);
}

void visual::draw_labels() {
    float area = get_visible_world().area();
    if (area < 16000) {
        for (auto iter = drawnTBoxes.begin(); iter < drawnTBoxes.end(); iter++) {

            //draw street labels
            if (iter->name.find("unknown") == std::string::npos) {
                settextattrs(8, iter->angle);
                setcolor(iter->textColour);
             
                drawtext(iter->centre, iter->name, iter->streetlength, iter->streetlength);
            }
        }
    }
    settextrotation(0);
}

//takes the vector of street ids given by path finding function and populates
//path struct
//convert the path from intersection 1 to intersection 2 (given in intersection ids)
//into vector of pathSSegs that can be drawn in graphics

void visual::createPath(vector<unsigned int> srcPath) {
    path.clear();
    instructions.clear();

    updatePathToDraw(srcPath);

    createInstructions();
}

void visual::updatePathToDraw(const vector<unsigned int> &srcPath) {
    for (auto iter = srcPath.begin(); iter < srcPath.end(); iter++) {
        //get endpoints of street seg and convert to xy coords
        pathSSeg temp;
        StreetSegmentInfo sInfo = getStreetSegmentInfo(*iter);

        temp.id = *iter;
        temp.name = getStreetName(sInfo.streetID);
        LatLon start = getIntersectionPosition(sInfo.from);
        LatLon end = getIntersectionPosition(sInfo.to);

        t_point tempStart;
        t_point tempEnd;

        tempStart.x = mapCoord->lon_to_x(start.lon());
        tempStart.y = mapCoord->lat_to_y(start.lat());
        tempEnd.x = mapCoord->lon_to_x(end.lon());
        tempEnd.y = mapCoord->lat_to_y(end.lat());

        cpSeg tempSeg;

        unsigned cpCount = sInfo.curvePointCount;
        //loop through curve points
        for (unsigned cp = 0; cp <= cpCount; cp++) {
            if (cpCount == 0) {
                tempSeg.from.x = tempStart.x;
                tempSeg.from.y = tempStart.y;
                tempSeg.to.x = tempEnd.x;
                tempSeg.to.y = tempEnd.y;

            } else {
                //connect to start
                if (cp == 0) {
                    tempSeg.from.x = mapCoord->lon_to_x(start.lon());
                    tempSeg.from.y = mapCoord->lat_to_y(start.lat());
                    LatLon point = getStreetSegmentCurvePoint(*iter, cp);
                    tempSeg.to.x = mapCoord->lon_to_x(point.lon());
                    tempSeg.to.y = mapCoord->lat_to_y(point.lat());
                }//connect to end
                else if (cp == cpCount) {
                    LatLon point = getStreetSegmentCurvePoint(*iter, cp - 1);
                    tempSeg.from.x = mapCoord->lon_to_x(point.lon());
                    tempSeg.from.y = mapCoord->lat_to_y(point.lat());
                    tempSeg.to.x = mapCoord->lon_to_x(end.lon());
                    tempSeg.to.y = mapCoord->lat_to_y(end.lat());
                }//connect middle points
                else {
                    LatLon pointFrom = getStreetSegmentCurvePoint(*iter, (cp - 1));
                    tempSeg.from.x = mapCoord->lon_to_x(pointFrom.lon());
                    tempSeg.from.y = mapCoord->lat_to_y(pointFrom.lat());

                    LatLon pointTo = getStreetSegmentCurvePoint(*iter, cp);
                    tempSeg.to.x = mapCoord->lon_to_x(pointTo.lon());
                    tempSeg.to.y = mapCoord->lat_to_y(pointTo.lat());
                }
            }
            temp.curvepts.push_back(tempSeg);
        }
        string classification = extraMapData->getStreetSegmentType(sInfo.wayOSMID);
        temp.classification = classification;
        path.push_back(temp);
    }
}

void visual::drawPath() {

    setlinestyle(SOLID, ROUND); //set line style to solid and round capstyle
    int scale = 0;

    float area = get_visible_world().area();

    //determine street width based on zoom level
    if (area > 17300)
        scale = 3; //scale = 1;
    else if (area > 2240)
        scale = 8; //scale = 3;
    else if (area > 105)
        scale = 9; //scale = 6;
    else if (area > 14)
        scale = 18; //scale = 12;
    else if (area > 5)
        scale = 37; //scale = 25;
    else
        scale = 60; //scale = 40;

    t_color pathColour = t_color(242, 155, 146, 255); //Light red
    setcolor(pathColour);
    setlinewidth(scale);
    //draw path
    for (auto strIter = path.begin(); strIter < path.end(); strIter++) {
        for (auto cpIter = strIter->curvepts.begin(); cpIter < strIter->curvepts.end(); cpIter++) {
            drawline(cpIter->from, cpIter->to);
        }
    }
}

void visual::drawHighlightedStreet(dirnInstruct highlightedSeg) {
    setlinestyle(SOLID, ROUND);
    int scale = 0;

    float area = get_visible_world().area();

    //determine street width based on zoom level
    if (area > 17300)
        scale = 5; //scale = 1;
    else if (area > 2240)
        scale = 10; //scale = 3;
    else if (area > 105)
        scale = 12; //scale = 6;
    else if (area > 14)
        scale = 20; //scale = 12;
    else if (area > 5)
        scale = 40; //scale = 25;
    else
        scale = 75; //scale = 40;
    t_color pathColour = t_color(211, 56, 40, 255); //red
    setcolor(pathColour);
    setlinewidth(scale);
    //draw path
    for (auto strIter = highlightedSeg.streetSec.begin(); strIter < highlightedSeg.streetSec.end(); strIter++) {
        for (auto cpIter = (*strIter)->curvepts.begin(); cpIter < (*strIter)->curvepts.end(); cpIter++) {
            drawline(cpIter->from, cpIter->to);
        }
    }
}

void visual::createInstructions() {
    for (auto iter = path.begin(); iter < path.end(); iter++) {

        dirnInstruct temp;

        temp.dist = find_street_segment_length(iter->id);

        temp.streetSec.push_back(&(*iter));

        auto nextIter = std::next(iter, 1);
        if (iter != path.end() - 1) {
            float angle = checkTurn(*iter, *nextIter);
            if (iter->name.find("unknown") == std::string::npos || fabs(angle) < 30.0) {
                //group consecutive segments with same name (dont group for turns of unknown streets (angle > 30 degrees)
                while (iter != path.end() - 1 && nextIter->name == iter->name) {
                    if (iter->name.find("unknown") == std::string::npos || fabs(angle) < 30.0) {
                        temp.dist = temp.dist + find_street_segment_length(nextIter->id);
                        angle = checkTurn(*iter, *nextIter);
                        iter++;
                        nextIter = std::next(iter, 1);
                        temp.streetSec.push_back(&(*iter));
                    } else
                        break;
                }
            }
        }

        //round distance to nearest 10
        temp.roundedDist = round(temp.dist / 10.0)*10.0;

        if (temp.roundedDist > 1000) {
            temp.roundedDist = temp.dist / 100;
            temp.roundedDist = round(temp.roundedDist);
            temp.roundedDist = temp.roundedDist / 10;
            //round double to 1 dec point in string
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(1) << temp.roundedDist;
            temp.distStr = ss.str();
            temp.distStr = temp.distStr + " km";
        } else {
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(0) << temp.roundedDist;
            temp.distStr = ss.str();
            temp.distStr = temp.distStr + " m";
        }

        if (iter == path.end() - 1) {
            temp.instructFollow = "Follow " + iter->name;
            temp.instructTurn = "to reach the destination";

        } else {

            float angle = checkTurn(*iter, *nextIter);
            //-1 = right, 1 = left, 0 = straight
            int sign;
            if (fabs(angle) < 10)
                sign = 0;
            else if (angle < 0)
                sign = -1;
            else
                sign = 1;

            string turnInstruct;
            if (sign == -1) {
                turnInstruct = "Turn right";
            } else if (sign == 1) {
                turnInstruct = "Turn left";
            } else
                turnInstruct = "Continue straight";

            string instruct1;
            //if name is unknown don't add anything onto the instruct
            //other wise add streetname. (i.e. turn left onto example street)
            if (nextIter->name.find("unknown") == std::string::npos) {
                turnInstruct = turnInstruct + " onto " + nextIter->name;
            }

            if (iter->name.find("unknown") == std::string::npos) {
                instruct1 = "Follow " + iter->name;
            } else
                instruct1 = "Follow the road";

            //special case for ramps to highways
            auto nextNextIter = std::next(nextIter, 1);
            if (iter != path.end() - 2 && nextIter->classification == "motorway_link") {
                if (nextNextIter->classification == "motorway") {
                    turnInstruct = "Take the ramp onto " + nextNextIter->name;
                }
            }
            temp.instructFollow = instruct1;
            temp.instructTurn = turnInstruct;

        }

        //this calculates the midpoint of the current street the instruction refers to
        t_point midpoint;

        midpoint.x = (temp.streetSec.front()->curvepts.front().from.x +
                temp.streetSec.back()->curvepts.back().to.x) / 2;
        midpoint.y = (temp.streetSec.front()->curvepts.front().from.y +
                temp.streetSec.back()->curvepts.back().to.y) / 2;

        temp.startOfSeg.first = midpoint.x;
        temp.startOfSeg.second = midpoint.y;

        instructions.push_back(temp);
    }
}

//determines whether you should turn right, left or go straight

float visual::checkTurn(pathSSeg& currentS, pathSSeg& nextS) {
    t_point a;
    t_point b;
    t_point c;
    //LatLon currFrom = getIntersectionPosition(currentS.from);
    //LatLon currTo = getIntersectionPosition(currentS.to);
    cpSeg currentLCp = currentS.curvepts.back();
    cpSeg currentFCp = currentS.curvepts.front();
    cpSeg nextFCp = nextS.curvepts.front();
    cpSeg nextLCp = nextS.curvepts.back();
    //LatLon nextFrom = getIntersectionPosition(nextS.from);
    //LatLon nextTo = getIntersectionPosition(nextS.to);

    //find the intersection where the two streets segments intersect
    if (currentFCp.from.x == nextFCp.from.x && currentFCp.from.y == nextFCp.from.y) {

        //a.x = mapCoord->lon_to_x(currTo.lon());
        //a.y = mapCoord->lat_to_y(currTo.lat());
        a = currentFCp.to;

        //b.x = mapCoord->lon_to_x(currFrom.lon());
        //b.y = mapCoord->lat_to_y(currFrom.lat());
        b = currentFCp.from;

        //c.x = mapCoord->lon_to_x(nextTo.lon());
        //c.y = mapCoord->lat_to_y(nextTo.lat());
        c = nextFCp.to;
    } else if (currentLCp.to.x == nextFCp.from.x && currentLCp.to.y == nextFCp.from.y) {

        //a.x = mapCoord->lon_to_x(currFrom.lon());
        //a.y = mapCoord->lat_to_y(currFrom.lat());
        a = currentLCp.from;

        //b.x = mapCoord->lon_to_x(currTo.lon());
        //b.y = mapCoord->lat_to_y(currTo.lat());
        b = currentLCp.to;

        //c.x = mapCoord->lon_to_x(nextTo.lon());
        //c.y = mapCoord->lat_to_y(nextTo.lat());
        c = nextFCp.to;
    } else if (currentLCp.to.x == nextLCp.to.x && currentLCp.to.y == nextLCp.to.y) {

        //a.x = mapCoord->lon_to_x(currFrom.lon());
        //a.y = mapCoord->lat_to_y(currFrom.lat());
        a = currentLCp.from;

        //b.x = mapCoord->lon_to_x(currTo.lon());
        //b.y = mapCoord->lat_to_y(currTo.lat());
        b = currentLCp.to;

        //c.x = mapCoord->lon_to_x(nextFrom.lon());
        //c.y = mapCoord->lat_to_y(nextFrom.lat());
        c = nextLCp.from;
    } else if (currentFCp.from.x == nextLCp.to.x && currentFCp.from.y == nextLCp.to.y) {

        //a.x = mapCoord->lon_to_x(currTo.lon());
        //a.y = mapCoord->lat_to_y(currTo.lat());
        a = currentFCp.to;

        //b.x = mapCoord->lon_to_x(currFrom.lon());
        //b.y = mapCoord->lat_to_y(currFrom.lat());
        b = currentFCp.from;

        //c.x = mapCoord->lon_to_x(nextFrom.lon());
        //c.y = mapCoord->lat_to_y(nextFrom.lat());
        c = nextLCp.from;
    }

    //create vectors and calculate 2D cross product, dot prod and angle to determine turn direction
    t_point v1 = b - a;
    t_point v2 = c - b;

    float crossprod = -v1.y * v2.x + v1.x * v2.y;
    float dotprod = v1.x * v2.x + v1.y * v2.y;
    float angle = atan2(crossprod, dotprod)* 180 / PI;
    return angle;
    //cout<<angle<<endl;
    //cout<<crossprod<<"    ";
    //-1 = right, 1 = left, 0 = straight
    //int sign = (crossprod < 0) ? -1 : (crossprod > 0 ? 1 : 0);
    /*int sign;
    if (fabs(angle)< 10)
        sign = 0;
    else if (angle < 0)
        sign = -1;
    else
        sign = 1;
    
    string instruct;
    if (sign == -1){
        instruct = "Turn right";
    }
    else if (sign == 1){
        instruct = "Turn left";
    }
    else
        instruct = "Continue straight";
    
    //string nextName = getStreetName(nextS.streetID);
    
    //if name is unknown don't add anything onto the instruct
    //other wise add streetname. (i.e. turn left onto example street)
    if (nextS.name.find("unknown") == std::string::npos) {
        instruct = instruct + " onto " + nextS.name;
    }
    
    return instruct;*/
}

void visual::drawM4Info(const std::vector<DeliveryInfo>& deliveries, const std::vector<unsigned>& depots) {

    float scale = 0;

    float area = get_visible_world().area();

    //determine location pointer radius based on zoom level
    if (area > 17300)
        scale = 10; //scale = 1;
    else if (area > 2240)
        scale = 1; //scale = 3;
    else if (area > 105)
        scale = 0.5; //scale = 6;
    else if (area > 14)
        scale = 0.1; //scale = 12;
    else if (area > 5)
        scale = 0.1; //scale = 25;
    else
        scale = 0.1; //scale = 40;

    intersection_data temp;
    int delivIndex = 0;
    for (auto iter = deliveries.begin(); iter < deliveries.end(); iter++) {
        //draw pickup
        temp.xy.first = mapCoord->lon_to_x(getIntersectionPosition(iter->pickUp).lon());
        temp.xy.second = mapCoord->lat_to_y(getIntersectionPosition(iter->pickUp).lat());
        setcolor(GREEN);
        fillarc(temp.xy.first, temp.xy.second, scale, 0, 360);

        setfontsize(10);
        setcolor(BLACK);
        string ID = std::to_string(delivIndex);
        string IDP = ID + "P"; // pickup
        string IDD = ID + "D"; // drop off
        drawtext(temp.xy.first, temp.xy.second, IDP, FLT_MAX, FLT_MAX);

        //draw drop off
        temp.xy.first = mapCoord->lon_to_x(getIntersectionPosition(iter->dropOff).lon());
        temp.xy.second = mapCoord->lat_to_y(getIntersectionPosition(iter->dropOff).lat());
        setcolor(RED);
        fillarc(temp.xy.first, temp.xy.second, scale, 0, 360);

        setcolor(BLACK);
        drawtext(temp.xy.first, temp.xy.second, IDD, FLT_MAX, FLT_MAX);

        delivIndex++;
    }

    int i = 0;
    for (auto iter = depots.begin(); iter < depots.end(); iter++) {
        //draw depot
        temp.xy.first = mapCoord->lon_to_x(getIntersectionPosition(*iter).lon());
        temp.xy.second = mapCoord->lat_to_y(getIntersectionPosition(*iter).lat());
        setcolor(BLUE);
        fillarc(temp.xy.first, temp.xy.second, scale, 0, 360);

        setfontsize(10);
        setcolor(WHITE);
        string ID = std::to_string(i);

        drawtext(temp.xy.first, temp.xy.second, ID, FLT_MAX, FLT_MAX);

        i++;
    }

}

