/* Author: Jack Lee, Marius Stan, Matthew Chapleau
 * Graphic aspect of the mapper
 */

#include "m1.h"
#include "m2.h"
#include "m3.h"
#include "Global4.h"
#include "visual.h"
#include "graphics.h"
#include "Coord.h"
#include "Global.h"
#include "Global3.h"
#include <chrono>
#include <thread>

#include "m4.h"
#include "courierAlgorithm.h"

//#define m4Tests

using namespace std;

void button_press(float x, float y, t_event_buttonPressed button_info);
void mouse_move(float x, float y);
void key_press(char key_pressed, int keysym);

//Draws the map which has been
//loaded with load_map()

void draw_map() {
    mapGraphics = new visual;
    mapInterface->build_interface();

    event_loop(button_press, mouse_move, key_press, draw_screen);

    delete mapGraphics;
    delete mapInterface;
}

void draw_screen() {
    //Double Buffering
    set_drawing_buffer(OFF_SCREEN);
    clearscreen();

    //Draw map
    mapGraphics->draw_features();
    mapGraphics->draw_railways();
    mapGraphics->checkTBoxesOnScreen();
    mapGraphics->draw_streets();
    if (mapInterface->direction_instruction_check_routing()) mapGraphics->drawPath();
    if (mapInterface->direction_instruction_check_routing()) mapGraphics->drawHighlightedStreet(mapInterface->direction_instruction_get_current_instruction());
    mapGraphics->draw_subways();
    mapGraphics->draw_POI();
    mapGraphics->draw_labels();

    //Draw interface
        mapInterface->highlighted_interface_draw(true);
        mapInterface->result_interface_draw(true);
        mapInterface->error_interface_draw(true);
        mapInterface->layer_interface_draw(true);
        mapInterface->direction_interface_draw(true);
        mapInterface->search_interface_draw(true);
    copy_off_screen_buffer_to_screen();
    set_drawing_buffer(ON_SCREEN);

    //TEST COURIER ALGORITHM
#ifdef m4Tests
    std::vector<DeliveryInfo> deliveries;
    std::vector<unsigned> depots;
    float turn_penalty;
    std::vector<unsigned> result_path;

    deliveries = {DeliveryInfo(34879, 389264), DeliveryInfo(291829, 231525), DeliveryInfo(129725, 383125), DeliveryInfo(195441, 389264), DeliveryInfo(89516, 394484), DeliveryInfo(89516, 76650), DeliveryInfo(89516, 310581), DeliveryInfo(286772, 17241), DeliveryInfo(394891, 31461), DeliveryInfo(66940, 347829), DeliveryInfo(343938, 41336), DeliveryInfo(89516, 130528), DeliveryInfo(343938, 83342), DeliveryInfo(422492, 66208), DeliveryInfo(135963, 409382), DeliveryInfo(143440, 49854), DeliveryInfo(64254, 293818), DeliveryInfo(36527, 138649), DeliveryInfo(242272, 96989), DeliveryInfo(219488, 257177), DeliveryInfo(343938, 83342), DeliveryInfo(335283, 31461), DeliveryInfo(89516, 272137), DeliveryInfo(150084, 187224), DeliveryInfo(116559, 394484), DeliveryInfo(25457, 17241), DeliveryInfo(143440, 147035), DeliveryInfo(105571, 114243), DeliveryInfo(69656, 138649), DeliveryInfo(343938, 17241), DeliveryInfo(360534, 394484), DeliveryInfo(105571, 23238), DeliveryInfo(343938, 257177), DeliveryInfo(89516, 257177), DeliveryInfo(274269, 24644), DeliveryInfo(105571, 69040), DeliveryInfo(89516, 83342), DeliveryInfo(403738, 118970), DeliveryInfo(400133, 158490), DeliveryInfo(86129, 158490), DeliveryInfo(231240, 121008), DeliveryInfo(59697, 259279), DeliveryInfo(2586, 158490), DeliveryInfo(152228, 158490), DeliveryInfo(260440, 76650), DeliveryInfo(264388, 234780), DeliveryInfo(62758, 318743), DeliveryInfo(143440, 390891), DeliveryInfo(254647, 286103), DeliveryInfo(143440, 155660), DeliveryInfo(89516, 138649), DeliveryInfo(33082, 247326), DeliveryInfo(249835, 314504), DeliveryInfo(429827, 362691), DeliveryInfo(343938, 158490), DeliveryInfo(89516, 343518), DeliveryInfo(179361, 204300), DeliveryInfo(354374, 310032), DeliveryInfo(143440, 168741), DeliveryInfo(336040, 40447), DeliveryInfo(343938, 394685), DeliveryInfo(143440, 17241), DeliveryInfo(319729, 394484), DeliveryInfo(143440, 17241), DeliveryInfo(143440, 25775), DeliveryInfo(11296, 338914)};
        depots = {68};
    turn_penalty = 15;

    mapGraphics->drawM4Info(deliveries, depots);
    result_path = traveling_courier(deliveries, depots, turn_penalty);
    mapGraphics->createPath(result_path);
    mapGraphics->drawPath();
#endif
}

void button_press(float x, float y, t_event_buttonPressed button_info) {
    //Search bar
    mapInterface->search_interface_refresh(xworld_to_scrn(x), yworld_to_scrn(y), button_info.button, -1);

    //Left button press
    if (button_info.button == 1) {
        //Order is important
        //Layer buttons
        mapInterface->layer_interface_refresh(xworld_to_scrn(x), yworld_to_scrn(y));
        //Highlighted panels
        mapInterface->highlighted_interface_refresh(x, y);
        //Result panels
        mapInterface->result_interface_refresh("\0", "\0", x, y);
        //Direction interface
        mapInterface->direction_interface_refresh(xworld_to_scrn(x), yworld_to_scrn(y), button_info.button);
    }
}

void mouse_move(float x, float y) {
    mapInterface->direction_interface_refresh(xworld_to_scrn(x), yworld_to_scrn(y), -1);
}

void key_press(char key_pressed, int keysym) {
    mapInterface->search_interface_input_spec(key_pressed, keysym);
    mapInterface->search_interface_refresh(-1, -1, -1, keysym);
    mapInterface->direction_interface_refresh(-1, -1, 2);
}
