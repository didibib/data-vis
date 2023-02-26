#include "precomp.h"

namespace DataVis
{
const char* Tree::__RADIAL = "Radial";

void Tree::Extract::MSP(Tree& _tree, Graph& _graph, int _root)
{
	auto& vertices = _graph.Vertices();

	// Keep track of the parent of each vertex so we can construct a tree after
	std::vector<int> parents;
	// Keep track of which vertex are already processed
	std::vector<bool> included;
	// Keep track of the minimum edge cost of a vertex
	std::vector<float> costs;
	// Keep track of the actual minimum edge associated with the cost
	std::vector<Graph::Edge*> edges;
	parents.resize(vertices.size());
	included.resize(vertices.size());
	costs.resize(vertices.size());
	// Initialize all but the root as infinite, so that the root is picked first
	// Make parent of root -1
	for (int i = 0; i < vertices.size(); i++) {
		if (i != _root) costs[i] = 1e30f;
		else parents[i] = -1;
	}

	// Graph will have |vertices| nodes
	for (int i = 0; i < vertices.size(); i++) {
		// Argmin for costs
		float min = 1e30;
		int idx = -1;
		for (int v = 0; v < vertices.size(); v++) {
			if (!included[v] && costs[v] < min) {
				min = costs[v];
				idx = v;
			}
		}

		// Add lowest vertex
		assert(idx != -1);
		included[idx] = true;

		// Update outgoing edges from this vertex
		auto& out_edges = vertices[idx].m_out_edges;
		for (int j = 0; j < out_edges.size(); j++) {
			int v = out_edges[j].m_target;
			if (!included[v] && out_edges[j].get_property().weight < costs[v]) {
				// Found a cheaper edge to v
				parents[v] = idx;
				costs[v] = out_edges[j].get_property().weight;
			}
		}
	}

	// Construct a tree
	_tree.m_root = std::make_shared<Node>(Node(_root, _graph.Nodes()[_root].get().GetPosition()));

	// Construct recursive lambda to create the tree
	static std::function<void(std::shared_ptr<Node>)> MakeTree = [&](std::shared_ptr<Node> n) {
		// Look through all vertices which have parent == n.vertex
		for (int i = 0; i < vertices.size(); i++)
			if (parents[i] == n->vertex_idx) {
				// Add node to n.children
				auto child = std::make_shared<Node>(Node(i, _graph.Nodes()[i].get().GetPosition(), n));
				n->children.push_back(child);
				MakeTree(child);
			}
	};
	MakeTree(_tree.m_root);
	_tree.PostBuild();
}

//--------------------------------------------------------------
int Tree::Leaves(std::shared_ptr<Node> node)
{
	int leaves = 0;
	if (node->children.size() == 0) leaves++;
	for (auto& child : node->children) leaves += Leaves(child);
	return leaves;
}

int Tree::Depth(std::shared_ptr<Node> node)
{
	int max = 0;
	for (auto& child : node->children)
	{
		int child_depth = Depth(child);
		if (child_depth > max) max = child_depth;
	}
	return max + 1;
}



//--------------------------------------------------------------
void Tree::SwapRoot(std::shared_ptr<Node> _new_root)
{
	std::shared_ptr<Node> node = _new_root;
	while (node != m_root)
	{
		node->children.push_back(node->parent);
		auto& parent_children = node->parent->children;
		for (int i = 0; i < parent_children.size(); i++)
		{
			if (parent_children[i] == node)
			{
				// Replace node with final element
				parent_children[i] = parent_children[parent_children.size() - 1];
				parent_children.resize(parent_children.size() - 1);
				break;
			}
		}
		// Move to next node
		node = node->parent;
	}

	node = _new_root;
	std::shared_ptr<Node> prev = nullptr;
	while (node != nullptr)
	{
		auto next = node->parent;
		node->parent = prev;
		prev = node;
		node = next;
	}

	m_root = _new_root;
	PostBuild();
}

//--------------------------------------------------------------
void Tree::PostBuild()
{
	leaves = Leaves(m_root);
	depth = Depth(m_root);
	CreateReferenceNodes();
}

//--------------------------------------------------------------
void Tree::HandleInput()
{
}

void Tree::Select(const ofCamera& _camera, const glm::vec3& _position)
{
	std::function<std::shared_ptr<Tree::Node>(std::shared_ptr<Tree::Node>)> Select = [&](std::shared_ptr<Tree::Node> _node) {
		if (_node.get()->Inside(_camera, _position)) return _node;
		std::stack<std::shared_ptr<Tree::Node>> stack;
		for (auto& child : _node->children) stack.push(child);
		while (stack.empty() == false) {
			auto& node = stack.top(); stack.pop();
			if (node.get()->Inside(_camera, _position)) return node;
			for (auto& child : node->children) stack.push(child);
		}
		return std::shared_ptr<Tree::Node>();
	};
	m_selected_node = Select(m_root);
}

void Tree::Select( const glm::vec3& _pos )
{
	printf( "Selecting a node" );
	// Loop through all nodes and find one within radius of pos
	std::function<std::shared_ptr<Node>( std::shared_ptr<Node> )> select_in_tree;
	select_in_tree = [&]( std::shared_ptr<Node> n )
	{
		if (n->Inside( _pos )) return n;
		for (auto& child : n->children)
		{
			std::shared_ptr<Node> selected = select_in_tree( child );
			if (selected != std::shared_ptr<Node>()) return selected;
		}
		return std::shared_ptr<Node>();
	};
	SetSelectedNode(select_in_tree( m_root ));
	
}

void Tree::SetSelectedNode( std::shared_ptr<Node> n )
{
	if (m_selected_node != std::shared_ptr<Node>())
		m_selected_node->color = ofColor::white;
	if (n != std::shared_ptr<Node>()) n->color = ofColor::green;
	m_selected_node = n;
}

void Tree::SetBounds()
{
	// Still uses hardcoded radius and delta_angle
	int r = (depth - 1) * 150;
	m_bounds = { {-r, -r}, {r, r} };
	//m_bounds.clear();
	//m_bounds.addVertex( { -r,r,0 } );
	//m_bounds.addVertex( { r, r, 0 } );
	//m_bounds.addVertex( { r, -r, 0 } );
	//m_bounds.addVertex( { -r, -r, 0 } );
	//m_bounds.addVertex( { -r, r, 0 } );

}

void Tree::Update(float _delta_time)
{
	// TODO: Check if we should animate before traversing
	std::function<void(std::shared_ptr<Node>)> EaseInEaseOut = [&](std::shared_ptr<Node> n) {
		n->EaseInEaseOut(_delta_time);
		for (auto child : n->children) EaseInEaseOut(child);
	};
	EaseInEaseOut(m_root);
}

//--------------------------------------------------------------
void Tree::DrawLayout()
{
	ofSetDrawBitmapMode(OF_BITMAPMODE_SIMPLE);

	// Draw radial circles
	ofNoFill();
	ofSetColor(65); 
	ofSetCircleResolution(50);
	for (int i = 0; i < depth - 1; i++)
	{
		ofDrawCircle(glm::vec3(0), 100 + i * 150);
	}

	// Draw nodes and edges
	ofFill();
	ofSetColor(255, 0, 0);
	ofDrawCircle(m_root->GetPosition(), m_root->GetRadius());
	ofDrawBitmapStringHighlight(ofToString(m_root->vertex_idx), m_root->GetPosition() + glm::vec3(10, 10, -1));

	std::stack<std::shared_ptr<Node>> stack;
	for (auto& child : m_root->children) stack.push(child);

	while (stack.empty() == false) {
		auto node = stack.top(); stack.pop();
		auto parent = node->parent;
		static glm::vec3 sub = { 0, 0, -1 }; // To draw edge behind nodes
		ofFill();
		ofSetColor(123);
		ofDrawLine(parent->GetPosition() + sub, node->GetPosition() + sub);

		ofSetColor(node->color);
		ofDrawCircle(node->GetPosition(), node->GetRadius());

		ofDrawBitmapStringHighlight(ofToString(node->vertex_idx), node->GetPosition() + glm::vec3(10, 10, -1));

		for (auto& child : node->children) stack.push(child);
	}
}

void Tree::Gui()
{
	if (m_selected_node.get() != nullptr)
	{
		auto x = m_selected_node.get();
		ImGui::Begin("Selected Node");

		ImGui::Text("Vertex: %i", m_selected_node->vertex_idx);
		ImGui::Text("Position: (%f.0, %f.0)", m_selected_node->GetPosition().x, m_selected_node->GetPosition().y);

		if (ImGui::Button("Make root"))
		{
			SwapRoot(m_selected_node);
			Tree::Layout::Radial(*this, 100, 150);
			UpdateBounds();
		}
		ImGui::End();
	}
}

void Tree::CreateReferenceNodes() {
	// https://jonasdevlieghere.com/containers-of-unique-pointers/
	static std::function<std::vector<std::reference_wrapper<ILayout::Node>>(std::shared_ptr<Tree::Node>)> Wrap
		= [&](shared_ptr<Tree::Node> _root) {
		std::vector<std::reference_wrapper<ILayout::Node>> nodes;
		std::stack<std::shared_ptr<Node>> stack;
		for (auto& child : m_root->children) stack.push(child);
		while (stack.empty() == false) {
			auto node = stack.top(); stack.pop();
			nodes.push_back(std::ref(*node));
			for (auto& child : node->children) stack.push(child);
		}
		return nodes;
	};
	m_reference_nodes = Wrap(m_root);
}

std::vector<std::reference_wrapper<ILayout::Node>> Tree::Nodes()
{
	return m_reference_nodes;
}

//--------------------------------------------------------------
// Layouts
//--------------------------------------------------------------
const std::vector <std::pair<std::string, std::function<void(Tree&, std::string)>>>& Tree::Layout::Functions()
{
	static std::vector <std::pair<std::string, std::function<void(Tree&, std::string)>>> layout_functions = {
		{ __RADIAL, Tree::Layout::RadialCmdline }
	};
	return layout_functions;
}

//--------------------------------------------------------------
void Tree::Layout::RadialCmdline(Tree& _tree, std::string _cmdline_input)
{
	static RadialData rd;
	static auto options = rd.Options();
	ParseCmdline(options, _cmdline_input);
	Radial(_tree, rd.step, rd.delta_angle);
}

void Tree::Layout::Radial(Tree& _tree, float _step, float _delta_angle)
{
	auto& node = _tree.Root();
	node->SetNewPosition(glm::vec3(0));
	RadialSubTree(*node, 0, TWO_PI, 0, _step, _delta_angle);
}

void Tree::Layout::RadialSubTree(Tree::Node& _node, float _wedge_start, float _wedge_end, int _depth, float _step, float _delta_angle)
{
	float new_wedge_start = _wedge_start;
	float radius = _step + (_delta_angle * _depth);
	float parent_leaves = Tree::Leaves(std::make_shared<Tree::Node>(_node));
	for (auto& child : _node.children) {
		float child_leaves = Tree::Leaves(child);
		float new_wedge_end = new_wedge_start + (child_leaves / parent_leaves * (_wedge_end - _wedge_start));
		float angle = (new_wedge_start + new_wedge_end) * .5f;
		child->SetNewPosition(glm::vec3(radius * glm::cos(angle), radius * glm::sin(angle), 0));
		if (child->children.size() > 0)
			RadialSubTree(*child, new_wedge_start, new_wedge_end, _depth + 1, _step, _delta_angle);
		new_wedge_start = new_wedge_end;
	}
}
} // DataVis
