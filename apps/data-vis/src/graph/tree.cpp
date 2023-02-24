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
	_tree.m_root = std::make_shared<Node>(Node(_root, _graph.Nodes()[_root].get().position));

	// Construct recursive lambda to create the tree
	static std::function<void(std::shared_ptr<Node>)> MakeTree = [&](std::shared_ptr<Node> n) {
		// Look through all vertices which have parent == n.vertex
		for (int i = 0; i < vertices.size(); i++)
			if (parents[i] == n->vertex_idx) {
				// Add node to n.children
				auto child = std::make_shared<Node>(Node(i, _graph.Nodes()[i].get().position, n));
				n->children.push_back(child);
				MakeTree(child);
			}
	};
	MakeTree(_tree.m_root);
	_tree.SetProperties();
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
std::shared_ptr<Tree::Node> Tree::Select(glm::vec3 _pos)
{
	// Loop through all nodes and find one within radius of pos
	std::function<std::shared_ptr<Node>(std::shared_ptr<Node>)> select_in_tree = [&](std::shared_ptr<Tree::Node> n) {
		if (glm::length(n->position - _pos) < radius) {
			return n;
		}
		for (auto& child : n->children) {
			std::shared_ptr<Node> selected = select_in_tree(child);
			if (selected != nullptr) return selected;
		}
		std::shared_ptr<Node> temp = nullptr;
		return temp;
	};
	return select_in_tree(m_root);
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
	SetProperties();
}

//--------------------------------------------------------------
void Tree::SetProperties()
{
	leaves = Leaves(m_root);
	depth = Depth(m_root);
}

//--------------------------------------------------------------
void Tree::HandleInput()
{
}

void Tree::Update(float _delta_time)
{
	std::function<void(std::shared_ptr<Node>)> Move = [&](std::shared_ptr<Node> n) {
		if (glm::length(n->position - n->new_position) < 2 * speed) n->position = n->new_position;
		else n->position += normalize(n->new_position - n->position) * speed;
		for (auto child : n->children) Move(child);
	};
	Move(m_root);
}

//--------------------------------------------------------------
void Tree::Draw()
{
	ofSetDrawBitmapMode(OF_BITMAPMODE_SIMPLE);

	// Draw radial circles
	ofNoFill();
	ofSetColor(65);
	for (int i = 0; i < depth - 1; i++)
	{
		ofDrawCircle(glm::vec3(0), 100 + i * 150);
	}

	// Draw nodes and edges
	ofFill();
	ofSetColor(255, 0, 0);
	ofDrawCircle(m_root->position, radius);
	ofDrawBitmapStringHighlight(ofToString(m_root->vertex_idx), m_root->position + glm::vec3(10, 10, -1));
	ofSetColor(255);
	std::stack<std::shared_ptr<Node>> stack;
	for (auto& child : m_root->children) stack.push(child);

	while (stack.empty() == false) {
		auto node = stack.top(); stack.pop();
		auto parent = node->parent;
		static glm::vec3 sub = { 0, 0, -1 }; // To draw edge behind nodes
		ofSetColor(123);
		ofDrawLine(parent->position + sub, node->position + sub);

		ofSetColor(255);
		ofDrawCircle(node->position, radius);
		//ofDrawBitmapStringHighlight( ofToString( node->subtree_count ), node->position + glm::vec3( 10, 10, -1 ) );
		ofDrawBitmapStringHighlight(ofToString(node->vertex_idx), node->position + glm::vec3(10, 10, -1));

		for (auto& child : node->children) stack.push(child);
	}
}

void Tree::Gui()
{
}

std::vector<std::reference_wrapper<ILayout::Node>> Tree::Nodes()
{
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
	static auto nodes = Wrap(m_root);
	return nodes;
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
	node->new_position.x = 0; node->new_position.y = 0;
	RadialSubTree(*node, 0, TWO_PI, 0, _step, _delta_angle);
}

void Tree::Layout::RadialSubTree(Tree::Node& _node, float _wedge_start, float _wedge_end, int _depth, float _step, float _delta_angle)
{
	float new_wedge_start = _wedge_start;
	float radius = _step + (_delta_angle * _depth);
	float parent_leaves = Tree::Leaves(std::make_shared<Tree::Node>(_node));
	std::cout << "parent_leaves " << parent_leaves << std::endl;
	for (auto& child : _node.children) {
		float child_leaves = Tree::Leaves(child);
		std::cout << "child_leaves " << child_leaves << std::endl;

		float new_wedge_end = new_wedge_start + (child_leaves / parent_leaves * (_wedge_end - _wedge_start));
		float angle = (new_wedge_start + new_wedge_end) * .5f;
		child->new_position.x = radius * glm::cos(angle);
		child->new_position.y = radius * glm::sin(angle);

		if (child->children.size() > 0)
			RadialSubTree(*child, new_wedge_start, new_wedge_end, _depth + 1, _step, _delta_angle);
		new_wedge_start = new_wedge_end;
	}
}


} // DataVis
