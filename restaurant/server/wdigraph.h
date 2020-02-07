#ifndef _WEIGHTED_DIGRAPH_H_
#define _WEIGHTED_DIGRAPH_H_

#include "digraph.h"
#include <unordered_map>

using namespace std;

class WDigraph : public Digraph {
	public:
		int getCost(int u, int v) const {
			return costs.at(u).at(v);
		}

		void addEdge(int u, int v, int w) {
			Digraph::addEdge(u, v);
			costs[u][v] = w;
		}

	private:
		unordered_map<int, unordered_map<int, int>> costs;
};

#endif
