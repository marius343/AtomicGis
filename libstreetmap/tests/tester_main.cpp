#include <iostream>
#include <unittest++/UnitTest++.h>
#include <algorithm>
#include <set>
#include "m1.h"
#include "unit_test_util.h"
#include "StreetsDatabaseAPI.h"
#include <string>
#include <random>

#include "m1.h"
#include "m2.h"
#include "visual.h"
#include "Global3.h"

#include <fstream>

/*
 * This is the main that drives running
 * unit tests.
 */

std::string map_name = "/cad2/ece297s/public/maps/toronto_canada.streets.bin";


double testAvg;
vector<double> testResults;


int main(int /*argc*/, char** /*argv*/) {
    bool load_success = load_map(map_name);
    //mapGraphics = new visual;
    
    testAvg = 0.0;

    if (!load_success) {
        std::cout << "ERROR: Could not load map file: '" << map_name << "'!";
        std::cout << " Subsequent tests will likely fail." << std::endl;
        //Don't abort tests, since we still want to show that all
        //tests fail.
    }

    static std::ofstream perf;
    perf.open("NewTestScores.csv", std::ios_base::app);
    
    perf<< "CutGuage = 25"<<std::endl;
    perf<< "0 - 3 toronto 4-7 london 8-11 newyork"<<endl;

    //Run the unit tests
    int num_failures = UnitTest::RunAllTests();
    
    
    testAvg = testAvg/12.0;
    cout<<testAvg<<endl;
    int i = 0;
    for (auto iter = testResults.begin();iter < testResults.end(); iter ++){
        perf<< i <<","<<*iter<<endl;
        i++;
    }
    
    perf<<testAvg<<endl;
    perf.close();
    
    

    close_map();

    return num_failures;
}