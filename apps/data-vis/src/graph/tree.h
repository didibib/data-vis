#pragma once

namespace DataVis
{
class Tree : public ILayout
{
	static const char* __RADIAL;
	struct RadialData : Data
	{
	public:
		float step = 100;
		float delta_angle = 100;
		po::options_description Options() override
		{
			po::options_description desc(__RADIAL);
			desc.add_options()
				("r", po::value(&step), "Radius of the innermost concentric circle, default = 100")
				("d", po::value(&delta_angle), "Delta angle (degrees) constant for the drawing�s concentric circles, default = 100");
			return desc;
		}
	};
	//--------------------------------------------------------------
public:
	struct Node : ILayout::Node
	{
	public:
		Node(int _vertex_idx, glm::vec3 _current_position, std::shared_ptr<Node> _parent = nullptr)
			: ILayout::Node(_vertex_idx, _current_position)
		{
			parent = _parent;
		}
		std::shared_ptr<Node> parent;
		std::vector<std::shared_ptr<Node>> children;
	};

	class Extract
	{
	public:
		// Prim's Algorithm https://www.wikiwand.com/en/Prim%27s_algorithm
		static void MSP(Tree&, Graph&, int _root);
	};

	//--------------------------------------------------------------
	Tree() = default;
	void HandleInput() override;
	void Update(float delta_time) override;
	void Draw() override;
	void Gui() override;
	std::vector<std::reference_wrapper<ILayout::Node>> Nodes() override;

	//--------------------------------------------------------------
	std::shared_ptr<Node> Root() { return m_root; }
	std::shared_ptr<Node> Select(glm::vec3 _pos);
	static int Leaves(std::shared_ptr<Tree::Node>);
	static int Depth(std::shared_ptr<Tree::Node>);
	void SwapRoot(std::shared_ptr<Tree::Node>);

	float radius = 5;
	float speed = 5;
	std::shared_ptr<Tree::Node> selected_node;

	// Properties
	int leaves, depth;

	class Layout {
	public:
		static const std::vector <std::pair<std::string, std::function<void(Tree&, std::string)>>>& Functions();
		static void RadialCmdline(Tree&, std::string);
		static void Radial(Tree&, float step, float delta_angle);
	private:
		// Pavlo, 2006 https://scholarworks.rit.edu/cgi/viewcontent.cgi?referer=&httpsredir=1&article=1355&context=theses
		static void RadialSubTree(Tree::Node&, float angle_start, float angle_end, int depth, float step, float delta_angle);
	};

	//--------------------------------------------------------------
private:
	void CreateReferenceNodes();
	void PostBuild();

	std::shared_ptr<Tree::Node> m_root;
	std::vector<std::reference_wrapper<ILayout::Node>> m_reference_nodes;
};

} // DataVis