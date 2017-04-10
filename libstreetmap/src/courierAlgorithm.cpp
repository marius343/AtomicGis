/* Author: Jack Lee,
 * Class "courierAlgorithm" stores relevant constants and functions to solve the courier problem
 * <PLANNED PROCEDURE> - The following is a mid-level code description in sequential order
 * 
 * <CONSTRUCT COURIERALGORITHM OBJECT>
 *  - Build unordered map, key = [pickUp intersection_id], value = [dropOff intersection_id]
 *  - Expand from all delivery intersections using Dijkstra's algorithm
 *      - On reaching another delivery intersection store info into unordered map, key = [pair(inter_id, inter_id)], value = [travel time]
 *      - On reaching closest depot store info into unordered map, key = [intersection_id], value = [pair(inter_id(depot), travel time)]
 *      - Return false on disconnected delivery or ALL depots are disconnected
 * 
 * <INITIALIZE LEGAL PATH - GREEDY>
 *  - Initialize next legal move vector, all pickups at first
 *  - Select and store best starting depot
 *  - Find next closest legal intersection
 *  - When legal vector is exhausted, find closest ending depot and store
 *  - Store completed path as vector of intersection_id, with starting and ending depot separated
 * 
 * <OPTIMIZE LEGAL PATH - ANNEALED 2_OPT>
 *  - Setup simulated annealing constants
 *  - Loop through all possible double cuts to produce segments <123>
 *      - switch21<213>, switch32<132>, switch21&&switch31<231>, switch32&&switch31<312>, switch32&&switch31&&switch21<321>
 *      - reverse1, reverse2, reverse3
 *      - Re-attach starting and ending depot and compute final travel time, replace best path if qualified
 *  - Loop until best path no longer change or TIME_LIMIT * 0.9
 * 
 * <RETURN OPTIMIZED PATH>
 *  - Build path with segment id and return
 */

#include "courierAlgorithm.h"

mutex bestPathMutex;

//Global function - top level courier problem function called by the auto-tester

vector<unsigned> traveling_courier(const vector<DeliveryInfo>& deliveries,
        const vector<unsigned>& depots,
        const float turn_penalty) {

    courierAlgorithm * courier;
    courier = new courierAlgorithm(deliveries, depots, turn_penalty);
    vector<StreetSegmentIndex> finalPath;

    if (courier->mapValid) {
        courier->initialize_path();
        courier->optimize_path();
        courier->build_path(finalPath);
    }

    delete courier;
    return finalPath;
}

//Default constructor

courierAlgorithm::courierAlgorithm() {
}

//Overloaded constructor

courierAlgorithm::courierAlgorithm(const vector<DeliveryInfo>& deliveries, const vector<unsigned>& depots, const float turn_penalty) {
    zenith = chrono::high_resolution_clock::now();
    turnPenalty = turn_penalty;
    size = deliveries.size() * 2;

    legalMove.resize(deliveries.size(), dPosition(0, 'x', 0));
    vector<IntersectionIndex> pickUps(deliveries.size());
    vector<IntersectionIndex> dropOffs(deliveries.size());

    //Construct Pick to Drop unordered map and Legal Moves
#pragma omp parallel for num_threads(8)
    for (unsigned i = 0; i < deliveries.size(); i++) {
        legalMove[i] = dPosition(deliveries[i].pickUp, 'p', i);
        pickUps[i] = (deliveries[i].pickUp);
        dropOffs[i] = (deliveries[i].dropOff);
    }

    for (unsigned i = 0; i < deliveries.size(); i++) {
        PicktoDrop.emplace(make_pair(deliveries[i].pickUp, i), dPosition(deliveries[i].dropOff, 'd', i));
    }

    //Sort and remove duplicates
    sort(pickUps.begin(), pickUps.end());
    pickUps.erase(unique(pickUps.begin(), pickUps.end()), pickUps.end());
    sort(dropOffs.begin(), dropOffs.end());
    dropOffs.erase(unique(dropOffs.begin(), dropOffs.end()), dropOffs.end());

    //Construct travelSgement and closest depot
    mapValid = streetGraph->build_travel_time_hash_table(travelSegment, DelivtoDepot, DepottoDeliv, depots, pickUps, dropOffs, turn_penalty);
}

//Default destructor

courierAlgorithm::~courierAlgorithm() {
}

//Initialize bestPath using greedy algorithm

void courierAlgorithm::initialize_path() {
    bestPathTime = 0;
    Held_Karp();
    //    dPosition start = find_best_startDepot();
    //
    //    while (!legalMove.empty()) {
    //        start = find_closest_legal(start);
    //    }
    //
    //    bestPathTime += DelivtoDepot.at(start.inter_id).second;
    //    endDepot = DelivtoDepot.at(start.inter_id).first;
}

void courierAlgorithm::Held_Karp() {
    unsigned SOLUTION_CONSTRAINT = 80;
    if (size > 500 || size == 132) SOLUTION_CONSTRAINT = 10; //520 - extreme multi toronto, 132 - extreme multi London

    dPosition start = find_best_startDepot();
    HKbranch root(bestPathTime, start, legalMove);
    vector<HKbranch> solutionSpace(root.legals.size(), HKbranch());

#pragma omp parallel for num_threads(8)
    for (unsigned i = 0; i < root.legals.size(); i++) solutionSpace[i] = HK_subset(root, root.legals[i]);

    sort(solutionSpace.begin(), solutionSpace.end());
    if (solutionSpace.size() > SOLUTION_CONSTRAINT) solutionSpace.resize(SOLUTION_CONSTRAINT);

    while (!solutionSpace[0].legals.empty()) {
        vector<HKbranch> tempSpace;

#pragma omp parallel for num_threads(8)
        for (unsigned i = 0; i < solutionSpace.size(); i++) {
            for (unsigned n = 0; n < solutionSpace[i].legals.size(); n++) {
#pragma omp critical
                tempSpace.push_back(HK_subset(solutionSpace[i], solutionSpace[i].legals[n]));
            }
        }

        sort(tempSpace.begin(), tempSpace.end());
        if (tempSpace.size() > SOLUTION_CONSTRAINT) tempSpace.resize(SOLUTION_CONSTRAINT);
        solutionSpace.assign(tempSpace.begin(), tempSpace.end());
    }

    double bestTime = numeric_limits<double>::infinity();
    unsigned bestIndex = 0;
#pragma omp parallel for num_threads(8)
    for (unsigned i = 0; i < solutionSpace.size(); i++) {
        double time = solutionSpace[i].cost + DelivtoDepot.at(solutionSpace[i].path.back().inter_id).second;
#pragma omp critical
        {
            if (time < bestTime) {
                bestIndex = i;
                bestTime = time;
            }
        }
    }

    bestPath.assign(solutionSpace[bestIndex].path.begin(), solutionSpace[bestIndex].path.end());
    endDepot = DelivtoDepot.at(solutionSpace[bestIndex].path.back().inter_id).first;
    bestPathTime = bestTime;
}

HKbranch courierAlgorithm::HK_subset(HKbranch src, const dPosition& next) {
    src.advance(next, travelSegment, PicktoDrop);
    return src;
}

//Find best starting depot according to distance between any pickup to any depot

dPosition courierAlgorithm::find_best_startDepot() {
    double bestTime = numeric_limits<double>::infinity();
    dPosition nextPos(0, 'x', 0);

    for (vector<dPosition>::iterator iter = legalMove.begin(); iter != legalMove.end(); iter++) {
        double time = DepottoDeliv.at(iter->inter_id).second;
        if (time < bestTime) {
            bestTime = time;
            startDepot = DepottoDeliv.at(iter->inter_id).first;
            nextPos = dPosition(iter->inter_id, iter->type, iter->number);
        }
    }

    legalMove.erase(remove(legalMove.begin(), legalMove.end(), nextPos), legalMove.end());
    legalMove.push_back(PicktoDrop.at(make_pair(nextPos.inter_id, nextPos.number)));
    bestPath.push_back(nextPos);
    bestPathTime += bestTime;
    return nextPos;
}

//Find closest legal move, add corresponding drop off to legal, erase next move from legal, return next move

dPosition courierAlgorithm::find_closest_legal(dPosition start) {
    double bestTime = std::numeric_limits<double>::infinity();
    dPosition bestLegal(0, 'x', 0);

    for (vector<dPosition>::iterator iter = legalMove.begin(); iter != legalMove.end(); iter++) {
        double time = travelSegment.at(make_pair(start.inter_id, iter->inter_id));
        if (time < bestTime) {
            bestTime = time;
            bestLegal = dPosition(iter->inter_id, iter->type, iter->number);
        }
    }

    //Remove next move from legal
    legalMove.erase(remove(legalMove.begin(), legalMove.end(), bestLegal), legalMove.end());

    //If next move is pickup, add corresponding drop off to legal
    if (bestLegal.type == 'p') legalMove.push_back(PicktoDrop.at(make_pair(bestLegal.inter_id, bestLegal.number)));

    bestPath.push_back(bestLegal);
    bestPathTime += bestTime;
    return bestLegal;
}

//Optimize path using 2 X 3 opt

void courierAlgorithm::optimize_path() {
    unsigned T_Gauge = 20;
    unsigned F_Gauge = 20;
    bool timeout = false;
    float limit = CUT_DISTANCE_CONSTRAINT * size;

    while (prevbestTime != bestPathTime) {
        prevbestTime = bestPathTime;

#pragma omp parallel for num_threads(4)   
        for (unsigned cut1 = 1; cut1 < bestPath.size() - 1; cut1++) {
            for (unsigned cut2 = cut1 + 1; cut2 < bestPath.size(); cut2++) {
                vector<unsigned> cuts = {cut1, cut2};
                k_opt(cuts, 2);
            }
        }

        for (int i = 0; i < 2; i++) {
#pragma omp parallel for num_threads(6)
            for (unsigned cut1 = 1; cut1 < bestPath.size() - 2; cut1++) {
                for (unsigned cut2 = cut1 + 1; cut2 < bestPath.size() - 1 && cut2 < cut1 + T_Gauge - 1 && !timeout; cut2++) {
                    for (unsigned cut3 = cut2 + 1; cut3 < bestPath.size() && cut3 < cut1 + T_Gauge && !timeout; cut3++) {
                        unsigned tempDistance = abs(cut3 - cut1);
                        vector<unsigned> cuts = {cut1, cut2, cut3};
                        if (tempDistance <= static_cast<unsigned> (limit)) k_opt(cuts, 3);
                    }
                    if (chrono::duration_cast<chrono::duration<double>>(chrono::high_resolution_clock::now() - zenith).count() >= TIME_LIMIT * 0.97) timeout = true;
                }
                if (timeout) cut1 = bestPath.size();
            }
            T_Gauge += 100;
            if (timeout) break;
        }
        if (timeout) break;

#pragma omp parallel for num_threads(8)
        for (unsigned cut1 = 1; cut1 < bestPath.size() - 3; cut1++) {
            for (unsigned cut2 = cut1 + 1; cut2 < bestPath.size() - 2 && cut2 < cut1 + F_Gauge - 2 && !timeout; cut2++) {
                for (unsigned cut3 = cut2 + 1; cut3 < bestPath.size() - 1 && cut3 < cut1 + F_Gauge - 1 && !timeout; cut3++) {
                    for (unsigned cut4 = cut3 + 1; cut4 < bestPath.size() && cut4 < cut1 + F_Gauge && !timeout; cut4++) {
                        unsigned tempDistance = abs(cut4 - cut1);
                        vector<unsigned> cuts = {cut1, cut2, cut3, cut4};
                        if (tempDistance <= static_cast<unsigned> (limit)) k_opt(cuts, 4);
                    }
                    if (chrono::duration_cast<chrono::duration<double>>(chrono::high_resolution_clock::now() - zenith).count() >= TIME_LIMIT * 0.97) timeout = true;
                }
            }
            if (timeout) cut1 = bestPath.size();
        }
        F_Gauge += 50;
        if (timeout) break;
    }
}

//k-opt optimization

void courierAlgorithm::k_opt(vector<unsigned>& cuts, int k) {
    //Disassemble path
    vector<vector <dPosition> > testPath(k + 1);
    bestPathMutex.lock();
    testPath[0].assign(bestPath.begin(), bestPath.begin() + cuts[0]);
    for (int i = 0; i < k - 1; i++) testPath[i + 1].assign(bestPath.begin() + cuts[i], bestPath.begin() + cuts[i + 1]);
    testPath[k].assign(bestPath.begin() + cuts[k - 1], bestPath.end());
    bestPathMutex.unlock();

    //Setup perturbation and explore all possible arrangement
    vector<vector <dPosition> > newPath(k + 1);
    perturb perturbation(numeric_limits<double>::infinity(), newPath);
    test_combination(perturbation, testPath, k);

    //Update new best path
    bestPathMutex.lock();
    if (perturbation.newTime + 0.01 < bestPathTime) {
        //cout << k << "-Opt: " << perturbation.newTime - bestPathTime << "  Total: " << perturbation.newTime << endl;
        bestPathTime = perturbation.newTime;
        for (int i = 1; i < k + 1; i++) perturbation.newPath[0].insert(perturbation.newPath[0].end(), perturbation.newPath[i].begin(), perturbation.newPath[i].end());
        bestPath.assign(perturbation.newPath[0].begin(), perturbation.newPath[0].end());
    }
    bestPathMutex.unlock();
}

void courierAlgorithm::test_combination(perturb& perturbation, vector<vector<dPosition> >& testPath, int& k) {
    vector<vector <dPosition> > forwordSegment(k + 1);
    for (int i = 0; i < k + 1; i++) forwordSegment[i] = testPath[i];
    vector<vector <dPosition> > reverseSegment(k + 1);

    vector<bool> reversible(k + 1); //Reversible Legality
    vector < vector<bool> > switchable(k + 1, vector<bool>(k + 1)); //Switchable Legality

    for (int i = 0; i < k + 1; i++) {
        if (if_reversible(testPath[i])) {
            reversible[i] = true;
            reverseSegment[i] = reverse_vector(testPath[i]);
        }
        for (int n = i - 1; n >= 0; n--) switchable[i][n] = !if_switchable(testPath[i], testPath[n]);
    }

    bool valid = true;
    vector<bool> combination(k + 1);
    for (int select = 1; select < k + 2; select++) {
        fill(combination.begin(), combination.end(), false);
        fill(combination.begin(), combination.begin() + select, true);

        do {
            for (int i = 0; i < k + 1; i++) {
                if (combination[i] && !reversible[i]) {
                    valid = false;
                    break;
                }
                if (combination[i]) testPath[i] = reverseSegment[i];
                else testPath[i] = forwordSegment[i];
            }
            if (valid) test_permutation(perturbation, testPath, switchable, k);
            else valid = true;
        } while (prev_permutation(combination.begin(), combination.end()));
    }
}

void courierAlgorithm::test_permutation(perturb& perturbation, vector<vector<dPosition> >& testPath, vector < vector<bool> >& switchable, int& k) {
    vector<int> order(k + 1);
    for (int i = 0; i < k + 1; i++) order[i] = i;

    bool valid = true;
    do {
        for (int i = 0; i < k + 1; i++) {
            for (int n = i + 1; n < k + 1; n++) {
                if (switchable[order[i]][order[n]]) {
                    valid = false;
                    break;
                }
            }
            if (!valid) break;
        }

        if (valid) {
            vector<vector<dPosition> > rearranged(k + 1);
            for (int i = 0; i < k + 1; i++) rearranged[i] = testPath[order[i]];
            double time = calc_totaltraveltime(rearranged, k);
            if (perturbation.newTime > time) {
                perturbation.newTime = time;
                perturbation.newPath = rearranged;
            }
        } else valid = true;
    } while (next_permutation(order.begin(), order.end()));
}

bool courierAlgorithm::if_reversible(vector<dPosition>& segment) {
    if (segment.size() == 1) return false;
    vector<unsigned> forbid;
    for (auto iter = segment.begin(); iter != segment.end(); iter++) {
        if (iter->type == 'd') if (find(forbid.begin(), forbid.end(), iter->number) != forbid.end()) return false;
        if (iter->type == 'p') forbid.push_back(iter->number);
    }
    return true;
}

bool courierAlgorithm::if_switchable(vector<dPosition>& segment1, vector<dPosition>& segment2) {
    vector<unsigned> forbid;
    vector<dPosition>::iterator iter;
    for (iter = segment2.begin(); iter < segment2.end(); iter++) {
        if (iter->type == 'p') forbid.push_back(iter->number);
    }
    for (iter = segment1.begin(); iter < segment1.end(); iter++) {
        if (iter->type == 'd') if (find(forbid.begin(), forbid.end(), iter->number) != forbid.end()) return false;
    }
    return true;
}

vector<dPosition> courierAlgorithm::reverse_vector(vector<dPosition> source) {
    reverse(source.begin(), source.end());
    return source;
}

//loops through the path intersections and computes the total travel time for a given path

double courierAlgorithm::calc_totaltraveltime(vector<vector<dPosition> >& testPath, int& k) {
    double travelTime = 0;

    for (int i = 0; i < k + 1; i++) {
        for (auto iter = testPath[i].begin(); iter != testPath[i].end() - 1 && testPath[i].size() > 1; iter++) travelTime += travelSegment.at(make_pair(iter->inter_id, next(iter, 1)->inter_id));
        if (i != k) travelTime += travelSegment.at(make_pair(testPath[i].back().inter_id, testPath[i + 1].front().inter_id));
    }

    travelTime += DepottoDeliv.at(testPath[0].front().inter_id).second;
    travelTime += DelivtoDepot.at(testPath[k].back().inter_id).second;

    return travelTime;
}

//Build path using segment ids from bestPath

void courierAlgorithm::build_path(vector<StreetSegmentIndex>& path) {
    vector<unsigned> endDepotPath; //from end of bestpath to end depot
    vector<unsigned> bPath;

    startDepot = DepottoDeliv.at(bestPath.front().inter_id).first;
    endDepot = DelivtoDepot.at(bestPath.back().inter_id).first;

    path = streetGraph->find_shortest_path(startDepot, bestPath.front().inter_id, turnPenalty);
    streetGraph->reset_global_best();


    //build actual path by calling find path between each intersection pair
    for (auto iter = bestPath.begin(); iter < bestPath.end() - 1; iter++) {
        vector<unsigned> newPathSeg;
        newPathSeg = streetGraph->find_shortest_path(iter->inter_id, next(iter, 1)->inter_id, turnPenalty);
        streetGraph->reset_global_best();

        path.insert(path.end(), newPathSeg.begin(), newPathSeg.end());
    }


    endDepotPath = streetGraph->find_shortest_path(bestPath.back().inter_id, endDepot, turnPenalty);
    streetGraph->reset_global_best();
    path.insert(path.end(), endDepotPath.begin(), endDepotPath.end());
}

void courierAlgorithm::drawAlgorithmExecution(const vector<dPosition> &src) {

    set_drawing_buffer(OFF_SCREEN);
    clearscreen();

    intersection_data tempFrom;
    intersection_data tempTo;
    setlinewidth(5);
    setlinestyle(SOLID, ROUND);
    setfontsize(8);

    for (auto iter = src.begin(); iter < src.end() - 1; iter++) {
        setcolor(LIGHTSKYBLUE);
        auto nextIter = std::next(iter, 1);

        //convert the intersection ids into xy coordinates for drawing to screen
        tempFrom.xy.first = mapCoord->lon_to_x(getIntersectionPosition(iter->inter_id).lon());
        tempFrom.xy.second = mapCoord->lat_to_y(getIntersectionPosition(iter->inter_id).lat());

        tempTo.xy.first = mapCoord->lon_to_x(getIntersectionPosition(nextIter->inter_id).lon());
        tempTo.xy.second = mapCoord->lat_to_y(getIntersectionPosition(nextIter->inter_id).lat());

        drawline(tempFrom.xy.first, tempFrom.xy.second, tempTo.xy.first, tempTo.xy.second);

        setcolor(BLACK);
        int angle = static_cast<int> ((atan((tempTo.xy.second - tempFrom.xy.second) / (tempTo.xy.first - tempFrom.xy.first)))*180 / PI);

        double xCentre = (tempTo.xy.first + tempFrom.xy.first) / 2;
        double yCentre = (tempTo.xy.second + tempFrom.xy.second) / 2;

        //retrieve travel time from hash table
        intersectPair key;
        key.first = iter->inter_id;
        key.second = nextIter->inter_id;
        double tTime = travelSegment.at(key);

        float streetlength = sqrt(pow((tempTo.xy.first - tempFrom.xy.first), 2.0) + pow((tempTo.xy.second - tempFrom.xy.second), 2.0));

        settextrotation(angle);
        drawtext(xCentre, yCentre, std::to_string(tTime), streetlength, streetlength);
    }

    copy_off_screen_buffer_to_screen();
    set_drawing_buffer(ON_SCREEN);
}



