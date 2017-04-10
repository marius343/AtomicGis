/*#include <iostream>
#include <string>
#include <unittest++/UnitTest++.h>

#include "unit_test_util.h"

#include "m1.h"
#include "m2.h"

TEST(TestDrawToronto) {
    bool load_success = false;
    {
        ECE297_TIME_CONSTRAINT(13000);

        load_success = load_map("/cad2/ece297s/public/maps/toronto_canada.streets.bin");

    }
    CHECK(load_success);

    draw_map();

    close_map();
}*/
