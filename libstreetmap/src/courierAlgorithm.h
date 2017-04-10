/* Author: Jack Lee,
 * Class "courierAlgorithm" stores relevant constants and functions to solve the courier problem
 */

#ifndef COURIERALGORITHM_H
#define COURIERALGORITHM_H

#include "m1.h"
#include "m3.h"
#include "m4.h"
#include "StreetsDatabaseAPI.h"
#include "Global3.h"
#include "pathfindingAlgorithm.h"
#include <string>
#include <queue>
#include <list>
#include <algorithm>
#include <unordered_map>
#include <functional>
#include <chrono>
#include <thread>
#include <mutex>

#define TIME_LIMIT 30
#define CUT_DISTANCE_CONSTRAINT 0.8

using namespace std;

namespace std {
    template<>
    struct hash<pair<IntersectionIndex, IntersectionIndex>>
    {

        std::size_t operator()(const pair<IntersectionIndex, IntersectionIndex>& iPair) const {
            using std::size_t;
            using std::hash;
            using std::string;
            return ((hash<unsigned>()(iPair.first)^(hash<unsigned>()(iPair.second) << 1)) >> 1);
        }
    };
}

struct dPosition {
    IntersectionIndex inter_id;
    char type; //p - pickup, d - dropoff, x - depot
    unsigned number;

    dPosition(IntersectionIndex _inter_id, char _type, unsigned _number) {
        inter_id = _inter_id;
        type = _type;
        number = _number;
    }

    bool operator==(const dPosition& i) const {
        return ((i.inter_id == inter_id)&&(i.type == type)&&(i.number == number));
    }

    bool operator!=(const dPosition& i) const {
        return ((i.inter_id != inter_id) || (i.type != type) || (i.number != number));
    }
};

typedef pair<IntersectionIndex, IntersectionIndex> intersectPair;
typedef unordered_map<pair<IntersectionIndex, unsigned>, dPosition> deliveryMap;
typedef unordered_map<pair<IntersectionIndex, IntersectionIndex>, double> pathMap;
typedef unordered_map<IntersectionIndex, pair<IntersectionIndex, double>> depotMap;

struct perturb {
    double newTime;
    vector<vector <dPosition> > newPath;

    perturb(double _newTime, vector<vector <dPosition> >& _newPath) {
        newTime = _newTime;
        newPath = _newPath;
    }
};

struct HKbranch {
    double cost;
    vector<dPosition> path;
    vector<dPosition> legals;
    
    HKbranch() {
        cost = 0;
    }

    HKbranch(const double& time, const dPosition& start, const vector<dPosition>& legalmoves) {
        cost = time;
        path.push_back(start);
        legals = legalmoves;
    }

    void advance(dPosition next, const pathMap& travelSegment, const deliveryMap& PicktoDrop) {
        cost += travelSegment.at(make_pair(path.back().inter_id, next.inter_id));
        path.push_back(next);
        legals.erase(remove(legals.begin(), legals.end(), next), legals.end());
        if (next.type == 'p') legals.push_back(PicktoDrop.at(make_pair(next.inter_id, next.number)));
    }
    
    bool operator < (const HKbranch& rhs) const {
        return (cost < rhs.cost);
    }
};


class courierAlgorithm {
private:
    //Pre-optimization
    chrono::high_resolution_clock::time_point zenith;
    bool mapValid;
    float turnPenalty;
    unsigned size;
    vector<dPosition> legalMove;

    //Data structure
    deliveryMap PicktoDrop;
    pathMap travelSegment;
    depotMap DelivtoDepot;
    depotMap DepottoDeliv;

    //Current/Best path
    double prevbestTime = -1;
    double bestPathTime;
    vector<dPosition> bestPath;
    IntersectionIndex startDepot;
    IntersectionIndex endDepot;

public:
    //Constructor & destructor
    courierAlgorithm();
    courierAlgorithm(const vector<DeliveryInfo>& deliveries, const vector<unsigned>& depots, const float turn_penalty);
    ~courierAlgorithm();

    //Courier function called by the tester
    friend vector<unsigned> traveling_courier(const vector<DeliveryInfo>& deliveries, const vector<unsigned>& depots, const float turn_penalty);

    //Initialize path with greedy algorithm
    void initialize_path();
    void Held_Karp();
    dPosition find_best_startDepot();
    dPosition find_closest_legal(dPosition start);

    //Optimize path with Simulated Annealed k-opt
    void optimize_path();
    void k_opt(vector<unsigned>& cuts, int k);
    bool if_reversible(vector<dPosition>& segment);
    bool if_switchable(vector<dPosition>& segment1, vector<dPosition>& segment2);
    vector<dPosition> reverse_vector(vector<dPosition> source);
    void test_combination(perturb& perturbation, vector<vector<dPosition> >& testPath, int& k);
    void test_permutation(perturb& perturbation, vector<vector<dPosition> >& testPath, vector<vector<bool> >& switchable, int& k);
    double calc_totaltraveltime(vector<vector<dPosition> >& testPath, int& k);

    //Build result path using A* Dijkstra
    void build_path(vector<StreetSegmentIndex>& path);

    //Helper/Test functions
    HKbranch HK_subset(HKbranch src, const dPosition& next);
    void drawAlgorithmExecution(const vector<dPosition>& src);
};

#endif /* COURIERALGORITHM_H */

