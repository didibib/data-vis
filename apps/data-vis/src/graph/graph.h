#pragma once

namespace DataVis {
	class Graph : public ILayout {
	public:
		struct Vertex
		{
			std::string name;
		};

		struct Edge
		{
			float weight = 1.f;
		};

		typedef boost::property<boost::graph_name_t, std::string> graph_p;
		typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, Vertex, Edge, graph_p> graph_u;
		typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, Vertex, Edge, graph_p> graph_d;

		//--------------------------------------------------------------
		class Extract {
		public:
			static void Load(Graph&, std::string filename);
		};

		//--------------------------------------------------------------
		Graph() = default;
		void HandleInput() override;
		void Select(const ofCamera&, const glm::vec3&) override;
		void Update(float delta_time) override;
		void Draw() override;
		void Gui() override;
		std::vector<std::reference_wrapper<ILayout::Node>> Nodes() override;

		//--------------------------------------------------------------
		auto& Edges() { return m_graph.m_edges; }
		auto& Vertices() { return m_graph.m_vertices; }

		float radius = 10;
	protected:
		void PostBuild() override;

	private:
		void CreateReferenceNodes();
		graph_u m_graph;
		std::vector<std::shared_ptr<ILayout::Node>> m_nodes;
		std::vector<std::reference_wrapper<ILayout::Node>> m_reference_nodes;
	};
}
