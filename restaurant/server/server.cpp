#include <iostream>
#include <list>
#include <string>
#include <sstream>
#include <cassert>
#include "wdigraph.h"
#include "dijkstra.h"
#include "server_util.h"
#include "serialport.h"

using namespace std;

int main() {
  WDigraph graph;
  unordered_map<int, Point> points;
  SerialPort Serial("/dev/ttyACM0");
  // build the graph
  readGraph("edmonton-roads-2.0.1.txt", graph, points);

  // read a request
  char c;
  int s = 0;
  Point sPoint, ePoint;
  string temp, temp1, a, out;
  while(true){
    temp = Serial.readline();
    
    stringstream ss(temp);
    cout << temp <<endl;
    ss >> c >> sPoint.lat >> sPoint.lon >> ePoint.lat >> ePoint.lon;

    if(c != 'R'){
      continue;
    }
    //cin >> c >> sPoint.lat >> sPoint.lon >> ePoint.lat >> ePoint.lon;

    // c will be 'R' in part 1, no need to check until part 2

    // get the points closest to the two input points
    int start = findClosest(sPoint, points), end = findClosest(ePoint, points);

    // run dijkstra's to compute a shortest path
    unordered_map<int, PLI> tree;
    dijkstra(graph, start, tree);

    if (tree.find(end) == tree.end()) {
        // no path
        temp = "N 0\n";
        // Use serial.writeline to communicate with arduino
        Serial.writeline(temp);
    }
    else {
      // read off the path by stepping back through the search tree
      list<int> path;
      while (end != start) {
        path.push_front(end);
        end = tree[end].second;
      }
      path.push_front(start);

      // output the path
      
      temp = to_string(path.size());
      
      
      out = "N " + temp + "\n";
      // send the number of the waypoints
      Serial.writeline(out);
      // wait for arduino to input
      a = Serial.readline(2000);
      // check whether it's the right one
      if(a != "A\n"){
        continue;
      }
      s = 0;
      //cout << "N " << path.size() << endl;
      for (auto v : path) {
        
        // put everything into one string and send the waypoints to the client

        temp = to_string(points[v].lat);
        
        temp1 = to_string(points[v].lon);
        out = "W "+ temp + " " + temp1 + "\n";
        
        Serial.writeline(out);
        // wait for client to respond
        a = Serial.readline(2000);
        // check the response
        if(a[0] != 'A'){
          s = 1;
          break;
        }
        //cout << "W " << points[v].lat << ' ' << points[v].lon << endl;
      }
      if(s){
        continue;
      }

      // send the ending signal
      out = "E\n";
      cout << out;
      Serial.writeline(out);
      //cout << "E" << endl;
    }
  }

  return 0;
}
