#include <cassert>
#include <cstdlib>
#include <fstream>
#include "server_util.h"

// returns the manhattan distance between two points
long long manhattan(const Point& pt1, const Point& pt2) {
  return abs(pt1.lat-pt2.lat) + abs(pt1.lon-pt2.lon);
}

// finds the id of the point that is closest to the given point "pt"
int findClosest(const Point& pt, const unordered_map<int, Point>& points) {
  pair<int, Point> best = *points.begin();

  // just scan each vertex, linear time is acceptable
  for (const auto& check : points) {
    if (manhattan(pt, check.second) < manhattan(pt, best.second)) {
      best = check;
    }
  }
  return best.first;
}

// Read the graph from a file that is assumed to have the same format as the
// "Edmonton graph" file, store the weighted graph in g and the mapping
// of vertex IDs to coordinates in "points".
// Will not clear g nor points beforehand.
void readGraph(const string& filename, WDigraph& g, unordered_map<int, Point>& points) {
  ifstream fin(filename);
  string line;

  while (getline(fin, line)) {
    // split the string around the commas, there will always be 4 substrings
    string p[4];
    int at = 0;
    for (auto c : line) {
      if (c == ',') {
        // start new string
        ++at;
      }
      else {
        // append character to the string we are building
        p[at] += c;
      }
      // don't want to index out of bounds!
      assert(at < 4);
    }

    if (at != 3) {
      // this should not be reached with edmonton-roads-2.0.1.txt, but
      // it's here anyway in case a trailing blank line was added or something
      break;
    }

    if (p[0] == "V") {
      // new vertex

      // get vertex ID
      int id = stoi(p[1]);
      assert(id == stoll(p[1])); // sanity check: asserts if some id is not 32-bit

      // get lat/lon of the vertex
      points[id].lat = static_cast<long long>(stod(p[2])*100000);

      points[id].lon = static_cast<long long>(stod(p[3])*100000);
      g.addVertex(id);
    }
    else {
      // new directed edge

      // get endpoints
      int u = stoi(p[1]), v = stoi(p[2]);
      g.addEdge(u, v, manhattan(points[u], points[v]));

      // note, we ignore street names (i.e. p[3]) in this assignment
    }
  }
}
