
#ifndef GLOBAL3_H
#define GLOBAL3_H

#include "visual.h"
#include "pathfindingAlgorithm.h"


//Made this a global variable so I could access functions in the kdtree class
extern visual * mapGraphics;
extern pathfindingGraph* streetGraph;
extern double testAvg;
extern vector<double> testResults;

#endif /* GLOBAL3_H */

