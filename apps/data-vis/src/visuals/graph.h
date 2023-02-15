#pragma once
namespace DataVis {
	class Node {
	public:
		Node() = default;
	};

	struct Vertex
	{
		std::string name, label, shape;
	};

	struct Edge
	{
		std::string label;
		float weight;
	};

	typedef boost::property<boost::graph_name_t, std::string> graph_p;
	typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, Vertex, Edge, graph_p> graph_u;
	typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, Vertex, Edge, graph_p> graph_d;

	class Graph {

	private:
		graph_u graph_;

	public:
		Graph() = default;
		void Load(std::string filename);
		void Update();
		void Draw();
		void Exit();
		const graph_u& Get( ) { return graph_; }
	};
}
