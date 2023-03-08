#pragma once

namespace DataVis {
	class Graph : public ILayout {
	public:

		//--------------------------------------------------------------
		class Extract {
		public:
			static bool Load(Graph&, std::string filename);
		};

		//--------------------------------------------------------------
		Graph() = default;
		void HandleInput() override;
		void Select( const glm::vec3&) override;
		void Update(float delta_time) override;
		void DrawLayout() override;
		void Gui() override;
		std::vector<std::reference_wrapper<ILayout::Node>> Nodes() override;

		//--------------------------------------------------------------
		auto& Edges() { return m_graph.all_edges; }
		auto& Vertices() { return m_graph.all_nodes; }

		float radius = 10;
	protected:
		void PostBuild() override;

	private:
		void CreateReferenceNodes();
		Model::MainGraph m_graph;
		std::vector<std::shared_ptr<ILayout::Node>> m_nodes;
		std::vector<std::reference_wrapper<ILayout::Node>> m_reference_nodes;
	};
}
