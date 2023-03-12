#pragma once

namespace DataVis
{
class Tree : public IStructure
{
public:
	struct ImGuiData
	{
		bool checkbox_draw_all_edges = false;
		float input_radial_step = 100;
		float input_radial_delta_angle = 100;
	} m_imgui_data;

	class Node : public IStructure::Node
	{
	public:
		Node(std::string _vertex_id, VertexIdx _vertex_index, std::shared_ptr<Node> _parent = nullptr, glm::vec3 _current_position = glm::vec3(0))
			: IStructure::Node(_vertex_id, _vertex_index, _current_position)
		{
			parent = _parent;
		}

		std::shared_ptr<Node> parent;
		std::vector<std::shared_ptr<Node>> children;
	};

	//--------------------------------------------------------------
	Tree() = default;
	void Init(const std::shared_ptr<Dataset>) = 0;

	//--------------------------------------------------------------
	std::shared_ptr<Node> Root();
	static uint Leaves(std::shared_ptr<Tree::Node>);
	static uint Depth(std::shared_ptr<Tree::Node>);
	void SwapRoot(std::shared_ptr<Tree::Node>);

	float speed = 5;

	// Properties
	int leaves = 0, depth = 0;

protected:
	void DrawNodes() override;
	void NodeInfoGui() override;

	void UpdateProperties();
	std::shared_ptr<Tree::Node> m_root;
};

// Prim's Algorithm https://www.wikiwand.com/en/Prim%27s_algorithm
class MSP : public Tree
{
public:
	void Init(const std::shared_ptr<Dataset>) override;

private:
	void Create(VertexIdx _root);
};


} // DataVis