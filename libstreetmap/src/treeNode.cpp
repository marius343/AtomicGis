/* Author: Marius Stan
 * Tree node for k-d tree
 */

#include "treeNode.h"

treeNode::treeNode() {
    id = 0;
    lat = 0;
    lon = 0;
    left = NULL;
    right = NULL;
}

treeNode::treeNode(int new_id, double new_lat, double new_lon) {
    id = new_id;
    lat = new_lat;
    lon = new_lon;
    left = NULL;
    right = NULL;
}

void treeNode::set_id(unsigned new_id) {
    id = new_id;
}

void treeNode::set_left(treeNode* new_left) {
    left = new_left;
}

void treeNode::set_right(treeNode* new_right) {
    right = new_right;
}

unsigned treeNode::get_id() {
    return id;
}

treeNode* treeNode::get_left() {
    return left;
}

treeNode* treeNode::get_right() {
    return right;
}

//Function prints in order Left, Node, Right

void treeNode::print() {
    if (left != NULL) left->print();
    std::cout << id << std::endl;
    if (right != NULL) right->print();
}

void treeNode::set_lat(double new_lat) {
    lat = new_lat;
}

void treeNode::set_lon(double new_lon) {
    lon = new_lon;
}

double treeNode::get_lat() {
    return lat;
}

double treeNode::get_lon() {
    return lon;
}

//Recursive: delete the left subtree, then the right, then the node

treeNode::~treeNode() {
    if (left != NULL) delete left;
    if (right != NULL) delete right;

}

