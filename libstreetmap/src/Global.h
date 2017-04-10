/* Author: Jack Lee
 * This header declares global variables accessible by both m1.cpp and m2.cpp
 */

#ifndef GLOBAL_H
#define GLOBAL_H

#include "ID.h"
#include "kdTree.h"
#include "treeNode.h"
#include "Coord.h"


//See class header file for description
extern ID * mapID;
extern Coord * mapCoord;
extern kdTree * kdTrees;


#endif /* GLOBAL_H */

