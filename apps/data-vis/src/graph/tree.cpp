#include "precomp.h"

namespace DataVis
{
	Tree Tree::Extractor::MSP( Graph& _graph )
	{
        // Implement Prim's Algorithm
        // https://www.wikiwand.com/en/Prim%27s_algorithm
        auto& vertices = _graph.Vertices();
        // Initialize lowest cost connections and corresponding edges
        std::vector<float> costs;
        std::vector<Edge*> edges;
        costs.resize(vertices.size());
        edges.resize(vertices.size());

        Tree n;
        return n;
	}

} // DataVis
