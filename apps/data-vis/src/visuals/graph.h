#pragma once

namespace DataVis {

	struct Node
	{
		std::string name, label, shape;
		glm::vec3 position;
	};

	struct Edge
	{
		std::string label;
		float weight = 1.f;
	};

	typedef boost::property<boost::graph_name_t, std::string> graph_p;
	typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, Node, Edge, graph_p> graph_u;
	typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, Node, Edge, graph_p> graph_d;

	class Graph {
	private:
		void RandomPos();
		graph_u m_graph;

	public:
		Graph() = default;
		void Load(std::string filename);
		void Update();
		void Draw();
		void Exit();
		float radius = 10;
	};
}
