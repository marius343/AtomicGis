/* Author: Jack Lee,
 * Class "interface" draws and stores relevant information for the interface
 * The interface does not vary with zoom level
 */

#ifndef INTERFACE_H
#define INTERFACE_H

#include "Global.h"
#include "Global2.h"
#include "Global3.h"
#include "graphics.h"
#include "Coord.h"
#include "kdTree.h"
#include "m1.h"
#include "m3.h"
#include <sstream>
#include <string>
#include <iomanip>
#include <math.h>

#define X_OFFSET 4
#define PANEL_GAP 4
#define SHADOW_OFFSET 3
#define INDICATOR_X_OFFSET 5
#define TURN_PENALTY 15

using namespace std;

struct search_layout {
    t_bound_box search_bar;
    t_bound_box search_bar_shadow;
    t_bound_box search_button;
    t_bound_box wait_window;
    t_bound_box wait_window_shadow;
    bool searching;
    bool typing;
};

struct layer_layout {
    t_bound_box subway_button;
    t_bound_box subway_button_shadow;
};

struct result_layout {
    pair<float, float> world_xy;
    int inter_id; //-1 if poi
    string poi_name; // "\0" of intersection
    string description;
    string coord;
    string alpha;
    t_bound_box panel;
    t_bound_box panel_text;
    t_bound_box panel_coord;
    t_bound_box panel_shadow;
    t_bound_box panel_header;
    t_bound_box panel_exit;
    t_point label [5];
    t_point label_shadow [5];
    t_bound_box label_header;
};

struct highlighted_layout {
    pair<float, float> world_xy;
    int inter_id;
    string poi_name;
    string description;
    string coord;
    string number;
    string type;
    t_bound_box panel;
    t_bound_box panel_text;
    t_bound_box panel_coord;
    t_bound_box panel_shadow;
    t_bound_box panel_header;
    t_bound_box panel_exit;
    t_point label [5];
    t_point label_shadow [5];
    t_bound_box label_header;
};

struct direction_layout {
    //Search
    bool enabled;
    bool from_hold;
    vector<bool> dest_hold;
    bool searching;
    bool routing;
    pair<char, unsigned> from_position;
    vector<t_point> from_button;
    t_bound_box from_header;
    vector<t_point> from_shadow;
    vector<pair<char, unsigned>> dest_position;
    vector<vector<t_point>> dest_button;
    vector<t_bound_box> dest_header;
    vector<vector<t_point>> dest_shadow;
    t_bound_box search_button;
    t_bound_box search_shadow;
    t_bound_box wait_window;
    t_bound_box wait_window_shadow;
    //Instruction
    unsigned instruction_position;
    vector<dirnInstruct> instructions;
    t_bound_box instruction_window;
    t_bound_box instruction_traveltime;
    t_bound_box instruction_shadow;
    t_bound_box instruction_left;
    t_bound_box instruction_right;
    t_bound_box instruction_string_1;
    t_bound_box instruction_string_2;
    t_bound_box instruction_string_distance;
    t_bound_box instruction_exit;
};

struct error_layout {
    int timer;
    string prev_error;
    string error;
    t_bound_box panel;
    t_bound_box panel_shadow;
};

class interface {
private:
    //Search bar constants
    float search_bar_height, search_bar_width, wait_window_width, search_button_width;
    bool shift, alt;
    string userInput;
    string autocomplete;
    search_layout search_interface;

    //Layer button
    float screen_bottom, screen_right, layer_button_width, layer_button_height;
    layer_layout layer_interface;

    //General Panel constants
    unsigned result_panel_count;
    unsigned highlighted_panel_count;
    float label_height, label_width;
    float panel_height, panel_width, header_width, exit_width;

    //Search result section layout
    int SEARCH_RESULT_OFFSET = 30;
    t_bound_box result_title;
    t_bound_box result_title_shadow;
    vector<result_layout> result_interface;

    //Highlighted section layout
    int HIGHLIGHTED_RESULT_OFFSET = 30;
    t_bound_box highlighted_title;
    t_bound_box highlighted_title_shadow;
    vector<highlighted_layout> highlighted_interface;

    //Direction interface
    vector<unsigned> path_segments;
    pair<char, unsigned> last_from_position;
    vector<pair<char, unsigned>> last_dest_position;
    float direction_search_width, direction_search_height;
    float indicator_width, indicator_height;
    float instruction_y, instruction_height, instruction_width, instruction_button_width, instruction_string_height;
    float instruction_exit_height, instruction_exit_width;
    direction_layout direction_interface;

    //Error box
    float screen_xcenter, error_panel_width, error_panel_height;
    error_layout error_interface;

public:
    interface();
    void build_interface();
    ~interface();

    //Search interface components
    void search_interface_refresh(int scrn_x, int scrn_y, int button, int keysym);
    void search_interface_draw(bool shadow);
    void search_interface_spec(search_layout &temp, bool searching, bool typing);
    void search_interface_input_spec(char key_pressed, int keysym);

    //Layer interface components
    void layer_interface_refresh(float scrn_x, float scrn_y);
    void layer_interface_draw(bool shadow);
    void layer_interface_spec();

    //Search result components
    void result_interface_refresh(string input1, string input2, float world_x, float world_y);
    void result_interface_draw(bool shadow);
    void result_interface_add(unsigned index, string input1, string input2);
    void result_interface_remove(float scrn_x, float scrn_y);
    void result_interface_spec(result_layout &temp, pair<float, float> position, string information, unsigned panel_number);
    bool result_interface_duplicate_check(int inter_id, string poi_name);

    //Highlighted interface components
    void highlighted_interface_refresh(float world_x, float world_y);
    void highlighted_interface_draw(bool shadow);
    void highlighted_interface_add(float world_x, float world_y);
    bool highlighted_interface_remove(float scrn_x, float scrn_y);
    void highlighted_interface_spec(highlighted_layout &temp, float scrn_x, float scrn_y, unsigned panel_number, string text, string type);
    bool highlighted_interface_duplicate_check(int inter_id, string poi_name);

    //Direction interface components
    void direction_interface_refresh(float scrn_x, float scrn_y, int button);
    void direction_interface_draw(bool shadow);
    void direction_interface_spec();
    bool direction_indicator_movement(float scrn_y);
    bool direction_indicator_out_of_position();
    void direction_indicator_reset();
    dirnInstruct direction_instruction_get_current_instruction();
    bool direction_instruction_check_poi_panels();
    bool direction_instruction_check_routing();
    void direction_instruction_set_routing(vector<unsigned> path);

    //Error interface
    void error_interface_draw(bool shadow);
    void error_interface_spec(string error);

    //Helper functions
    bool check_screen_intersect(float scrn_x, float scrn_y, t_bound_box bound);
    void config_panel_offsets();
    float calc_zoom_factor(double segmentlength);
    void center_screen_at(float world_x, float world_y, float zoom);
    void adjust_poi_panel_to_closest(float world_x, float world_y);
};



#endif /* INTERFACE_H */

