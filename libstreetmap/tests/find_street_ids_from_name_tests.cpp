/*#include <algorithm>
#include <set>
#include <unittest++/UnitTest++.h>
#include "m1.h"
#include "unit_test_util.h"
#include "StreetsDatabaseAPI.h"
#include <vector>

using ece297test::relative_error;

struct MapFixture {

    MapFixture() {
        //Load the map
        load_map("/cad2/ece297s/public/maps/toronto_canada.streets.bin");
    }

    ~MapFixture() {
        //Clean-up
        close_map();
    }
};

/*struct Find_streetIDs {
    std::vector<unsigned> expected;
    std::vector<unsigned> actual;
    unsigned expect;
};*/
/*
SUITE(Functionality) {

    TEST_FIXTURE(MapFixture, func_cornerCase_find_streetIDs) {
        //test functionality
        std::vector<unsigned> expected;
        std::vector<unsigned> actual;
        std::string streetName;

        expected = {};
        actual = find_street_ids_from_name("NoWhereStreet");
        std::sort(actual.begin(), actual.end());
        CHECK_EQUAL(expected, actual);

        expected = {5983, 18959};
        actual = find_street_ids_from_name("Asquith Avenue");
        std::sort(actual.begin(), actual.end());
        CHECK_EQUAL(expected, actual);

        expected = {2179, 15322};
        actual = find_street_ids_from_name("Aspen Avenue");
        std::sort(actual.begin(), actual.end());
        CHECK_EQUAL(expected, actual);

        expected = {13379, 16949};
        actual = find_street_ids_from_name("Ashwood Crescent");
        std::sort(actual.begin(), actual.end());
        CHECK_EQUAL(expected, actual);

        expected = {6891, 16223, 17660};
        actual = find_street_ids_from_name("Arrowhead Road");
        std::sort(actual.begin(), actual.end());
        CHECK_EQUAL(expected, actual);
    }

    TEST_FIXTURE(MapFixture, func_random_find_streetIDs) {
        std::vector<unsigned> expected;
        std::vector<unsigned> actual;
        std::string streetName;
        unsigned expect;
        unsigned act;


        for (unsigned i = 0; i < 1000000; i++) {
            expect = static_cast<unsigned> (rand() % 21222);
            streetName = getStreetName(expect);
            actual = find_street_ids_from_name(streetName);
            act = 22222; //larger than possible values
            for (auto iter = actual.begin(); iter != actual.end(); iter++) {
                if (*iter == expect) {
                    act = *iter;
                    break;
                }
            }
            CHECK_EQUAL(expect, act);
        }
    }
}

SUITE(Performance) {

    TEST_FIXTURE(MapFixture, perf_find_streetIDs_from_name) {
        ECE297_TIME_CONSTRAINT(250); //In ms
        unsigned expect;
        std::vector<unsigned> actual;
        std::string streetName;
        //perform 1 million tests with random numbers
        for (unsigned i = 0; i < 1000000; i++) {
            expect = static_cast<unsigned> (rand() % 21222);
            streetName = getStreetName(expect);
            actual = find_street_ids_from_name(streetName);
        }
    }
}
 */