#include <cassert>
#include "dijkstra.h"
#include "heap.h"

void dijkstra(const WDigraph& graph, int startVertex, unordered_map<int, PLI>& tree) {
  BinaryHeap<int, PLI> heap;

  tree.clear();

  // the PLI key in the heap has the distance first and the vertex second, so
  // the min item in the heap according to < will have minimum distance

  heap.insert(startVertex, PLI(0, -1));

  while (heap.size() > 0) {
    int v = heap.min().first;
    PLI prev = heap.min().second;

    heap.popMin();

    if (tree.find(v) != tree.end()) {
      // if v was already reached (i.e. "burned"), do nothing
      continue;
    }

    // Record tree[v] = (d,u) where u is the predecessor of v on the shortest
    // path to v and d is the length of this path
    tree[v] = prev;

    // a fire just started at v, burn all outgoing edges!
    for (auto iter = graph.neighbours(v); iter != graph.endIterator(v); iter++) {
      // compute when the fire will reach this neigbour of v:
      //        time_v_burns + edge_cost
      int cost = prev.first + graph.getCost(v, *iter);
      assert(cost >= 0); // sanity check, making sure we don't "overflow"
      heap.insert(*iter, PLI(cost, v));
    }
  }
}
