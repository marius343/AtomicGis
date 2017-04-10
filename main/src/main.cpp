#include <iostream>
#include <string>
#include "m1.h"
#include "m2.h"
#include "m3.h"
#include "m4.h"
#include "OSMDatabaseAPI.h"
#include "StreetsDatabaseAPI.h"
#include <iomanip>
#include <locale>
#include<chrono>
#include <unordered_map>

using namespace std;

int main(int argc, char** argv) {

    std::string mapName = "";
    std::string map_path;
    if (argc == 1) {
        //Use a default map
        //map_path = "/cad2/ece297s/public/maps/saint-helena.streets.bin";
        map_path = "/cad2/ece297s/public/maps/toronto_canada.streets.bin";
    } else if (argc == 2) {
        //Get the map from the command line
        map_path = argv[1];
    } else {
        //Invalid arguments
        std::cerr << "Usage: " << argv[0] << " [map_file_path]\n";
        std::cerr << "  If no map_file_path is provided a default map is loaded.\n";
        return 1;
    }
    
    //hash table for pairing name to full path
    std::unordered_map<string, string> mapPathPair;

    string maps[] = {"beijing", "cairo", "cape-town", "golden-horseshoe",
        "hamilton", "hong-kong", "iceland", "london", "moscow", "new-delhi",
        "new-york", "rio-de-janeiro", "saint-helena", "singapore", "sydney",
        "tehran", "tokyo", "toronto"};

    string paths[] = {"/cad2/ece297s/public/maps/beijing_china.streets.bin",
        "/cad2/ece297s/public/maps/cairo_egypt.streets.bin",
        "/cad2/ece297s/public/maps/cape-town_south-africa.streets.bin",
        "/cad2/ece297s/public/maps/golden-horseshoe_canada.streets.bin",
        "/cad2/ece297s/public/maps/hamilton_canada.streets.bin",
        "/cad2/ece297s/public/maps/hong-kong_china.streets.bin",
        "/cad2/ece297s/public/maps/iceland.streets.bin",
        "/cad2/ece297s/public/maps/london_england.streets.bin",
        "/cad2/ece297s/public/maps/moscow_russia.streets.bin",
        "/cad2/ece297s/public/maps/new-delhi_india.streets.bin",
        "/cad2/ece297s/public/maps/new-york_usa.streets.bin",
        "/cad2/ece297s/public/maps/rio-de-janeiro_brazil.streets.bin",
        "/cad2/ece297s/public/maps/saint-helena.streets.bin",
        "/cad2/ece297s/public/maps/singapore.streets.bin",
        "/cad2/ece297s/public/maps/sydney_australia.streets.bin",
        "/cad2/ece297s/public/maps/tehran_iran.streets.bin",
        "/cad2/ece297s/public/maps/tokyo_japan.streets.bin",
        "/cad2/ece297s/public/maps/toronto_canada.streets.bin"};

    for (int i = 0; i < 18; i++) {
        mapPathPair.insert(std::pair<string, string>(maps[i], paths[i]));
    }


    cout << "Welcome to the ATOMIC GIS!" << endl;
    cout << "Enter \"end\" to exit" << endl;
    cout << "Available Maps:" << endl;
    cout << "\n";
    for (int i = 0; i < 18; i = i + 3) {
        cout << setw(20) << left << maps[i] << setw(20) << left << maps[i + 1] << setw(20) << left << maps[i + 2] << endl;
    }

    cout << "\nAlternatively, specify a custom map path.\n\n";

    std::locale loc;
    cout << "Enter map name: ";
    cin>>mapName;
    while (mapName != "end") {
        string lmapName = mapName;
        //make name all lower case
        for (std::string::size_type i = 0; i < mapName.length(); ++i)
            lmapName[i] = std::tolower(lmapName[i], loc);

        //return iter pointing to item with key lmapName or to end if dne
        std::unordered_map<string, string>::const_iterator got = mapPathPair.find(lmapName);

        //didn't find key, assume user specified custom path
        if (got == mapPathPair.end())
            map_path = mapName;
            //found key, make map_path the paired path
        else
            map_path = got->second;

        bool load_success = load_map(map_path);
        if (!load_success) {
            std::cerr << "Failed to load map '" << map_path << "'\n";
            std::cout << "Map is not available" << endl;
            //return 2;
        } else {

            std::cout << "Successfully loaded map '" << map_path << "'\n";
//
//            std::vector<DeliveryInfo> deliveries;
//            std::vector<unsigned> depots;
//            float turn_penalty;
//            std::vector<unsigned> result_path;
//            chrono::high_resolution_clock::time_point startTime;
//            deliveries = {DeliveryInfo(70831, 51733), DeliveryInfo(64614, 49104), DeliveryInfo(86395, 90198), DeliveryInfo(68915, 1748), DeliveryInfo(77817, 36464), DeliveryInfo(7906, 86992), DeliveryInfo(85195, 97964), DeliveryInfo(24366, 11113), DeliveryInfo(62657, 72429), DeliveryInfo(44826, 33045), DeliveryInfo(6566, 86491), DeliveryInfo(91850, 60035), DeliveryInfo(36280, 57275), DeliveryInfo(97481, 1195), DeliveryInfo(68300, 60860), DeliveryInfo(77791, 15128), DeliveryInfo(69739, 8078), DeliveryInfo(68385, 108770), DeliveryInfo(75459, 100013), DeliveryInfo(67285, 100826), DeliveryInfo(16103, 47349), DeliveryInfo(44690, 30333), DeliveryInfo(989, 81045), DeliveryInfo(95265, 10843), DeliveryInfo(15804, 90041), DeliveryInfo(66262, 30923), DeliveryInfo(69352, 11124), DeliveryInfo(39741, 80709), DeliveryInfo(107067, 9155), DeliveryInfo(9727, 87561), DeliveryInfo(14679, 69801), DeliveryInfo(49350, 45035), DeliveryInfo(46806, 39996), DeliveryInfo(22340, 50841), DeliveryInfo(83735, 29110), DeliveryInfo(84125, 35208), DeliveryInfo(27827, 94577), DeliveryInfo(99880, 45391), DeliveryInfo(51371, 56818), DeliveryInfo(108031, 19488), DeliveryInfo(41567, 48318), DeliveryInfo(2266, 5702), DeliveryInfo(82273, 58171), DeliveryInfo(105662, 92219), DeliveryInfo(71511, 93073), DeliveryInfo(76398, 49443), DeliveryInfo(44689, 105495), DeliveryInfo(20806, 49666), DeliveryInfo(74292, 43619), DeliveryInfo(99932, 46063), DeliveryInfo(67216, 82420), DeliveryInfo(83186, 45317), DeliveryInfo(76221, 39119), DeliveryInfo(23822, 101787), DeliveryInfo(30252, 47210), DeliveryInfo(21261, 30777), DeliveryInfo(32640, 60454), DeliveryInfo(5132, 5321), DeliveryInfo(36493, 6120), DeliveryInfo(57286, 100769), DeliveryInfo(96376, 26250), DeliveryInfo(51728, 103554), DeliveryInfo(40558, 39484), DeliveryInfo(2725, 55441), DeliveryInfo(74490, 26052), DeliveryInfo(39219, 29203), DeliveryInfo(81784, 3342), DeliveryInfo(9007, 54902), DeliveryInfo(74175, 27755), DeliveryInfo(21235, 22648), DeliveryInfo(53109, 15924), DeliveryInfo(4863, 44723), DeliveryInfo(37191, 76062), DeliveryInfo(88341, 85383), DeliveryInfo(94683, 55071), DeliveryInfo(36870, 22371), DeliveryInfo(27010, 53986), DeliveryInfo(4562, 6641), DeliveryInfo(54939, 83587), DeliveryInfo(56452, 69532), DeliveryInfo(98765, 72206), DeliveryInfo(57705, 18710), DeliveryInfo(93630, 76760), DeliveryInfo(75607, 23835), DeliveryInfo(50531, 62871), DeliveryInfo(87576, 103929), DeliveryInfo(75119, 104926), DeliveryInfo(28917, 94863), DeliveryInfo(48014, 79421), DeliveryInfo(67807, 70881), DeliveryInfo(36713, 40596), DeliveryInfo(60356, 107976), DeliveryInfo(61155, 49594), DeliveryInfo(5404, 68375), DeliveryInfo(86324, 87156), DeliveryInfo(58034, 43156), DeliveryInfo(26541, 93050), DeliveryInfo(49782, 56269), DeliveryInfo(98805, 88285), DeliveryInfo(96318, 57406), DeliveryInfo(3909, 1063), DeliveryInfo(106669, 38602), DeliveryInfo(60559, 39417), DeliveryInfo(32310, 77756), DeliveryInfo(43215, 79559), DeliveryInfo(2719, 85706), DeliveryInfo(44546, 50836), DeliveryInfo(103884, 51061), DeliveryInfo(33352, 97894), DeliveryInfo(13982, 53927), DeliveryInfo(39136, 83642), DeliveryInfo(16454, 16407), DeliveryInfo(107566, 10186), DeliveryInfo(55738, 29559), DeliveryInfo(20433, 64640), DeliveryInfo(51018, 47704), DeliveryInfo(83418, 20963), DeliveryInfo(46690, 88588), DeliveryInfo(79849, 202), DeliveryInfo(74264, 55481), DeliveryInfo(86580, 46058), DeliveryInfo(41171, 66137), DeliveryInfo(75263, 31994), DeliveryInfo(9586, 45923), DeliveryInfo(103834, 69443), DeliveryInfo(50331, 53154), DeliveryInfo(60773, 99528), DeliveryInfo(55406, 19758), DeliveryInfo(6145, 89729), DeliveryInfo(108448, 58740), DeliveryInfo(81702, 13838), DeliveryInfo(103362, 31397), DeliveryInfo(54844, 92286), DeliveryInfo(43567, 69760), DeliveryInfo(79978, 86281), DeliveryInfo(22727, 51589), DeliveryInfo(80415, 31898), DeliveryInfo(94398, 36759), DeliveryInfo(85643, 11886), DeliveryInfo(99530, 91160), DeliveryInfo(24362, 91302), DeliveryInfo(61012, 103837), DeliveryInfo(65021, 32757), DeliveryInfo(51692, 101825), DeliveryInfo(98287, 68477), DeliveryInfo(61805, 90143), DeliveryInfo(92611, 90642), DeliveryInfo(89383, 26267), DeliveryInfo(107584, 16088), DeliveryInfo(26525, 77272), DeliveryInfo(82828, 107683), DeliveryInfo(47068, 107059), DeliveryInfo(41357, 87541), DeliveryInfo(1224, 69355), DeliveryInfo(25809, 18541), DeliveryInfo(98564, 427), DeliveryInfo(68616, 101140), DeliveryInfo(41462, 75845), DeliveryInfo(84210, 94998), DeliveryInfo(47770, 5933), DeliveryInfo(74323, 20719), DeliveryInfo(96722, 88605), DeliveryInfo(98608, 101900), DeliveryInfo(16936, 27083), DeliveryInfo(47448, 39881), DeliveryInfo(74333, 43454), DeliveryInfo(18735, 66274), DeliveryInfo(51610, 59328), DeliveryInfo(7572, 44358), DeliveryInfo(96967, 76061), DeliveryInfo(20466, 30814), DeliveryInfo(78564, 5062), DeliveryInfo(102853, 7788), DeliveryInfo(47378, 8940), DeliveryInfo(4738, 45674)};
//            depots = {14, 55539, 66199, 38064, 87930, 15037, 9781, 42243, 62859, 50192, 58489, 39444, 53709, 93336, 60037, 7650, 28282, 105649, 38892, 87051};
//            turn_penalty = 15;
//            startTime = chrono::high_resolution_clock::now();
//            result_path = traveling_courier(deliveries, depots, turn_penalty);
//
//            std::cout<<chrono::duration_cast<chrono::duration<double>>(chrono::high_resolution_clock::now() - startTime).count() << std::endl;;
//
//            return 0;



            //You can now do something with the map
            draw_map();
          
            //Clean-up the map related data structures
            std::cout << "Closing map\n";
            close_map();
        }
        cout << "\n";
        cout << "Enter map name: ";
        cin>>mapName;
    }
    cout << "Goodbye!" << endl;

    return 0;
}
