/* Author: Marius Stan
 * Tree node for k-d tree
 * Class variables can be used to represent various map elements (POIs, intersections, etc.)
 */

#ifndef TREENODE_H
#define TREENODE_H

#include <iostream>
#include <stdio.h>
#include <string.h>
#include "StreetsDatabaseAPI.h"

class treeNode {
    unsigned id;
    float lat;
    float lon;
    treeNode* left;
    treeNode* right;
public:
    //Constructor & Destructor
    treeNode();
    treeNode(int new_id, double new_lat, double new_lon);
    ~treeNode();

    //Mutators
    void set_id(unsigned new_id);
    void set_left(treeNode* new_left);
    void set_right(treeNode* new_right);
    void set_lat(double new_lat);
    void set_lon(double new_lon);

    //Accessors
    double get_lat();
    double get_lon();
    unsigned get_id();
    treeNode* get_left();
    treeNode* get_right();
    void print();
};




#endif /* TREENODE_H */

