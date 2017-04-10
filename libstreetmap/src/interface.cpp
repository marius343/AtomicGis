/* Author: Jack Lee,
 * Class "interface" draws and stores relevant information for the interface
 * The interface does not vary with zoom level
 */

#include "interface.h"
#include "visual.h"

interface::interface() {
    build_interface();
}

void interface::build_interface() {
    //Search bar constants
    shift = false;
    search_bar_height = (0 - get_visible_screen().get_height()) / 25;
    search_bar_width = (get_visible_screen().get_width() / 3) * 1.1;
    search_button_width = (get_visible_screen().get_width() / 3) * 0.10;
    wait_window_width = get_visible_screen().get_width() / 8;

    //Layer button constants
    screen_bottom = (0 - get_visible_screen().get_height()) * 0.99 + 20;
    screen_right = get_visible_screen().get_width() * 0.99 + 110;
    layer_button_height = get_visible_screen().get_width() * 0.02;
    layer_button_width = get_visible_screen().get_width() / 14 + 15;

    //Search results/ Highlighted interface constants
    result_panel_count = 0;
    highlighted_panel_count = 0;
    label_height = (0 - get_visible_screen().get_height()) / 27;
    label_width = label_height / 1.75;
    panel_height = (0 - get_visible_screen().get_height()) / 15;
    panel_width = get_visible_screen().get_width() / 3 + 4;
    header_width = panel_width * 0.05;
    exit_width = panel_width * 0.04;

    //Error box
    error_interface.error.clear();
    error_interface.prev_error.clear();
    error_interface.timer = 0;
    error_panel_width = get_visible_screen().get_width() / 2.3;
    error_panel_height = (0 - get_visible_screen().get_height()) / 31;

    //Direction interface constants
    direction_interface.instruction_position = 0;
    direction_search_width = search_bar_width * 0.10;
    direction_search_height = search_bar_height;
    indicator_width = get_visible_screen().get_width() / 20;
    indicator_height = (0 - get_visible_screen().get_height()) / 31;
    instruction_string_height = (0 - get_visible_screen().get_height()) / 32;
    instruction_height = 3 * instruction_string_height;
    instruction_button_width = header_width;
    instruction_width = get_visible_screen().get_width() / 2.5;
    instruction_exit_height = instruction_height * 0.6;
    instruction_exit_width = exit_width;
    instruction_y = screen_bottom - error_panel_height - instruction_height - (0 - get_visible_screen().get_height()) / 45;
    direction_interface.dest_hold.resize(1);
    direction_interface.dest_hold[0] = false;
    direction_interface.from_hold = false;
    direction_interface.enabled = false;
    direction_interface.searching = false;
    direction_interface.routing = false;
    pair<char, unsigned> temp_pos(' ', -1);
    last_from_position = temp_pos;
    last_dest_position.resize(1);
    last_dest_position[0] = temp_pos;
    direction_interface.from_position = temp_pos;
    direction_interface.dest_position.resize(1);
    direction_interface.dest_position[0] = temp_pos;
    direction_interface.from_button.resize(5);
    direction_interface.from_shadow.resize(5);
    direction_interface.dest_button.resize(1);
    direction_interface.dest_button[0].resize(5);
    direction_interface.dest_shadow.resize(1);
    direction_interface.dest_shadow[0].resize(5);
    direction_interface.dest_header.resize(1);

    search_interface_spec(search_interface, false, false);
    layer_interface_spec();
    direction_interface_spec();
}

interface::~interface() {
}

//Refresh search interface based on events

void interface::search_interface_refresh(int scrn_x, int scrn_y, int button, int keysym) {
    //Left click
    if (button == 1 || button == -1) {
        //Pressed search button
        if (check_screen_intersect(scrn_x, scrn_y, search_interface.search_button) || keysym == 65293) {
            if (userInput != autocomplete && !autocomplete.empty()) {
                userInput = autocomplete;
            } else {
                search_interface_spec(search_interface, true, true);
                search_interface_draw(true);

                stringstream Input(userInput);
                if (userInput.find(",", 0) != string::npos) {
                    //Find intersection search
                    string street1, street2;
                    getline(Input, street1, ',');
                    getline(Input, street2, ',');
                    while (street1.front() == ' ' && !street1.empty()) street1.erase(street1.begin());
                    while (street2.front() == ' ' && !street2.empty()) street2.erase(street2.begin());
                    while (street1.back() == ' ' && !street1.empty()) street1.pop_back();
                    while (street2.back() == ' ' && !street2.empty()) street2.pop_back();

                    result_interface_refresh(street1, street2, 0, 0);
                } else {
                    //POI search
                    string input1;
                    getline(Input, input1);
                    while (input1.front() == ' ' && !input1.empty()) input1.erase(input1.begin());
                    while (input1.back() == ' ' && !input1.empty()) input1.pop_back();

                    result_interface_refresh(input1, "\0", 0, 0);
                }
            }
            search_interface_spec(search_interface, false, true);
            draw_screen();
            return;
        }

        //Pressed search bar
        if (check_screen_intersect(scrn_x, scrn_y, search_interface.search_bar)) {
            search_interface_spec(search_interface, false, true);
            search_interface_draw(false);
            return;
        }

        if (button != -1) {
            //Pressed anywhere else
            search_interface_spec(search_interface, false, true);
            search_interface_draw(false);
        }
    }

    //Right click
    if (button == 3) {
        search_interface_spec(search_interface, false, false);
        search_interface_draw(false);
    }

}

//Draw search bar

void interface::search_interface_draw(bool shadow) {
    set_coordinate_system(GL_SCREEN);

    //Respec
    search_interface_spec(search_interface, search_interface.searching, search_interface.typing);

    //Draw shadows
    if (shadow) {
        setcolor(52, 73, 94, 255 * 0.4); //SHADOW
        fillrect(search_interface.search_bar_shadow);
    }

    //Draw is user is typing
    if (search_interface.typing) {
        //Draw search bar
        setcolor(236, 240, 241, 255);
        fillrect(search_interface.search_bar);
    } else {
        //Draw search bar
        setcolor(206, 210, 211, 255);
        fillrect(search_interface.search_bar);
    }

    //Draw search button and text
    setcolor(36, 47, 62, 255);
    fillrect(search_interface.search_button);
    /*
    string file_temp = "Icons/interface_search.png";
    const char * file = file_temp.c_str();
    Surface icon = load_png_from_file(file);
    draw_surface(icon, search_interface.search_button.left() + search_interface.search_button.get_width() / 4, search_interface.search_button.top());
     */
    setcolor(236, 240, 241, 255);
    settextattrs(20, 0);
    drawtext_in(search_interface.search_button, "GO", 100);

    //Draw wait window if active
    if (search_interface.searching) {
        if (shadow) {
            setcolor(52, 73, 94, 255 * 0.4); //SHADOW
            fillrect(search_interface.wait_window_shadow);
        }
        setcolor(36, 47, 62, 255);
        fillrect(search_interface.wait_window);

        setcolor(236, 240, 241, 255);
        setfontsize(11);
        drawtext_in(search_interface.wait_window, "Traversing data tree ...", 100);
    }

    if (!userInput.empty()) {
        //Draw typed text
        setcolor(152, 173, 194, 255); //autocomplete
        setfontsize(11);
        drawtext_in_left(search_interface.search_bar, autocomplete, 100);

        setcolor(52, 73, 94, 255);
        drawtext_in_left(search_interface.search_bar, userInput, 100);
    }

    set_coordinate_system(GL_WORLD);
}

//build structure search_layout & layer button layout & error box layout

void interface::search_interface_spec(search_layout& temp, bool searching, bool typing) {
    //t_bound_box construction: left, bottom, right, top
    //Search bar
    t_bound_box temp_search_bar(X_OFFSET, search_bar_height, X_OFFSET + search_bar_width - search_button_width, PANEL_GAP);
    temp.search_bar = temp_search_bar;

    t_bound_box temp_search_bar_shadow(X_OFFSET + SHADOW_OFFSET, search_bar_height + SHADOW_OFFSET, X_OFFSET + search_bar_width + SHADOW_OFFSET, PANEL_GAP + SHADOW_OFFSET);
    temp.search_bar_shadow = temp_search_bar_shadow;

    if (searching) {
        temp.searching = true;

        t_bound_box temp_wait_window(X_OFFSET * 2 + direction_search_width + search_bar_width, search_bar_height, X_OFFSET * 2 + search_bar_width + direction_search_width + wait_window_width, PANEL_GAP);
        temp.wait_window = temp_wait_window;

        t_bound_box temp_wait_window_shadow(X_OFFSET * 2 + direction_search_width + search_bar_width + SHADOW_OFFSET, search_bar_height + SHADOW_OFFSET, X_OFFSET * 2 + search_bar_width + direction_search_width + wait_window_width + SHADOW_OFFSET, PANEL_GAP + SHADOW_OFFSET);
        temp.wait_window_shadow = temp_wait_window_shadow;

        //t_bound_box temp_search_button(X_OFFSET + search_bar_width - search_button_width + SHADOW_OFFSET / 2, search_bar_height + SHADOW_OFFSET / 2, X_OFFSET + search_bar_width + SHADOW_OFFSET / 2, PANEL_GAP + SHADOW_OFFSET / 2);
        //temp.search_button = temp_search_button;

    } else {
        temp.searching = false;

        t_bound_box temp_search_button(X_OFFSET + search_bar_width - search_button_width, search_bar_height, X_OFFSET + search_bar_width, PANEL_GAP);
        temp.search_button = temp_search_button;
    }

    if (typing) {
        temp.typing = true;
    } else {
        temp.typing = false;
        userInput.clear();
    }
}

//Types Text in the search bar

void interface::search_interface_input_spec(char key_pressed, int keysym) {
    if (search_interface.typing && keysym != 65293) {
        //Search-Input
        if (keysym == 0xffe1) shift = true;
        else {
            //Backspace
            if (key_pressed == '\b' && !userInput.empty()) {
                userInput.pop_back();
            }

            if (userInput.size() <= 45 && key_pressed != '\b') {
                if (!(userInput.empty() && key_pressed == ' ')) {
                    if (shift) {
                        //Capitalize
                        userInput = userInput + static_cast<char> (toupper(key_pressed));
                        shift = false;
                    } else {
                        userInput = userInput + key_pressed;
                    }
                }
            }
        }

        //Auto-complete
        if (!userInput.empty()) {
            stringstream Input(userInput);
            string street1, street2;
            getline(Input, street1, ',');
            getline(Input, street2, ',');
            while (street1.front() == ' ' && !street1.empty()) street1.erase(street1.begin());
            while (street2.front() == ' ' && !street2.empty()) street2.erase(street2.begin());
            while (street1.back() == ' ' && !street1.empty()) street1.pop_back();
            while (street2.back() == ' ' && !street2.empty()) street2.pop_back();
            if (!street1.empty() && street2.empty()) {
                //First street
                autocomplete = mapID->search_autocomplete(street1);
                if (userInput.find(",", 0) != string::npos) autocomplete.clear();
            } else if (!street2.empty()) {
                //Second Street
                size_t pos = 0;
                size_t last;
                while (pos != string::npos) {
                    last = pos;
                    pos = userInput.find(street2, pos + 1);
                }
                autocomplete = userInput.substr(0, last) + mapID->search_autocomplete(street2);
            }
        } else {
            autocomplete.clear();
        }

        set_coordinate_system(GL_SCREEN);

        //Redraw search bar
        setcolor(236, 240, 241, 255);
        fillrect(search_interface.search_bar);

        if (!userInput.empty()) {
            //Draw typed text
            setcolor(152, 173, 194, 255); //autocomplete
            setfontsize(11);
            drawtext_in_left(search_interface.search_bar, autocomplete, 100);

            setcolor(52, 73, 94, 255);
            drawtext_in_left(search_interface.search_bar, userInput, 100);
        }

        set_coordinate_system(GL_WORLD);
    }
}

//Refresh layer interface variable according to events

void interface::layer_interface_refresh(float scrn_x, float scrn_y) {
    //Clicked on subway button
    if (check_screen_intersect(scrn_x, scrn_y, layer_interface.subway_button)) {
        extraMapData->drawSubway = !extraMapData->drawSubway;
        layer_interface_draw(false);
    }
}

//Draw layer buttons

void interface::layer_interface_draw(bool shadow) {
    screen_bottom = (0 - get_visible_screen().get_height()) * 0.99 + 20;
    screen_right = get_visible_screen().get_width() * 0.99 + 110;
    layer_interface_spec();

    set_coordinate_system(GL_SCREEN);

    if (shadow) {
        setcolor(52, 73, 94, 255 * 0.4); //SHADOW
        fillrect(layer_interface.subway_button_shadow);
    }
    if (!extraMapData->drawSubway) {

        setcolor(236, 240, 241, 255);
        fillrect(layer_interface.subway_button);

        setcolor(52, 73, 94, 255);
        setfontsize(10);
        drawtext_in(layer_interface.subway_button, "SUBWAY LAYER", 100);

    } else if (extraMapData->drawSubway) {

        setcolor(52, 73, 94, 255); //PANEL COLOR - CLOUD
        fillrect(layer_interface.subway_button);

        setcolor(236, 240, 241, 255); //DESCRIPTION - WET ASPHALT
        setfontsize(10);
        drawtext_in(layer_interface.subway_button, "SUBWAY LAYER", 100);
    }

    set_coordinate_system(GL_WORLD);
}

//Build and adjust layer layout structure

void interface::layer_interface_spec() {
    //Subway Layer
    if (!extraMapData->drawSubway) {
        t_bound_box temp_subway_button(screen_right - layer_button_width, screen_bottom, screen_right, screen_bottom - layer_button_height);
        layer_interface.subway_button = temp_subway_button;

        t_bound_box temp_subway_button_shadow(screen_right - layer_button_width + SHADOW_OFFSET, screen_bottom + SHADOW_OFFSET, screen_right + SHADOW_OFFSET, screen_bottom - layer_button_height + SHADOW_OFFSET);
        layer_interface.subway_button_shadow = temp_subway_button_shadow;
    } else if (extraMapData->drawSubway) {

        t_bound_box temp_subway_button(screen_right - layer_button_width + SHADOW_OFFSET / 2, screen_bottom + SHADOW_OFFSET / 2, screen_right + SHADOW_OFFSET / 2, screen_bottom - layer_button_height + SHADOW_OFFSET / 2);
        layer_interface.subway_button = temp_subway_button;

        t_bound_box temp_subway_button_shadow(screen_right - layer_button_width + SHADOW_OFFSET, screen_bottom + SHADOW_OFFSET, screen_right + SHADOW_OFFSET, screen_bottom - layer_button_height + SHADOW_OFFSET);
        layer_interface.subway_button_shadow = temp_subway_button_shadow;
    }
}

//Refresh result interface based on events

void interface::result_interface_refresh(string input1, string input2, float world_x, float world_y) {
    //Mouse input
    if (input1 == "\0" && input2 == "\0") {
        int scrn_x = xworld_to_scrn(world_x);
        int scrn_y = yworld_to_scrn(world_y);

        if (check_screen_intersect(scrn_x, scrn_y, search_interface.search_bar) || check_screen_intersect(scrn_x, scrn_y, search_interface.search_button)) {
            return;
        } else {
            result_interface_remove(scrn_x, scrn_y);
        }
    } else if (input2 == "\0") {
        //Single input, Street Search
        if (mapID->search_input_type(input1).front() == 'P') {
            //Decode to get POI index
            string index = mapID->search_input_type(input1);
            index.erase(index.begin());

            result_interface_add(stoul(index), input1, input2);

        } else if (mapID->search_input_type(input1).front() == 'S') {
            error_interface_spec("To search intersection, enter two street names separated with ','");
        } else {
            error_interface_spec("Point of interest or street entered does not exist");
        }
    } else {
        //Intersection search
        if (mapID->search_input_type(input1).front() == 'S' && mapID->search_input_type(input2).front() == 'S') {
            if (input1 != input2) {
                //No need to pass in indexes due to get intersections from street names function
                result_interface_add(0, input1, input2);
            } else {
                error_interface_spec("Can not search intersections between the same street");
            }
        } else if (mapID->search_input_type(input1).front() == 'P' || mapID->search_input_type(input2).front() == 'P') {
            error_interface_spec("Intersection search inputs can not include point of interest names");
        } else {
            error_interface_spec("Entered streets do not exist");
        }
    }
    draw_screen();
}

//Draw all information stored in result_interface vector

void interface::result_interface_draw(bool shadow) {
    //Recalculate all result layouts
    unsigned counter = 0;
    for (vector<result_layout>::iterator iter = result_interface.begin(); iter != result_interface.end(); iter++) {
        result_interface_spec(*iter, iter->world_xy, iter->description, counter);
        counter++;
    }

    set_coordinate_system(GL_SCREEN);

    if (result_panel_count > 0) {
        if (shadow) {
            setcolor(52, 73, 94, 255 * 0.4); //SHADOW
            fillrect(result_title_shadow);
        }
        //Draw search result title and text
        setcolor(236, 240, 241, 255);
        fillrect(result_title);
        setcolor(52, 73, 94, 255);
        setfontsize(10);
        drawtext_in(result_title, "SEARCH RESULTS", 100);
    }

    //Draw panel and labels shadows
    if (shadow) {
        for (vector<result_layout>::iterator iter = result_interface.begin(); iter != result_interface.end(); iter++) {
            setcolor(52, 73, 94, 255 * 0.4); //SHADOW
            fillrect(iter->panel_shadow);
            fillpoly(iter->label_shadow, 5);
        }
    }
    //Draw label components
    for (vector<result_layout>::iterator iter = result_interface.begin(); iter != result_interface.end(); iter++) {
        setcolor(236, 240, 241, 255);
        fillpoly(iter->label, 5);

        setcolor(231, 76, 60, 255);
        fillrect(iter->label_header);

        setcolor(236, 240, 241, 255);
        setfontsize(12);
        drawtext_in(iter->label_header, iter->alpha, 100);
    }

    //Draw panel components
    for (vector<result_layout>::iterator iter = result_interface.begin(); iter != result_interface.end(); iter++) {

        setcolor(236, 240, 241, 255); //PANEL COLOR - CLOUD
        fillrect(iter->panel);

        setcolor(36, 47, 62, 255); //PANEL HEADER - DEEP BLUE
        fillrect(iter->panel_header);

        setcolor(231, 76, 60, 255); //PANEL EXIT - ALIZARIN
        fillrect(iter->panel_exit);

        setcolor(52, 73, 94, 255); //DESCRIPTION - WET ASPHALT
        setfontsize(11);
        drawtext_in(iter->panel_text, iter->description, 100);
        setfontsize(10);
        drawtext_in(iter->panel_coord, iter->coord, 100);

        setcolor(236, 240, 241, 255);
        setfontsize(14);
        drawtext_in(iter->panel_header, iter->alpha, 100);
    }

    set_coordinate_system(GL_WORLD);
}

//Add intersection to result_interface vector

void interface::result_interface_add(unsigned index, string input1, string input2) {
    if (input2 == "\0") {
        //POI search
        pair<float, float> position(mapCoord->lon_to_x(getPointOfInterestPosition(index).lon()), mapCoord->lat_to_y(getPointOfInterestPosition(index).lat()));

        result_layout temp;
        //Store ids for direction search
        temp.inter_id = -1;
        temp.poi_name = input1;
        string information = input1;

        if (!result_interface_duplicate_check(temp.inter_id, temp.poi_name)) {
            result_panel_count++;
            result_interface_spec(temp, position, information, result_panel_count);
            result_interface.push_back(temp);
        } else {
            error_interface_spec("You already searched this Point of Interest");
            error_interface_draw(true);
        }
    } else {
        //Intersection search
        vector<IntersectionIndex> inters = find_intersection_ids_from_street_names(input1, input2);
        if (inters.empty()) {
            error_interface_spec("Entered streets does not intersect");
            error_interface_draw(true);
            return;
        }

        for (vector<IntersectionIndex>::iterator iter = inters.begin(); iter != inters.end(); iter++) {
            pair<float, float> position(mapCoord->lon_to_x(getIntersectionPosition(*iter).lon()), mapCoord->lat_to_y(getIntersectionPosition(*iter).lat()));

            result_layout temp;
            //Store ids for direction search
            temp.inter_id = *iter;
            temp.poi_name = "\0";
            string information = input1 + " & " + input2;

            if (!result_interface_duplicate_check(temp.inter_id, temp.poi_name)) {
                result_panel_count++;
                result_interface_spec(temp, position, information, result_panel_count);
                result_interface.push_back(temp);
            } else {
                error_interface_spec("Intersection already displayed in search results");
                error_interface_draw(true);
            }
        }
    }

    //Move map to search result min-xy, max-xy
    if (result_panel_count != 0) {
        center_screen_at(result_interface[result_panel_count - 1].world_xy.first, result_interface[result_panel_count - 1].world_xy.second, 30);
    }
}

//Check and Remove from result_interface vector

void interface::result_interface_remove(float scrn_x, float scrn_y) {
    if (result_panel_count == 0) {
        return;
    } else {
        for (vector<result_layout>::iterator iter = result_interface.begin(); iter != result_interface.end(); iter++) {
            if (check_screen_intersect(scrn_x, scrn_y, iter->panel_exit)) {
                result_interface.erase(iter);
                result_panel_count--;

                break;
            }
        }
    }
}

//Build structure result_layout

void interface::result_interface_spec(result_layout& temp, pair<float, float> position, string information, unsigned panel_number) {
    config_panel_offsets();

    temp.world_xy.first = position.first;
    temp.world_xy.second = position.second;

    //Description
    temp.description = information;

    //Latitude Longitude
    ostringstream coord_stream;
    coord_stream << "Intersection" << " at ";
    if (mapCoord->y_to_lat(position.second) >= 0) coord_stream << setprecision(6) << mapCoord->y_to_lat(position.second) << "° N, ";
    if (mapCoord->y_to_lat(position.second) < 0) coord_stream << setprecision(6) << 0 - mapCoord->y_to_lat(position.second) << "° S, ";
    if (mapCoord->x_to_lon(position.first) >= 0) coord_stream << setprecision(6) << mapCoord->x_to_lon(position.first) << "° E";
    if (mapCoord->x_to_lon(position.first) < 0) coord_stream << setprecision(6) << 0 - mapCoord->x_to_lon(position.first) << "° W";
    temp.coord = coord_stream.str();

    int scrn_x = xworld_to_scrn(temp.world_xy.first);
    int scrn_y = yworld_to_scrn(temp.world_xy.second);

    string s(1, static_cast<char> ('A' + panel_number));
    temp.alpha = s;

    //Search Result title
    t_bound_box temp_result_title(X_OFFSET, search_bar_height + SEARCH_RESULT_OFFSET, X_OFFSET + panel_width, search_bar_height + PANEL_GAP);
    result_title = temp_result_title;

    t_bound_box temp_result_title_shadow(X_OFFSET + SHADOW_OFFSET, search_bar_height + SEARCH_RESULT_OFFSET + SHADOW_OFFSET, X_OFFSET + panel_width + SHADOW_OFFSET, search_bar_height + PANEL_GAP + SHADOW_OFFSET);
    result_title_shadow = temp_result_title_shadow;

    //t_bound_box construction: left, bottom, right, top
    //Panel
    t_bound_box temp_panel(X_OFFSET, SEARCH_RESULT_OFFSET + search_bar_height + (panel_height * (panel_number + 1)), X_OFFSET + panel_width - exit_width, SEARCH_RESULT_OFFSET + search_bar_height + (panel_height * panel_number) + PANEL_GAP);
    temp.panel = temp_panel;
    t_bound_box temp_panel_text(X_OFFSET, SEARCH_RESULT_OFFSET + search_bar_height + (panel_height * (panel_number + 1)) - panel_height / 2, X_OFFSET + panel_width, SEARCH_RESULT_OFFSET + search_bar_height + (panel_height * panel_number) + PANEL_GAP);
    temp.panel_text = temp_panel_text;
    t_bound_box temp_panel_coord(X_OFFSET, SEARCH_RESULT_OFFSET + search_bar_height + (panel_height * (panel_number + 1)), X_OFFSET + panel_width, SEARCH_RESULT_OFFSET + search_bar_height + (panel_height * panel_number) + PANEL_GAP + panel_height / 2);
    temp.panel_coord = temp_panel_coord;
    t_bound_box temp_panel_header(X_OFFSET, SEARCH_RESULT_OFFSET + search_bar_height + (panel_height * (panel_number + 1)), X_OFFSET + header_width, SEARCH_RESULT_OFFSET + search_bar_height + (panel_height * panel_number) + PANEL_GAP);
    temp.panel_header = temp_panel_header;
    t_bound_box temp_panel_exit(X_OFFSET + panel_width - exit_width, SEARCH_RESULT_OFFSET + search_bar_height + (panel_height * (panel_number + 1)), X_OFFSET + panel_width, SEARCH_RESULT_OFFSET + search_bar_height + (panel_height * panel_number) + PANEL_GAP);
    temp.panel_exit = temp_panel_exit;
    t_bound_box temp_panel_shadow(X_OFFSET + SHADOW_OFFSET, SEARCH_RESULT_OFFSET + search_bar_height + (panel_height * (panel_number + 1)) + SHADOW_OFFSET, X_OFFSET + panel_width + SHADOW_OFFSET, SEARCH_RESULT_OFFSET + search_bar_height + (panel_height * panel_number) + PANEL_GAP + SHADOW_OFFSET);
    temp.panel_shadow = temp_panel_shadow;

    //Label
    t_point temp_label_0(scrn_x, scrn_y);
    t_point temp_label_1(scrn_x - label_width / 2, scrn_y - label_height / 4);
    t_point temp_label_2(scrn_x - label_width / 2, scrn_y - label_height);
    t_point temp_label_3(scrn_x + label_width / 2, scrn_y - label_height);
    t_point temp_label_4(scrn_x + label_width / 2, scrn_y - label_height / 4);
    temp.label[0] = temp_label_0;
    temp.label[1] = temp_label_1;
    temp.label[2] = temp_label_2;
    temp.label[3] = temp_label_3;
    temp.label[4] = temp_label_4;
    t_point temp_label_shadow_0(scrn_x + SHADOW_OFFSET, scrn_y + SHADOW_OFFSET);
    t_point temp_label_shadow_1(scrn_x - label_width / 2 + SHADOW_OFFSET, scrn_y - label_height / 4 + SHADOW_OFFSET);
    t_point temp_label_shadow_2(scrn_x - label_width / 2 + SHADOW_OFFSET, scrn_y - label_height + SHADOW_OFFSET);
    t_point temp_label_shadow_3(scrn_x + label_width / 2 + SHADOW_OFFSET, scrn_y - label_height + SHADOW_OFFSET);
    t_point temp_label_shadow_4(scrn_x + label_width / 2 + SHADOW_OFFSET, scrn_y - label_height / 4 + SHADOW_OFFSET);
    temp.label_shadow[0] = temp_label_shadow_0;
    temp.label_shadow[1] = temp_label_shadow_1;
    temp.label_shadow[2] = temp_label_shadow_2;
    temp.label_shadow[3] = temp_label_shadow_3;
    temp.label_shadow[4] = temp_label_shadow_4;
    t_bound_box temp_label_header(scrn_x - label_width / 2, scrn_y - label_height / 4, scrn_x + label_width / 2, scrn_y - label_height);
    temp.label_header = temp_label_header;

}

//Check if there are result panel referring to the same object

bool interface::result_interface_duplicate_check(int inter_id, string poi_name) {
    for (vector<result_layout>::iterator iter = result_interface.begin(); iter != result_interface.end(); iter++) {
        if (iter->inter_id == -1 && poi_name == iter->poi_name) return true;

        if (iter->poi_name == "\0" && inter_id == iter->inter_id) return true;
    }
    return false;
}

//Refresh intersection interface based on events

void interface::highlighted_interface_refresh(float world_x, float world_y) {
    int scrn_x = xworld_to_scrn(world_x);
    int scrn_y = yworld_to_scrn(world_y);

    //Check mouse/UI intersection
    bool forbid = false;
    if (direction_interface.from_hold || direction_interface.dest_hold[0]) forbid = true;
    if (check_screen_intersect(scrn_x, scrn_y, search_interface.search_bar)) forbid = true;
    if (check_screen_intersect(scrn_x, scrn_y, search_interface.search_button)) forbid = true;
    if (check_screen_intersect(scrn_x, scrn_y, result_title)) forbid = true;
    if (check_screen_intersect(scrn_x, scrn_y, highlighted_title)) forbid = true;
    if (check_screen_intersect(scrn_x, scrn_y, layer_interface.subway_button)) forbid = true;
    if (check_screen_intersect(scrn_x, scrn_y, direction_interface.search_button)) forbid = true;
    if (direction_interface.enabled && check_screen_intersect(scrn_x, scrn_y, direction_interface.from_header)) forbid = true;
    if (direction_interface.enabled && check_screen_intersect(scrn_x, scrn_y, direction_interface.dest_header[0])) forbid = true;
    if (direction_interface.routing && check_screen_intersect(scrn_x, scrn_y, direction_interface.instruction_window)) forbid = true;
    if (direction_interface.routing && check_screen_intersect(scrn_x, scrn_y, direction_interface.instruction_exit)) forbid = true;
    for (vector<result_layout>::iterator iter = result_interface.begin(); iter != result_interface.end(); iter++) {
        if (check_screen_intersect(scrn_x, scrn_y, iter->panel_exit)) forbid = true;
        if (check_screen_intersect(scrn_x, scrn_y, iter->panel)) {
            center_screen_at(iter->world_xy.first, iter->world_xy.second, 30);
            forbid = true;
        }
    }
    for (vector<highlighted_layout>::iterator iter = highlighted_interface.begin(); iter != highlighted_interface.end(); iter++) {
        if (check_screen_intersect(scrn_x, scrn_y, iter->panel)) {
            center_screen_at(iter->world_xy.first, iter->world_xy.second, 30);
            forbid = true;
        }
        if (forbid) break;
    }
    if (forbid) return;

    if (!highlighted_interface_remove(scrn_x, scrn_y)) {

        highlighted_interface_add(world_x, world_y);
    }

    draw_screen();
}

//Draw all information stored in highlighted_interface vector

void interface::highlighted_interface_draw(bool shadow) {
    //Recalculate all intersection layouts
    unsigned counter = 0;
    for (vector<highlighted_layout>::iterator iter = highlighted_interface.begin(); iter != highlighted_interface.end(); iter++) {
        highlighted_interface_spec(*iter, iter->world_xy.first, iter->world_xy.second, counter, iter->description, iter->type);
        counter++;
    }

    set_coordinate_system(GL_SCREEN);

    //Draw panel and labels shadows
    if (shadow) {
        for (vector<highlighted_layout>::iterator iter = highlighted_interface.begin(); iter != highlighted_interface.end(); iter++) {
            setcolor(52, 73, 94, 255 * 0.4); //SHADOW
            fillrect(iter->panel_shadow);
            fillpoly(iter->label_shadow, 5);
        }
    }

    //Draw label components
    for (vector<highlighted_layout>::iterator iter = highlighted_interface.begin(); iter != highlighted_interface.end(); iter++) {
        setcolor(236, 240, 241, 255);
        fillpoly(iter->label, 5);

        setcolor(231, 76, 60, 255);
        fillrect(iter->label_header);

        setcolor(236, 240, 241, 255);
        setfontsize(12);
        drawtext_in(iter->label_header, iter->number, 100);
    }

    if (highlighted_panel_count > 0) {
        if (shadow) {
            setcolor(52, 73, 94, 255 * 0.4); //SHADOW
            fillrect(highlighted_title_shadow);
        }
        //highlighted title and text
        setcolor(236, 240, 241, 255);
        fillrect(highlighted_title);
        setcolor(52, 73, 94, 255);
        setfontsize(10);
        drawtext_in(highlighted_title, "HIGHLIGHTED", 100);
    }

    //Draw panel components
    for (vector<highlighted_layout>::iterator iter = highlighted_interface.begin(); iter != highlighted_interface.end(); iter++) {

        setcolor(236, 240, 241, 255); //PANEL COLOR - CLOUD
        fillrect(iter->panel);

        setcolor(36, 47, 62, 255); //PANEL HEADER - DEEP BLUE
        fillrect(iter->panel_header);

        setcolor(231, 76, 60, 255); //PANEL EXIT - ALIZARIN
        fillrect(iter->panel_exit);

        setcolor(52, 73, 94, 255); //DESCRIPTION - WET ASPHALT
        setfontsize(11);
        drawtext_in(iter->panel_text, iter->description, 100);
        setfontsize(10);
        drawtext_in(iter->panel_coord, iter->coord, 100);

        setcolor(236, 240, 241, 255);
        setfontsize(14);
        drawtext_in(iter->panel_header, iter->number, 100);
    }

    set_coordinate_system(GL_WORLD);
}

//Add intersection to highlighted_interface vector

void interface::highlighted_interface_add(float world_x, float world_y) {
    if (highlighted_panel_count == 5) {
        error_interface_spec("Max number of highlight panels reached, press the red edges to close panels");
        error_interface_draw(true);
    } else {
        LatLon click_coord(mapCoord->y_to_lat(world_y), mapCoord->x_to_lon(world_x));
        pair<char, unsigned> closest = kdTrees->findClosestElement(click_coord.lat(), click_coord.lon());
        string description;
        string type;
        float correct_x = 0, correct_y = 0;

        if (closest.first == 'n') {
            error_interface_spec("Object clicked is invisible at current zoom level or area clicked is empty");
            error_interface_draw(true);
        } else {
            highlighted_layout new_panel;

            //Intersection
            if (closest.first == 'i') {
                new_panel.inter_id = closest.second;
                new_panel.poi_name = "\0";
                description = getIntersectionName(closest.second);
                type = "Intersection";
                correct_x = mapCoord->lon_to_x(getIntersectionPosition(closest.second).lon());
                correct_y = mapCoord->lat_to_y(getIntersectionPosition(closest.second).lat());
            }

            //POI
            if (closest.first == 'p') {
                new_panel.inter_id = -1;
                new_panel.poi_name = extraMapData->POI_get_name(closest.second);
                description = extraMapData->POI_get_name(closest.second);
                //description = getPointOfInterestName(closest.second);
                type = mapID->sort_POI(extraMapData->POI_get_type(closest.second));
                //type = mapID->sort_POI(getPointOfInterestType(closest.second));
                correct_x = mapCoord->lon_to_x(extraMapData->POI_get_position(closest.second).second);
                correct_y = mapCoord->lat_to_y(extraMapData->POI_get_position(closest.second).first);
            }

            if (!highlighted_interface_duplicate_check(new_panel.inter_id, new_panel.poi_name)) {
                size_t index = 0;
                while (true) {
                    /* Locate the substring to replace. */
                    index = description.find("<unknown>", index);
                    if (index == std::string::npos) break;
                    description.replace(index, 9, "Unknown Street");
                    index += 14;
                }
                if (description.size() > 52) {
                    description.resize(52);
                    description = description + "...";
                }
                //Preliminary build
                highlighted_panel_count++;
                highlighted_interface_spec(new_panel, correct_x, correct_y, highlighted_panel_count, description, type);
                highlighted_interface.push_back(new_panel);
            } else {
                error_interface_spec("Point of Interest/Intersection already highlighted");
                error_interface_draw(true);
            }
        }
    }
}

//Check and Remove from highlighted_interface vector

bool interface::highlighted_interface_remove(float scrn_x, float scrn_y) {
    if (highlighted_panel_count == 0) {
        return false;
    } else {
        bool deleted = false;
        for (vector<result_layout>::iterator iter = result_interface.begin(); iter != result_interface.end(); iter++) {
            if (check_screen_intersect(scrn_x, scrn_y, iter->panel_exit)) deleted = true;
            if (deleted) break;
        }

        for (vector<highlighted_layout>::iterator iter = highlighted_interface.begin(); iter != highlighted_interface.end(); iter++) {
            if (check_screen_intersect(scrn_x, scrn_y, iter->panel_exit)) {
                highlighted_interface.erase(iter);
                deleted = true;
                highlighted_panel_count--;
            }

            if (deleted) break;
        }
        return deleted;
    }
}

//Build structure intersection_layout

void interface::highlighted_interface_spec(highlighted_layout &temp, float world_x, float world_y, unsigned panel_number, string text, string type) {
    config_panel_offsets();

    int scrn_x = xworld_to_scrn(world_x);
    int scrn_y = yworld_to_scrn(world_y);

    temp.world_xy.first = world_x;
    temp.world_xy.second = world_y;
    temp.number = to_string(panel_number + 1);
    temp.description = text;
    temp.type = type;
    //Latitude Longitude
    ostringstream stream;
    if (type.find("<unknown>") == string::npos) stream << type << " at ";
    if (mapCoord->y_to_lat(world_y) >= 0) stream << setprecision(6) << mapCoord->y_to_lat(world_y) << "° N, ";
    if (mapCoord->y_to_lat(world_y) < 0) stream << setprecision(6) << 0 - mapCoord->y_to_lat(world_y) << "° S, ";
    if (mapCoord->x_to_lon(world_x) >= 0) stream << setprecision(6) << mapCoord->x_to_lon(world_x) << "° E";
    if (mapCoord->x_to_lon(world_x) < 0) stream << setprecision(6) << 0 - mapCoord->x_to_lon(world_x) << "° W";
    temp.coord = stream.str();

    //Highlighted Result title
    t_bound_box temp_highlighted_title(X_OFFSET, search_bar_height + SEARCH_RESULT_OFFSET + HIGHLIGHTED_RESULT_OFFSET + panel_height * result_panel_count, X_OFFSET + panel_width, search_bar_height + SEARCH_RESULT_OFFSET + panel_height * result_panel_count + PANEL_GAP);
    highlighted_title = temp_highlighted_title;

    t_bound_box temp_highlighted_title_shadow(X_OFFSET + SHADOW_OFFSET, search_bar_height + SEARCH_RESULT_OFFSET + HIGHLIGHTED_RESULT_OFFSET + panel_height * result_panel_count + SHADOW_OFFSET, X_OFFSET + panel_width + SHADOW_OFFSET, search_bar_height + SEARCH_RESULT_OFFSET + panel_height * result_panel_count + PANEL_GAP + SHADOW_OFFSET);
    highlighted_title_shadow = temp_highlighted_title_shadow;

    //t_bound_box construction: left, bottom, right, top
    //Panel
    t_bound_box temp_panel(X_OFFSET, SEARCH_RESULT_OFFSET + HIGHLIGHTED_RESULT_OFFSET + search_bar_height + (panel_height * result_panel_count) + (panel_height * (panel_number + 1)), X_OFFSET + panel_width - exit_width, SEARCH_RESULT_OFFSET + HIGHLIGHTED_RESULT_OFFSET + search_bar_height + (panel_height * result_panel_count) + (panel_height * panel_number) + PANEL_GAP);
    temp.panel = temp_panel;
    t_bound_box temp_panel_text(X_OFFSET, SEARCH_RESULT_OFFSET + HIGHLIGHTED_RESULT_OFFSET + search_bar_height + (panel_height * result_panel_count) + (panel_height * (panel_number + 1)) - panel_height / 2, X_OFFSET + panel_width, SEARCH_RESULT_OFFSET + HIGHLIGHTED_RESULT_OFFSET + search_bar_height + (panel_height * result_panel_count) + (panel_height * panel_number) + PANEL_GAP);
    temp.panel_text = temp_panel_text;
    t_bound_box temp_panel_coord(X_OFFSET, SEARCH_RESULT_OFFSET + HIGHLIGHTED_RESULT_OFFSET + search_bar_height + (panel_height * result_panel_count) + (panel_height * (panel_number + 1)), X_OFFSET + panel_width, SEARCH_RESULT_OFFSET + HIGHLIGHTED_RESULT_OFFSET + search_bar_height + (panel_height * result_panel_count) + (panel_height * panel_number) + PANEL_GAP + panel_height / 2);
    temp.panel_coord = temp_panel_coord;
    t_bound_box temp_panel_header(X_OFFSET, SEARCH_RESULT_OFFSET + HIGHLIGHTED_RESULT_OFFSET + search_bar_height + (panel_height * result_panel_count) + (panel_height * (panel_number + 1)), X_OFFSET + header_width, SEARCH_RESULT_OFFSET + HIGHLIGHTED_RESULT_OFFSET + search_bar_height + (panel_height * result_panel_count) + (panel_height * panel_number) + PANEL_GAP);
    temp.panel_header = temp_panel_header;
    t_bound_box temp_panel_exit(X_OFFSET + panel_width - exit_width, SEARCH_RESULT_OFFSET + HIGHLIGHTED_RESULT_OFFSET + search_bar_height + (panel_height * result_panel_count) + (panel_height * (panel_number + 1)), X_OFFSET + panel_width, SEARCH_RESULT_OFFSET + HIGHLIGHTED_RESULT_OFFSET + search_bar_height + (panel_height * result_panel_count) + (panel_height * panel_number) + PANEL_GAP);
    temp.panel_exit = temp_panel_exit;
    t_bound_box temp_panel_shadow(X_OFFSET + SHADOW_OFFSET, SEARCH_RESULT_OFFSET + HIGHLIGHTED_RESULT_OFFSET + search_bar_height + (panel_height * result_panel_count) + (panel_height * (panel_number + 1)) + SHADOW_OFFSET, X_OFFSET + panel_width + SHADOW_OFFSET, SEARCH_RESULT_OFFSET + HIGHLIGHTED_RESULT_OFFSET + search_bar_height + (panel_height * result_panel_count) + (panel_height * panel_number) + PANEL_GAP + SHADOW_OFFSET);
    temp.panel_shadow = temp_panel_shadow;

    //Label
    t_point temp_label_0(scrn_x, scrn_y);
    t_point temp_label_1(scrn_x - label_width / 2, scrn_y - label_height / 4);
    t_point temp_label_2(scrn_x - label_width / 2, scrn_y - label_height);
    t_point temp_label_3(scrn_x + label_width / 2, scrn_y - label_height);
    t_point temp_label_4(scrn_x + label_width / 2, scrn_y - label_height / 4);
    temp.label[0] = temp_label_0;
    temp.label[1] = temp_label_1;
    temp.label[2] = temp_label_2;
    temp.label[3] = temp_label_3;
    temp.label[4] = temp_label_4;
    t_point temp_label_shadow_0(scrn_x + SHADOW_OFFSET, scrn_y + SHADOW_OFFSET);
    t_point temp_label_shadow_1(scrn_x - label_width / 2 + SHADOW_OFFSET, scrn_y - label_height / 4 + SHADOW_OFFSET);
    t_point temp_label_shadow_2(scrn_x - label_width / 2 + SHADOW_OFFSET, scrn_y - label_height + SHADOW_OFFSET);
    t_point temp_label_shadow_3(scrn_x + label_width / 2 + SHADOW_OFFSET, scrn_y - label_height + SHADOW_OFFSET);
    t_point temp_label_shadow_4(scrn_x + label_width / 2 + SHADOW_OFFSET, scrn_y - label_height / 4 + SHADOW_OFFSET);
    temp.label_shadow[0] = temp_label_shadow_0;
    temp.label_shadow[1] = temp_label_shadow_1;
    temp.label_shadow[2] = temp_label_shadow_2;
    temp.label_shadow[3] = temp_label_shadow_3;
    temp.label_shadow[4] = temp_label_shadow_4;
    t_bound_box temp_label_header(scrn_x - label_width / 2, scrn_y - label_height / 4, scrn_x + label_width / 2, scrn_y - label_height);
    temp.label_header = temp_label_header;
}

//Check if there are highlighted panels referring to the same object

bool interface::highlighted_interface_duplicate_check(int inter_id, string poi_name) {
    for (vector<highlighted_layout>::iterator iter = highlighted_interface.begin(); iter != highlighted_interface.end(); iter++) {
        if (iter->inter_id == -1 && poi_name == iter->poi_name) return true;
        if (iter->poi_name == "\0" && inter_id == iter->inter_id) return true;
    }
    return false;
}

//Refresh direction interface on mouse click and mouse move

void interface::direction_interface_refresh(float scrn_x, float scrn_y, int button) {
    if (result_panel_count + highlighted_panel_count < 2 || direction_instruction_check_poi_panels()) {
        if (direction_interface.enabled) {
            direction_interface.enabled = false;
            direction_interface.routing = false;
            draw_screen();
        }

        if (check_screen_intersect(scrn_x, scrn_y, direction_interface.search_button) && (button == 1 || button == 2)) {
            error_interface_spec("Need two panels with at least one intersection panel to enable direction search");
            error_interface_draw(true);
        }
    } else {
        if (direction_indicator_out_of_position()) {
            direction_interface.enabled = true;
            if (direction_interface.routing) direction_interface.routing = false;
            //Reset indicator position
            direction_indicator_reset();
            direction_interface_draw(true);
        }

        //Left click or keyboard input
        if (button == 1 || button == 2) {
            if (button == 1) {
                //Left click
                if (check_screen_intersect(scrn_x, scrn_y, direction_interface.from_header)) {
                    //Click on from indicator
                    if (direction_interface.from_hold) {
                        if ((direction_interface.from_position.first == 'S' && result_interface[direction_interface.from_position.second].inter_id == -1)
                                || (direction_interface.from_position.first == 'H' && highlighted_interface[direction_interface.from_position.second].inter_id == -1)) {
                            direction_interface.from_hold = true;
                            error_interface_spec("From indicator can not be a Point of Interest, please lock in an intersection instead");
                            error_interface_draw(true);
                        } else direction_interface.from_hold = false;
                    } else {
                        last_from_position = direction_interface.from_position;
                        last_dest_position.clear();
                        last_dest_position.push_back(direction_interface.dest_position[0]);
                        direction_interface.from_hold = true;
                        direction_interface.dest_hold[0] = false;
                    }
                } else if (check_screen_intersect(scrn_x, scrn_y, direction_interface.dest_header[0])) {
                    //Click on dest indicator
                    if (direction_interface.dest_hold[0]) {
                        if ((direction_interface.from_position.first == 'S' && result_interface[direction_interface.from_position.second].inter_id == -1)
                                || (direction_interface.from_position.first == 'H' && highlighted_interface[direction_interface.from_position.second].inter_id == -1)) {
                            direction_interface.from_hold = true;
                            error_interface_spec("From indicator can not be a Point of Interest, please lock in an intersection instead");
                            error_interface_draw(true);
                        } else direction_interface.from_hold = false;
                        direction_interface.dest_hold[0] = false;
                    } else {
                        last_from_position = direction_interface.from_position;
                        last_dest_position.clear();
                        last_dest_position.push_back(direction_interface.dest_position[0]);
                        direction_interface.dest_hold[0] = true;
                        direction_interface.from_hold = false;
                    }
                } else if (check_screen_intersect(scrn_x, scrn_y, direction_interface.search_button)) {
                    if (direction_interface.from_hold || direction_interface.dest_hold[0]) {
                        error_interface_spec("Direction indicators have to be locked to search, left click the indicator to lock");
                        error_interface_draw(true);
                    } else {
                        //Click search button
                        direction_interface.searching = true;
                        direction_interface_spec();
                        direction_interface_draw(false);
                        //Find and create path
                        path_segments.clear();
                        //path_segments = find_path_between_intersections(10192,79363,TURN_PENALTY);
                        //path_segments = find_path_to_point_of_interest(108781, "Motorcycle parking", 15);

                        //path_segments = find_path_to_point_of_interest(49549, "Coffee Time", 15);     
                        //std::cout<<compute_path_travel_time(path_segments, TURN_PENALTY) << std::endl;
                        if (direction_interface.from_position.first == 'S') {
                            if (direction_interface.dest_position[0].first == 'S') {
                                //SS
                                if (result_interface[direction_interface.dest_position[0].second].poi_name == "\0") {
                                    //intersection to intersection
                                    path_segments = find_path_between_intersections(result_interface[direction_interface.from_position.second].inter_id, result_interface[direction_interface.dest_position[0].second].inter_id, TURN_PENALTY);
                                    mapGraphics->createPath(path_segments);
                                    direction_instruction_set_routing(path_segments);
                                } else {
                                    //intersection to poi
                                    path_segments = find_path_to_point_of_interest(result_interface[direction_interface.from_position.second].inter_id, result_interface[direction_interface.dest_position[0].second].poi_name, TURN_PENALTY);
                                    mapGraphics->createPath(path_segments);
                                    direction_instruction_set_routing(path_segments);
                                }
                            } else {
                                //SH
                                if (highlighted_interface[direction_interface.dest_position[0].second].poi_name == "\0") {
                                    path_segments = find_path_between_intersections(result_interface[direction_interface.from_position.second].inter_id, highlighted_interface[direction_interface.dest_position[0].second].inter_id, TURN_PENALTY);
                                    mapGraphics->createPath(path_segments);
                                    direction_instruction_set_routing(path_segments);
                                } else {
                                    path_segments = find_path_to_point_of_interest(result_interface[direction_interface.from_position.second].inter_id, highlighted_interface[direction_interface.dest_position[0].second].poi_name, TURN_PENALTY);
                                    mapGraphics->createPath(path_segments);
                                    direction_instruction_set_routing(path_segments);
                                }
                            }
                        } else {
                            if (direction_interface.dest_position[0].first == 'S') {
                                //HS
                                if (result_interface[direction_interface.dest_position[0].second].poi_name == "\0") {
                                    path_segments = find_path_between_intersections(highlighted_interface[direction_interface.from_position.second].inter_id, result_interface[direction_interface.dest_position[0].second].inter_id, TURN_PENALTY);
                                    mapGraphics->createPath(path_segments);
                                    direction_instruction_set_routing(path_segments);
                                } else {
                                    path_segments = find_path_to_point_of_interest(highlighted_interface[direction_interface.from_position.second].inter_id, result_interface[direction_interface.dest_position[0].second].poi_name, TURN_PENALTY);
                                    mapGraphics->createPath(path_segments);
                                    direction_instruction_set_routing(path_segments);
                                }
                            } else {
                                //HH
                                if (highlighted_interface[direction_interface.dest_position[0].second].poi_name == "\0") {
                                    path_segments = find_path_between_intersections(highlighted_interface[direction_interface.from_position.second].inter_id, highlighted_interface[direction_interface.dest_position[0].second].inter_id, TURN_PENALTY);
                                    mapGraphics->createPath(path_segments);
                                    direction_instruction_set_routing(path_segments);
                                } else {
                                    path_segments = find_path_to_point_of_interest(highlighted_interface[direction_interface.from_position.second].inter_id, highlighted_interface[direction_interface.dest_position[0].second].poi_name, TURN_PENALTY);
                                    mapGraphics->createPath(path_segments);
                                    direction_instruction_set_routing(path_segments);
                                }
                            }
                        }

                        if (!direction_interface.routing) {
                            error_interface_spec("There is no path that connects the origin and the destination");
                            error_interface_draw(true);
                        } else {
                            direction_interface.instructions = mapGraphics->instructions;
                            direction_interface.instruction_position = 0;
                            center_screen_at(direction_interface.instructions[direction_interface.instruction_position].startOfSeg.first, direction_interface.instructions[direction_interface.instruction_position].startOfSeg.second, calc_zoom_factor(direction_interface.instructions[direction_interface.instruction_position].dist));
                        }
                        direction_interface.searching = false;
                    }
                } else if (direction_interface.routing && check_screen_intersect(scrn_x, scrn_y, direction_interface.instruction_exit)) {
                    //Click on instruction exit
                    direction_interface.routing = false;
                } else if (direction_interface.routing && check_screen_intersect(scrn_x, scrn_y, direction_interface.instruction_left)) {
                    //Click on instruction left
                    if (direction_interface.instruction_position != 0) {
                        direction_interface.instruction_position--;
                        center_screen_at(direction_interface.instructions[direction_interface.instruction_position].startOfSeg.first, direction_interface.instructions[direction_interface.instruction_position].startOfSeg.second, calc_zoom_factor(direction_interface.instructions[direction_interface.instruction_position].dist));
                    }
                } else if (direction_interface.routing && check_screen_intersect(scrn_x, scrn_y, direction_interface.instruction_right)) {
                    //Click on instruction right
                    if (direction_interface.instruction_position != direction_interface.instructions.size() - 1) {
                        direction_interface.instruction_position++;
                        center_screen_at(direction_interface.instructions[direction_interface.instruction_position].startOfSeg.first, direction_interface.instructions[direction_interface.instruction_position].startOfSeg.second, calc_zoom_factor(direction_interface.instructions[direction_interface.instruction_position].dist));
                    }
                } else if (direction_interface.routing && check_screen_intersect(scrn_x, scrn_y, direction_interface.instruction_window)) {
                    //Click on instruction panel
                    center_screen_at(direction_interface.instructions[direction_interface.instruction_position].startOfSeg.first, direction_interface.instructions[direction_interface.instruction_position].startOfSeg.second, calc_zoom_factor(direction_interface.instructions[direction_interface.instruction_position].dist));
                } else {
                    //click rest of the map
                    if ((direction_interface.from_position.first == 'S' && result_interface[direction_interface.from_position.second].inter_id == -1)
                            || (direction_interface.from_position.first == 'H' && highlighted_interface[direction_interface.from_position.second].inter_id == -1)) {
                        direction_interface.from_hold = true;
                        error_interface_spec("From indicator can not be a Point of Interest, please lock in an intersection instead");
                        error_interface_draw(true);
                    } else direction_interface.from_hold = false;

                    if (direction_interface.dest_hold[0]) direction_interface.dest_hold[0] = false;
                }
            }
            //Re-specification and draw_screen required
            direction_interface_spec();
            draw_screen();
        } else {
            //Mouse move
            if (direction_interface.from_hold) {
                if (direction_indicator_movement(scrn_y)) {
                    direction_interface_spec();
                    draw_screen();
                }
            } else if (direction_interface.dest_hold[0]) {
                if (direction_indicator_movement(scrn_y)) {
                    direction_interface_spec();
                    draw_screen();
                }
            }
        }
    }
}

//Draw refreshed direction interface

void interface::direction_interface_draw(bool shadow) {
    direction_interface_spec();

    set_coordinate_system(GL_SCREEN);

    if (!direction_interface.enabled) {
        //Not enabled
        if (shadow) {
            setcolor(52, 73, 94, 255 * 0.4); //SHADOW
            fillrect(direction_interface.search_shadow);
        }
        //Search button
        setcolor(136, 147, 162, 255);
        fillrect(direction_interface.search_button);
        //Button Text
        setcolor(206, 210, 211, 255);
        setfontsize(20);
        drawtext_in(direction_interface.search_button, "DR", 100);
        /*
        string file_temp = "Icons/interface_direction.png";
        const char * file = file_temp.c_str();
        Surface icon = load_png_from_file(file);
        draw_surface(icon, direction_interface.search_button.left() + direction_interface.search_button.get_width() / 4, direction_interface.search_button.top());
         */
    } else {
        if (shadow) {
            setcolor(52, 73, 94, 255 * 0.4); //SHADOW
            fillrect(direction_interface.search_shadow);
            fillpoly(&direction_interface.from_shadow[0], 5);
            fillpoly(&direction_interface.dest_shadow[0][0], 5);
        }
        //Search button
        setcolor(56, 67, 82, 255);
        fillrect(direction_interface.search_button);
        //Button Text
        /*
        string file_temp = "Icons/interface_direction.png";
        const char * file = file_temp.c_str();
        Surface icon = load_png_from_file(file);
        draw_surface(icon, direction_interface.search_button.get_xcenter() + direction_interface.search_button.get_width() / 4, direction_interface.search_button.get_ycenter());
         */
        setcolor(236, 240, 241, 255);
        setfontsize(20);
        drawtext_in(direction_interface.search_button, "DR", 100);


        //From indicator
        setcolor(236, 240, 241, 255); //PANEL COLOR - CLOUD
        fillpoly(&direction_interface.from_button[0], 5);
        if (direction_interface.from_hold) {
            setcolor(86, 97, 112, 255); //PANEL HEADER - LIGHT BLUE
            fillrect(direction_interface.from_header);
        } else {
            setcolor(36, 47, 62, 255); //PANEL HEADER - DEEP BLUE
            fillrect(direction_interface.from_header);
        }
        setcolor(236, 240, 241, 255);
        setfontsize(11);
        drawtext_in(direction_interface.from_header, "FROM", 100);

        //Dest indicator
        setcolor(236, 240, 241, 255); //PANEL COLOR - CLOUD
        fillpoly(&direction_interface.dest_button[0][0], 5);
        if (direction_interface.dest_hold[0]) {
            setcolor(86, 97, 112, 255); //PANEL HEADER - LIGHT BLUE
            fillrect(direction_interface.dest_header[0]);
        } else {
            setcolor(36, 47, 62, 255); //PANEL HEADER - DEEP BLUE
            fillrect(direction_interface.dest_header[0]);
        }
        setcolor(236, 240, 241, 255);
        setfontsize(11);
        drawtext_in(direction_interface.dest_header[0], "DEST", 100);

        if (direction_interface.searching) {
            setcolor(52, 73, 94, 255 * 0.4); //SHADOW
            fillrect(direction_interface.wait_window_shadow);
            setcolor(36, 47, 62, 255);
            fillrect(direction_interface.wait_window);
            setcolor(236, 240, 241, 255);
            setfontsize(11);
            drawtext_in(direction_interface.wait_window, "Finding Shortest Path ...", 100);
        }

        if (direction_interface.routing) {
            if (shadow) {
                setcolor(52, 73, 94, 255 * 0.4); //SHADOW
                fillrect(direction_interface.instruction_shadow);
            }
            setcolor(236, 240, 241, 255); //PANEL COLOR - CLOUD
            fillrect(direction_interface.instruction_window);
            setcolor(36, 47, 62, 255); //PANEL HEADER - DEEP BLUE
            fillrect(direction_interface.instruction_traveltime);

            if (direction_interface.instruction_position != 0) {
                setcolor(36, 47, 62, 255); //PANEL HEADER - DEEP BLUE
                fillrect(direction_interface.instruction_left);
            } else {
                setcolor(36, 47, 62, 255);
                fillrect(direction_interface.instruction_left);
            }
            if (direction_interface.instruction_position != direction_interface.instructions.size() - 1) {
                setcolor(36, 47, 62, 255); //PANEL HEADER - DEEP BLUE
                fillrect(direction_interface.instruction_right);
            } else {
                setcolor(36, 47, 62, 255);
                fillrect(direction_interface.instruction_right);
            }

            //String replace unknown and length limit
            size_t index = 0;
            while (true) {
                /* Locate the substring to replace. */
                index = direction_interface.instructions[direction_interface.instruction_position].instructFollow.find("<unknown>", index);
                if (index == std::string::npos) break;
                direction_interface.instructions[direction_interface.instruction_position].instructFollow.replace(index, 9, "Unknown Street");
                index += 14;
            }
            index = 0;
            while (true) {
                /* Locate the substring to replace. */
                index = direction_interface.instructions[direction_interface.instruction_position].instructTurn.find("<unknown>", index);
                if (index == std::string::npos) break;
                direction_interface.instructions[direction_interface.instruction_position].instructTurn.replace(index, 9, "Unknown Street");
                index += 14;
            }
            if (direction_interface.instructions[direction_interface.instruction_position].instructFollow.size() > 62) {
                direction_interface.instructions[direction_interface.instruction_position].instructFollow.resize(62);
                direction_interface.instructions[direction_interface.instruction_position].instructFollow = direction_interface.instructions[direction_interface.instruction_position].instructFollow + "...";
            }
            if (direction_interface.instructions[direction_interface.instruction_position].instructTurn.size() > 62) {
                direction_interface.instructions[direction_interface.instruction_position].instructTurn.resize(62);
                direction_interface.instructions[direction_interface.instruction_position].instructTurn = direction_interface.instructions[direction_interface.instruction_position].instructTurn + "...";
            }

            setcolor(231, 76, 60, 255); //PANEL EXIT - ALIZARIN
            fillrect(direction_interface.instruction_exit);
            setcolor(236, 240, 241, 255);
            setfontsize(55);
            if (direction_interface.instruction_position != 0) drawtext_in(direction_interface.instruction_left, "<", 100);
            if (direction_interface.instruction_position != direction_interface.instructions.size() - 1) drawtext_in(direction_interface.instruction_right, ">", 100);
            setcolor(236, 240, 241, 255); //PANEL COLOR - CLOUD
            setfontsize(11);

            string traveltime = to_string(compute_path_travel_time(path_segments, TURN_PENALTY) / 60);
            traveltime = traveltime.substr(0, traveltime.find(".", 0) + 2);
            drawtext_in(direction_interface.instruction_traveltime, "Estimated Travel Time: " + traveltime + " minutes", 100);
            setcolor(52, 73, 94, 255); //DESCRIPTION - WET ASPHALT

            setfontsize(11);
            drawtext_in(direction_interface.instruction_string_1, direction_interface.instructions[direction_interface.instruction_position].instructFollow, 100);
            drawtext_in(direction_interface.instruction_string_2, direction_interface.instructions[direction_interface.instruction_position].instructTurn, 100);
            setcolor(102, 123, 144, 255); //DESCRIPTION - WET ASPHALT
            setfontsize(10);
            drawtext_in(direction_interface.instruction_string_distance, "Segment Length: " + direction_interface.instructions[direction_interface.instruction_position].distStr, 100);
        }
    }

    set_coordinate_system(GL_WORLD);
}

//Reconfigure direction interface constants

void interface::direction_interface_spec() {
    //t_bound_box construction: left, bottom, right, top
    //search button
    t_bound_box temp_search_button(X_OFFSET + search_bar_width, direction_search_height, X_OFFSET + search_bar_width + direction_search_width, PANEL_GAP);
    direction_interface.search_button = temp_search_button;
    t_bound_box temp_search_shadow(X_OFFSET + search_bar_width + SHADOW_OFFSET, direction_search_height + SHADOW_OFFSET, X_OFFSET + search_bar_width + direction_search_width + SHADOW_OFFSET, PANEL_GAP + SHADOW_OFFSET);
    direction_interface.search_shadow = temp_search_shadow;

    if (direction_interface.enabled) {
        if (direction_interface.searching) {
            t_bound_box temp_wait_window(X_OFFSET * 2 + direction_search_width + search_bar_width, search_bar_height, X_OFFSET * 2 + search_bar_width + direction_search_width + wait_window_width, PANEL_GAP);
            direction_interface.wait_window = temp_wait_window;

            t_bound_box temp_wait_window_shadow(X_OFFSET * 2 + direction_search_width + search_bar_width + SHADOW_OFFSET, search_bar_height + SHADOW_OFFSET, X_OFFSET * 2 + search_bar_width + direction_search_width + wait_window_width + SHADOW_OFFSET, PANEL_GAP + SHADOW_OFFSET);
            direction_interface.wait_window_shadow = temp_wait_window_shadow;
        }

        float from_x = 0, from_y = 0, dest_x = 0, dest_y = 0;
        if (direction_interface.from_position.first == 'S') {
            from_x = result_interface[direction_interface.from_position.second].panel_exit.right() + INDICATOR_X_OFFSET;
            from_y = result_interface[direction_interface.from_position.second].panel_exit.get_ycenter();
        } else if (direction_interface.from_position.first == 'H') {
            from_x = highlighted_interface[direction_interface.from_position.second].panel_exit.right() + INDICATOR_X_OFFSET;
            from_y = highlighted_interface[direction_interface.from_position.second].panel_exit.get_ycenter();
        }
        if (direction_interface.dest_position[0].first == 'S') {
            dest_x = result_interface[direction_interface.dest_position[0].second].panel_exit.right() + INDICATOR_X_OFFSET;
            dest_y = result_interface[direction_interface.dest_position[0].second].panel_exit.get_ycenter();
        } else if (direction_interface.dest_position[0].first == 'H') {
            dest_x = highlighted_interface[direction_interface.dest_position[0].second].panel_exit.right() + INDICATOR_X_OFFSET;
            dest_y = highlighted_interface[direction_interface.dest_position[0].second].panel_exit.get_ycenter();
        }

        //From indicator
        t_point temp_from_button_0(from_x, from_y);
        t_point temp_from_button_1(from_x + indicator_width / 5, from_y - indicator_height / 2);
        t_point temp_from_button_2(from_x + indicator_width, from_y - indicator_height / 2);
        t_point temp_from_button_3(from_x + indicator_width, from_y + indicator_height / 2);
        t_point temp_from_button_4(from_x + indicator_width / 5, from_y + indicator_height / 2);
        direction_interface.from_button[0] = temp_from_button_0;
        direction_interface.from_button[1] = temp_from_button_1;
        direction_interface.from_button[2] = temp_from_button_2;
        direction_interface.from_button[3] = temp_from_button_3;
        direction_interface.from_button[4] = temp_from_button_4;
        t_point temp_from_shadow_0(from_x + SHADOW_OFFSET, from_y + SHADOW_OFFSET);
        t_point temp_from_shadow_1(from_x + indicator_width / 5 + SHADOW_OFFSET, from_y - indicator_height / 2 + SHADOW_OFFSET);
        t_point temp_from_shadow_2(from_x + indicator_width + SHADOW_OFFSET, from_y - indicator_height / 2 + SHADOW_OFFSET);
        t_point temp_from_shadow_3(from_x + indicator_width + SHADOW_OFFSET, from_y + indicator_height / 2 + SHADOW_OFFSET);
        t_point temp_from_shadow_4(from_x + indicator_width / 5 + SHADOW_OFFSET, from_y + indicator_height / 2 + SHADOW_OFFSET);
        direction_interface.from_shadow[0] = temp_from_shadow_0;
        direction_interface.from_shadow[1] = temp_from_shadow_1;
        direction_interface.from_shadow[2] = temp_from_shadow_2;
        direction_interface.from_shadow[3] = temp_from_shadow_3;
        direction_interface.from_shadow[4] = temp_from_shadow_4;
        t_bound_box temp_from_header(from_x + indicator_width / 5, from_y + indicator_height / 2, from_x + indicator_width, from_y - indicator_height / 2);
        direction_interface.from_header = temp_from_header;

        //Dest indicator
        t_point temp_dest_button_0(dest_x, dest_y);
        t_point temp_dest_button_1(dest_x + indicator_width / 5, dest_y - indicator_height / 2);
        t_point temp_dest_button_2(dest_x + indicator_width, dest_y - indicator_height / 2);
        t_point temp_dest_button_3(dest_x + indicator_width, dest_y + indicator_height / 2);
        t_point temp_dest_button_4(dest_x + indicator_width / 5, dest_y + indicator_height / 2);
        direction_interface.dest_button[0][0] = temp_dest_button_0;
        direction_interface.dest_button[0][1] = temp_dest_button_1;
        direction_interface.dest_button[0][2] = temp_dest_button_2;
        direction_interface.dest_button[0][3] = temp_dest_button_3;
        direction_interface.dest_button[0][4] = temp_dest_button_4;
        t_point temp_dest_shadow_0(dest_x + SHADOW_OFFSET, dest_y + SHADOW_OFFSET);
        t_point temp_dest_shadow_1(dest_x + indicator_width / 5 + SHADOW_OFFSET, dest_y - indicator_height / 2 + SHADOW_OFFSET);
        t_point temp_dest_shadow_2(dest_x + indicator_width + SHADOW_OFFSET, dest_y - indicator_height / 2 + SHADOW_OFFSET);
        t_point temp_dest_shadow_3(dest_x + indicator_width + SHADOW_OFFSET, dest_y + indicator_height / 2 + SHADOW_OFFSET);
        t_point temp_dest_shadow_4(dest_x + indicator_width / 5 + SHADOW_OFFSET, dest_y + indicator_height / 2 + SHADOW_OFFSET);
        direction_interface.dest_shadow[0][0] = temp_dest_shadow_0;
        direction_interface.dest_shadow[0][1] = temp_dest_shadow_1;
        direction_interface.dest_shadow[0][2] = temp_dest_shadow_2;
        direction_interface.dest_shadow[0][3] = temp_dest_shadow_3;
        direction_interface.dest_shadow[0][4] = temp_dest_shadow_4;
        t_bound_box temp_dest_header(dest_x + indicator_width / 5, dest_y + indicator_height / 2, dest_x + indicator_width, dest_y - indicator_height / 2);
        direction_interface.dest_header[0] = temp_dest_header;

        if (direction_interface.routing) {
            screen_xcenter = get_visible_screen().get_width() / 2;
            instruction_y = screen_bottom - error_panel_height - instruction_height - (0 - get_visible_screen().get_height()) / 45;
            //t_bound_box construction: left, bottom, right, top
            t_bound_box temp_instruction_window(screen_xcenter - instruction_width / 2, instruction_y + instruction_height, screen_xcenter + instruction_width / 2, instruction_y + PANEL_GAP);
            direction_interface.instruction_window = temp_instruction_window;
            t_bound_box temp_instruction_shadow(screen_xcenter - instruction_width / 2 + SHADOW_OFFSET, instruction_y + instruction_height + SHADOW_OFFSET, screen_xcenter + instruction_width / 2 + SHADOW_OFFSET, instruction_y + PANEL_GAP + SHADOW_OFFSET);
            direction_interface.instruction_shadow = temp_instruction_shadow;
            t_bound_box temp_instruction_traveltime(screen_xcenter - instruction_width / 4, instruction_y + PANEL_GAP, screen_xcenter + instruction_width / 4, instruction_y + PANEL_GAP - instruction_string_height * 1.15);
            direction_interface.instruction_traveltime = temp_instruction_traveltime;
            t_bound_box temp_instruction_left(screen_xcenter - instruction_width / 2, instruction_y + instruction_height, screen_xcenter - instruction_width / 2 + instruction_button_width, instruction_y + PANEL_GAP);
            direction_interface.instruction_left = temp_instruction_left;
            t_bound_box temp_instruction_right(screen_xcenter + instruction_width / 2 - instruction_button_width, instruction_y + instruction_height, screen_xcenter + instruction_width / 2, instruction_y + PANEL_GAP);
            direction_interface.instruction_right = temp_instruction_right;
            t_bound_box temp_instruction_string_1(screen_xcenter - instruction_width / 2 + instruction_button_width, instruction_y + instruction_string_height + instruction_string_height / 6, screen_xcenter + instruction_width / 2 - instruction_button_width, instruction_y + PANEL_GAP + instruction_string_height / 6);
            direction_interface.instruction_string_1 = temp_instruction_string_1;
            t_bound_box temp_instruction_string_2(screen_xcenter - instruction_width / 2 + instruction_button_width, instruction_y + 2 * instruction_string_height + instruction_string_height / 6, screen_xcenter + instruction_width / 2 - instruction_button_width, instruction_y + instruction_string_height + instruction_string_height / 6);
            direction_interface.instruction_string_2 = temp_instruction_string_2;
            t_bound_box temp_instruction_string_distance(screen_xcenter - instruction_width / 2 + instruction_button_width, instruction_y + 3 * instruction_string_height, screen_xcenter + instruction_width / 2 - instruction_button_width, instruction_y + 2 * instruction_string_height + instruction_string_height / 10);
            direction_interface.instruction_string_distance = temp_instruction_string_distance;
            t_bound_box temp_instruction_exit(screen_xcenter + instruction_width / 2, instruction_y + instruction_height / 2 + instruction_exit_height / 2, screen_xcenter + instruction_width / 2 + instruction_exit_width, instruction_y + instruction_height / 2 - instruction_exit_height / 2);
            direction_interface.instruction_exit = temp_instruction_exit;
        }
    }
}

//Reconfigure direction indicator positions / Drag and Drop

bool interface::direction_indicator_movement(float scrn_y) {
    if (direction_interface.from_hold) {
        //Save position
        if (direction_interface.from_position != last_dest_position[0]) {
            last_dest_position.clear();
            last_dest_position.push_back(direction_interface.dest_position[0]);
        }
        last_from_position = direction_interface.from_position;
        //Reposition from indicator
        unsigned counter = 0;
        if (result_panel_count != 0) {
            for (vector<result_layout>::iterator iter = result_interface.begin(); iter != result_interface.end(); iter++) {
                if (scrn_y > iter->panel.top() && scrn_y < iter->panel.bottom()) {
                    if (!(direction_interface.from_position.first == 'S' && direction_interface.from_position.second == counter)) {
                        //If indicators overlap
                        if (direction_interface.dest_position[0].first == 'S' && direction_interface.dest_position[0].second == counter) {
                            direction_interface.dest_position[0].first = direction_interface.from_position.first;
                            direction_interface.dest_position[0].second = direction_interface.from_position.second;
                        }
                        direction_interface.from_position.first = 'S';
                        direction_interface.from_position.second = counter;

                    }
                }
                counter++;
            }
        }
        counter = 0;
        if (highlighted_panel_count != 0) {
            for (vector<highlighted_layout>::iterator iter = highlighted_interface.begin(); iter != highlighted_interface.end(); iter++) {
                if (scrn_y > iter->panel.top() && scrn_y < iter->panel.bottom()) {
                    if (!(direction_interface.from_position.first == 'H' && direction_interface.from_position.second == counter)) {
                        //If indicators overlap
                        if (direction_interface.dest_position[0].first == 'H' && direction_interface.dest_position[0].second == counter) {
                            direction_interface.dest_position[0].first = direction_interface.from_position.first;
                            direction_interface.dest_position[0].second = direction_interface.from_position.second;
                        }
                        direction_interface.from_position.first = 'H';
                        direction_interface.from_position.second = counter;
                    }
                }
                counter++;
            }
        }
        if (direction_interface.from_position != last_dest_position[0]) direction_interface.dest_position[0] = last_dest_position[0];
        if (direction_interface.from_position == last_from_position) return false;
        else return true;
    } else if (direction_interface.dest_hold[0]) {
        //Save position
        if (direction_interface.dest_position[0] != last_from_position) {
            last_from_position = direction_interface.from_position;
        }
        last_dest_position.clear();
        last_dest_position.push_back(direction_interface.dest_position[0]);
        //Reposition dest indicator
        unsigned counter = 0;
        if (result_panel_count != 0) {
            for (vector<result_layout>::iterator iter = result_interface.begin(); iter != result_interface.end(); iter++) {
                if (scrn_y > iter->panel.top() && scrn_y < iter->panel.bottom()) {
                    if (!(direction_interface.dest_position[0].first == 'S' && direction_interface.dest_position[0].second == counter)) {
                        //If indicators overlap
                        if (direction_interface.from_position.first == 'S' && direction_interface.from_position.second == counter) {
                            //if (result_interface[direction_interface.dest_position[0].second].inter_id == -1) return false;
                            direction_interface.from_position.first = direction_interface.dest_position[0].first;
                            direction_interface.from_position.second = direction_interface.dest_position[0].second;
                        }
                        direction_interface.dest_position[0].first = 'S';
                        direction_interface.dest_position[0].second = counter;
                    }
                }
                counter++;
            }
        }
        counter = 0;
        if (highlighted_panel_count != 0) {
            for (vector<highlighted_layout>::iterator iter = highlighted_interface.begin(); iter != highlighted_interface.end(); iter++) {
                if (scrn_y > iter->panel.top() && scrn_y < iter->panel.bottom()) {
                    if (!(direction_interface.dest_position[0].first == 'H' && direction_interface.dest_position[0].second == counter)) {
                        //If indicators overlap
                        if (direction_interface.from_position.first == 'H' && direction_interface.from_position.second == counter) {
                            //if (highlighted_interface[direction_interface.dest_position[0].second].inter_id == -1) return false;
                            direction_interface.from_position.first = direction_interface.dest_position[0].first;
                            direction_interface.from_position.second = direction_interface.dest_position[0].second;
                        }
                        direction_interface.dest_position[0].first = 'H';
                        direction_interface.dest_position[0].second = counter;
                    }
                }
                counter++;
            }
        }
        if (direction_interface.dest_position[0] != last_from_position) direction_interface.from_position = last_from_position;
        if (direction_interface.dest_position[0] == last_dest_position[0]) return false;
        else return true;
    }
    return false;
}

//Check if any indicator requires a reset

bool interface::direction_indicator_out_of_position() {
    if ((!direction_interface.enabled)
            || (direction_interface.from_position.first == 'S' && direction_interface.from_position.second >= result_panel_count)
            || (direction_interface.from_position.first == 'H' && direction_interface.from_position.second >= highlighted_panel_count)
            || (direction_interface.dest_position[0].first == 'S' && direction_interface.dest_position[0].second >= result_panel_count)
            || (direction_interface.dest_position[0].first == 'H' && direction_interface.dest_position[0].second >= highlighted_panel_count)) {
        return true;
    } else return false;
}

//Reset indictor position when it is out of position

void interface::direction_indicator_reset() {
    unsigned counter = 0;
    bool from_set = false;
    for (vector<result_layout>::iterator iter = result_interface.begin(); (iter != result_interface.end()) && !from_set; iter++) {
        if (iter->poi_name == "\0") {
            pair<char, unsigned> temp_from_pos('S', counter);
            direction_interface.from_position = temp_from_pos;
            from_set = true;
        }
        counter++;
    }
    counter = 0;
    for (vector<highlighted_layout>::iterator iter = highlighted_interface.begin(); (iter != highlighted_interface.end()) && !from_set; iter++) {
        if (iter->poi_name == "\0") {
            pair<char, unsigned> temp_from_pos('H', counter);
            direction_interface.from_position = temp_from_pos;
            from_set = true;
        }
        counter++;
    }
    counter = 0;
    bool dest_set = false;
    for (vector<result_layout>::iterator iter = result_interface.begin(); (iter != result_interface.end()) && !dest_set; iter++) {
        if (!(direction_interface.from_position.first == 'S' && direction_interface.from_position.second == counter)) {
            pair<char, unsigned> temp_dest_pos('S', counter);
            direction_interface.dest_position[0] = temp_dest_pos;
            dest_set = true;
        }
        counter++;
    }
    counter = 0;
    for (vector<highlighted_layout>::iterator iter = highlighted_interface.begin(); (iter != highlighted_interface.end()) && !dest_set; iter++) {
        if (!(direction_interface.from_position.first == 'H' && direction_interface.from_position.second == counter)) {
            pair<char, unsigned> temp_dest_pos('H', counter);
            direction_interface.dest_position[0] = temp_dest_pos;
            dest_set = true;
        }
        counter++;
    }
}

//Accessor to current displayed instruction

dirnInstruct interface::direction_instruction_get_current_instruction() {
    return direction_interface.instructions[direction_interface.instruction_position];
}

//Check of all panels are poi panels

bool interface::direction_instruction_check_poi_panels() {
    if (result_panel_count != 0) {
        for (vector<result_layout>::iterator iter = result_interface.begin(); iter != result_interface.end(); iter++) {
            if (iter->poi_name == "\0") return false;
        }
    }
    if (highlighted_panel_count != 0) {
        for (vector<highlighted_layout>::iterator iter = highlighted_interface.begin(); iter != highlighted_interface.end(); iter++) {
            if (iter->poi_name == "\0") return false;
        }
    }
    return true;
}

//Accessor to routing toggle

bool interface::direction_instruction_check_routing() {
    return direction_interface.routing;
}

//Mutator for routing

void interface::direction_instruction_set_routing(vector<unsigned> path) {
    if (path.empty()) direction_interface.routing = false;
    else direction_interface.routing = true;
}

//Draw error

void interface::error_interface_draw(bool shadow) {
    error_interface.timer++;
    error_interface_spec(error_interface.error);

    set_coordinate_system(GL_SCREEN);
    if (!error_interface.error.empty()) {
        if (shadow) {
            setcolor(52, 73, 94, 255 * 0.4); //SHADOW
            fillrect(error_interface.panel_shadow);
        }
        setcolor(236, 240, 241, 255); //PANEL COLOR - CLOUD
        fillrect(error_interface.panel);

        setcolor(52, 73, 94, 255); //DESCRIPTION - WET ASPHALT
        setfontsize(11);
        drawtext_in(error_interface.panel, error_interface.error, 100);
    }

    if (error_interface.prev_error != error_interface.error) error_interface.timer = 0;
    if (error_interface.timer == 7) {

        error_interface.error.clear();
        error_interface.timer = 0;
    }
    error_interface.prev_error = error_interface.error;

    set_coordinate_system(GL_WORLD);

}

//Reconfigure error interface specifications

void interface::error_interface_spec(string error) {
    screen_xcenter = get_visible_screen().get_width() / 2;
    error_interface.error = error;

    //Error box
    t_bound_box temp_error_panel(screen_xcenter - error_panel_width / 2, screen_bottom, screen_xcenter + error_panel_width / 2, screen_bottom - error_panel_height);
    error_interface.panel = temp_error_panel;

    t_bound_box temp_error_panel_shadow(screen_xcenter - error_panel_width / 2 + SHADOW_OFFSET, screen_bottom + SHADOW_OFFSET, screen_xcenter + error_panel_width / 2 + SHADOW_OFFSET, screen_bottom - error_panel_height + SHADOW_OFFSET);
    error_interface.panel_shadow = temp_error_panel_shadow;
}

//Check if user click within bound, different from t_bound_box intersect function as screen coordinate is fliped

bool interface::check_screen_intersect(float scrn_x, float scrn_y, t_bound_box bound) {
    if (bound.left() < scrn_x && bound.bottom() > scrn_y && bound.right() > scrn_x && bound.top() < scrn_y) {
        return true;
    }
    return false;
}

void interface::config_panel_offsets() {
    //Configure offset
    if (result_panel_count == 0) SEARCH_RESULT_OFFSET = 0;
    else SEARCH_RESULT_OFFSET = 30;
    if (highlighted_panel_count == 0) HIGHLIGHTED_RESULT_OFFSET = 0;
    else HIGHLIGHTED_RESULT_OFFSET = 30;
}

float interface::calc_zoom_factor(double segmentlength) {
    if (segmentlength <= 2000) return 30;
    else if (segmentlength <= 5000) return 40;
    else if (segmentlength <= 8000) return 60;
    else if (segmentlength <= 12000) return 90;
    else return 120;
}

void interface::center_screen_at(float world_x, float world_y, float zoom) {
    set_visible_world((world_x - zoom), (world_y - zoom), (world_x + zoom), (world_y + zoom));
}

//When find path (intersection to poi) function finds the closet poi(with same name) to source intersection, the position of the old poi will be changed by this function

void interface::adjust_poi_panel_to_closest(float world_x, float world_y) {
    if (direction_interface.dest_position[0].first == 'S') {
        if ((result_interface[direction_interface.dest_position[0].second].world_xy.first != world_x) ||
                (result_interface[direction_interface.dest_position[0].second].world_xy.second != world_y)) {
            result_interface[direction_interface.dest_position[0].second].world_xy.first = world_x;
            result_interface[direction_interface.dest_position[0].second].world_xy.second = world_y;
            error_interface_spec("There is an identical point of interest closer to your position, a new path is generated");
            error_interface_draw(true);
        }
    } else if (direction_interface.dest_position[0].first == 'H') {
        if ((highlighted_interface[direction_interface.dest_position[0].second].world_xy.first != world_x) ||
                (highlighted_interface[direction_interface.dest_position[0].second].world_xy.second != world_y)) {
            highlighted_interface[direction_interface.dest_position[0].second].world_xy.first = world_x;
            highlighted_interface[direction_interface.dest_position[0].second].world_xy.second = world_y;
            error_interface_spec("There is an identical point of interest closer to your position, a new path is generated");
            error_interface_draw(true);
        }
    }
}

