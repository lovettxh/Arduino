#include <Arduino.h>
#include <Adafruit_ILI9341.h>

#include <stdlib.h>
#include <SD.h>
#include "consts_and_types.h"
#include "map_drawing.h"

// the variables to be shared across the project, they are declared here!
shared_vars shared;

Adafruit_ILI9341 tft = Adafruit_ILI9341(clientpins::tft_cs, clientpins::tft_dc);
// set a global variable to record the number of waypoints
int num;

void setup() {
  // initialize Arduino
  init();

  // initialize zoom pins
  pinMode(clientpins::zoom_in_pin, INPUT_PULLUP);
  pinMode(clientpins::zoom_out_pin, INPUT_PULLUP);

  // initialize joystick pins and calibrate centre reading
  pinMode(clientpins::joy_button_pin, INPUT_PULLUP);
  // x and y are reverse because of how our joystick is oriented
  shared.joy_centre = xy_pos(analogRead(clientpins::joy_y_pin), analogRead(clientpins::joy_x_pin));

  // initialize serial port
  Serial.begin(9600);
  Serial.flush(); // get rid of any leftover bits

  // initially no path is stored
  shared.num_waypoints = 0;

  // initialize display
  shared.tft = &tft;
  shared.tft->begin();
  shared.tft->setRotation(3);
  shared.tft->fillScreen(ILI9341_BLUE); // so we know the map redraws properly

  // initialize SD card
  if (!SD.begin(clientpins::sd_cs)) {
      Serial.println("Initialization has failed. Things to check:");
      Serial.println("* Is a card inserted properly?");
      Serial.println("* Is your wiring correct?");
      Serial.println("* Is the chipSelect pin the one for your shield or module?");

      while (1) {} // nothing to do here, fix the card issue and retry
  }

  // initialize the shared variables, from map_drawing.h
  // doesn't actually draw anything, just initializes values
  initialize_display_values();

  // initial draw of the map, from map_drawing.h
  draw_map();
  draw_cursor();

  // initial status message
  status_message("FROM?");
}

void process_input() {
  // read the zoom in and out buttons
  shared.zoom_in_pushed = (digitalRead(clientpins::zoom_in_pin) == LOW);
  shared.zoom_out_pushed = (digitalRead(clientpins::zoom_out_pin) == LOW);

  // read the joystick button
  shared.joy_button_pushed = (digitalRead(clientpins::joy_button_pin) == LOW);

  // joystick speed, higher is faster
  const int16_t step = 64;

  // get the joystick movement, dividing by step discretizes it
  // currently a far joystick push will move the cursor about 5 pixels
  xy_pos delta(
    // the funny x/y swap is because of our joystick orientation
    (analogRead(clientpins::joy_y_pin)-shared.joy_centre.x)/step,
    (analogRead(clientpins::joy_x_pin)-shared.joy_centre.y)/step
  );
  delta.x = -delta.x; // horizontal axis is reversed in our orientation

  // check if there was enough movement to move the cursor
  if (delta.x != 0 || delta.y != 0) {
    // if we are here, there was noticeable movement

    // the next three functions are in map_drawing.h
    erase_cursor();       // erase the current cursor
    move_cursor(delta);   // move the cursor, and the map view if the edge was nudged
    if (shared.redraw_map == 0) {
      // it looks funny if we redraw the cursor before the map scrolls
      draw_cursor();      // draw the new cursor position
    }
  }
}

// this function draw all the routes according to the content in shared.waypoints[]
void map_draw (){
  
  int x1, y1, x2, y2;
  // convert to display coordinate and draw each line.
  for (int i = 0; i < num - 1; i++){
          x1 = longitude_to_x(shared.map_number ,shared.waypoints[i].lon) - shared.map_coords.x;
          y1 = latitude_to_y(shared.map_number ,shared.waypoints[i].lat) - shared.map_coords.y;
          x2 = longitude_to_x(shared.map_number ,shared.waypoints[i + 1].lon) - shared.map_coords.x;
          y2 = latitude_to_y(shared.map_number ,shared.waypoints[i + 1].lat) - shared.map_coords.y;
          
          tft.drawLine(x1, y1, x2, y2, ILI9341_RED);
        }
        
}


// This function communicate with c++ program. Send and receive data
// and store every waypoints.
bool communicate(lon_lat_32 start, lon_lat_32 end, shared_vars &shared){
  String out, temp, c, cc, ccc;

  unsigned long time;
  
  int32_t lon, lat;
  // send out the request, the lat and lon of atarting and ending point
  out = "R " + String(start.lat) + " " + String(start.lon) + " " + String(end.lat) + " " + String(end.lon);
  // send to server
  Serial.println(out);
  // counting for time out
  time = millis();
  while(Serial.available() == 0){
    if(millis() - time > 10000){
      return false;
    }
  }

  // read from the server, this line should be N, number of waypoints
  c = Serial.readString();
    

  // check if it is N
  if(c[0] == 'N'){
    // collect the number part
    cc = c.substring(2);

    // convert to int and store
    num = cc.toInt();
    // send back "A\n" to server
    Serial.println("A");
  }else{
    return false;
  }
  // to see if waypoints exceed 500
  if(num > 500){
    return false;
  }
  // proceed for num times to read num lines of input
  for(int i = 0; i < num; i++){
    // time out
    time = millis();
    while(Serial.available() == 0){
      if(millis() - time > 2000){
        
        return false;
      }
    }
    while(Serial.available() > 0){
      // read line from server
      c = Serial.readString();
    }
    int index;
    // check whether it is 'W'
    if(c[0] == 'W'){
      // convert two coordinates in from string to int and store
      // in shared.waypoints[]
      cc = c.substring(2);
      lat = cc.toInt();
      index = cc.indexOf(' ');
      ccc = cc.substring(index + 1);
      lon = ccc.toInt();
      shared.waypoints[i] = lon_lat_32(lon, lat);
      // send response to the server
      Serial.println("A");
    }else{
      
      return false;
    }
    
  }
  // time out
  time = millis();
  while(Serial.available() == 0){
    if(millis() - time > 2000){
      return false;
    }
  }
  // read last line, should be "E\n"
  c = Serial.readString();
  
  if(c[0] == 'E'){
    return true;
  }else{
    return false;
  }

  
}


int main() {
  setup();

  // very simple finite state machine:
  // which endpoint are we waiting for?
  enum {WAIT_FOR_START, WAIT_FOR_STOP} curr_mode = WAIT_FOR_START;

  // the two points that are clicked
  lon_lat_32 start, end;
  bool s;

  while (true) {
    // clear entries for new state
    shared.zoom_in_pushed = 0;
    shared.zoom_out_pushed = 0;
    shared.joy_button_pushed = 0;
    shared.redraw_map = 0;

    // reads the three buttons and joystick movement
    // updates the cursor view, map display, and sets the
    // shared.redraw_map flag to 1 if we have to redraw the whole map
    // NOTE: this only updates the internal values representing
    // the cursor and map view, the redrawing occurs at the end of this loop
    process_input();

    // if a zoom button was pushed, update the map and cursor view values
    // for that button push (still need to redraw at the end of this loop)
    // function zoom_map() is from map_drawing.h
    if (shared.zoom_in_pushed) {
      zoom_map(1);
      shared.redraw_map = 1;
    }
    else if (shared.zoom_out_pushed) {
      zoom_map(-1);
      shared.redraw_map = 1;
    }

    // if the joystick button was clicked
    if (shared.joy_button_pushed) {

      if (curr_mode == WAIT_FOR_START) {
        // if we were waiting for the start point, record it
        // and indicate we are waiting for the end point
        start = get_cursor_lonlat();
        curr_mode = WAIT_FOR_STOP;
        status_message("TO?");

        // wait until the joystick button is no longer pushed
        while (digitalRead(clientpins::joy_button_pin) == LOW) {}
      }
      else {
        // if we were waiting for the end point, record it
        // and then communicate with the server to get the path
        end = get_cursor_lonlat();

        // TODO: communicate with the server to get the waypoints

        // now we have stored the path length in
        // shared.num_waypoints and the waypoints themselves in
        // the shared.waypoints[] array, switch back to asking for the
        // start point of a new request
        curr_mode = WAIT_FOR_START;

        // wait until the joystick button is no longer pushed
        while (digitalRead(clientpins::joy_button_pin) == LOW) {}
        status_message("Receiving path...");

        // returns whether the communication is success or not
        s = communicate(start, end, shared);
        
        // if it's success, proceed print line and update the map,
        // if it's not, set the status message back to "FROM?" and start a new loop
        if(s){
          //status_message("1FROM?");
          shared.redraw_map = 1;
          
        }else{
          status_message("FROM?");
          continue;
        }

        
      }
    }
    
    if (shared.redraw_map) {
      // redraw the status message
      if (curr_mode == WAIT_FOR_START) {
        status_message("FROM?");
      }
      else {
        status_message("TO?");
      }

      // redraw the map and cursor
      draw_map();
      draw_cursor();
      
      // draw the route
      map_draw();
      
      }
  }

  Serial.flush();
  return 0;
}
