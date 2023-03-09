#include "precomp.h"

namespace DataVis
{
const char* Tree::__RADIAL = "Radial";

//--------------------------------------------------------------
void Tree::Init(const std::shared_ptr<Dataset> _dataset)
{
	m_dataset = _dataset;
}

//--------------------------------------------------------------
int Tree::Leaves(std::shared_ptr<Node> node)
{
	int leaves = 0;
	if (node->children.size() == 0) leaves++;
	for (auto& child : node->children) leaves += Leaves(child);
	return leaves;
}

//--------------------------------------------------------------
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
}

//--------------------------------------------------------------
void Tree::HandleInput()
{
}

//--------------------------------------------------------------
void Tree::Select(const glm::vec3& _position)
{
	// Loop through all vertices and find one within radius of pos
	static std::function<std::shared_ptr<Tree::Node>(std::shared_ptr<Node>)> SelectInTree = [&](std::shared_ptr<Tree::Node> n)
	{
		if (n->Inside(_position)) return n;
		for (auto& child : n->children)
		{
			std::shared_ptr<Tree::Node> selected = SelectInTree(child);
			if (selected != nullptr) return selected;
		}
		return std::shared_ptr<Tree::Node>();
	};
	SetSelectedNode(SelectInTree(m_root));
}

//--------------------------------------------------------------
void Tree::SetSelectedNode(std::shared_ptr<Tree::Node> n)
{
	if (m_selected_node != nullptr)
		m_selected_node->color = ofColor::white;
	if (n != nullptr) n->color = ofColor::green;
	m_selected_node = n;
}

//--------------------------------------------------------------
void Tree::SetAABB()
{
	// Still uses hardcoded radius and delta_angle
	int r = (depth - 1) * 150;
	m_aabb = { {-r, -r}, {r, r} };
	//m_bounds.clear();
	//m_bounds.addVertex( { -r,r,0 } );
	//m_bounds.addVertex( { r, r, 0 } );
	//m_bounds.addVertex( { r, -r, 0 } );
	//m_bounds.addVertex( { -r, -r, 0 } );
	//m_bounds.addVertex( { -r, r, 0 } );
}

//--------------------------------------------------------------
void Tree::Update(float _delta_time)
{
	// TODO: Check if we should animate before traversing
	static std::function<void(std::shared_ptr<Node>)> EaseInEaseOut = [&](std::shared_ptr<Node> n) {
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

	// Draw vertices and edges
	ofFill();
	ofSetColor(255, 0, 0);
	ofDrawCircle(m_root->GetPosition(), m_root->GetRadius());
	ofDrawBitmapStringHighlight(ofToString(m_root->GetVertexId()), m_root->GetPosition() + glm::vec3(10, 10, -1));

	std::stack<std::shared_ptr<Node>> stack;
	for (auto& child : m_root->children) stack.push(child);

	while (stack.empty() == false) {
		auto node = stack.top(); stack.pop();
		auto parent = node->parent;
		static glm::vec3 sub = { 0, 0, -1 }; // To draw edge behind vertices
		ofFill();
		ofSetColor(123);
		ofDrawLine(parent->GetPosition() + sub, node->GetPosition() + sub);

		ofSetColor(node->color);
		ofDrawCircle(node->GetPosition(), node->GetRadius());

		ofDrawBitmapStringHighlight(ofToString(node->GetVertexId()), node->GetPosition() + glm::vec3(10, 10, -1));

		for (auto& child : node->children) stack.push(child);
	}
}

//--------------------------------------------------------------
void Tree::Gui()
{
	if (m_selected_node.get() != nullptr)
	{
		auto x = m_selected_node.get();
		ImGui::Begin("Selected Node");

		ImGui::Text("Vertex: %i", m_selected_node->GetVertexId());
		ImGui::Text("Position: (%f.0, %f.0)", m_selected_node->GetPosition().x, m_selected_node->GetPosition().y);

		if (ImGui::Button("Make root"))
		{
			SwapRoot(m_selected_node);
			Tree::Layout::Radial(*this, 100, 150);
			UpdateAABB();
		}
		ImGui::End();
	}
}

//--------------------------------------------------------------
// Extract
//--------------------------------------------------------------
void Tree::Extract::MSP(Tree& _tree, VertexIdx _root)
{
	auto& vertices = _tree.GetDataset().GetVertices();

	// Keep track of the parent of each vertex so we can construct a tree after
	std::vector<VertexIdx> parents(vertices.size(), 0);
	parents[_root] = -1;
	// Keep track of which vertex are already processed
	std::vector<bool> included(vertices.size(), false);
	// Keep track of the minimum edge cost of a vertex
	std::vector<float> costs(vertices.size(), 1e30f);
	costs[_root] = 0;

	// Graph will have |vertices| vertices
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
		assert(idx not_eq -1);
		included[idx] = true;

		// Update outgoing edges from this vertex
		for (auto& neigbor : *vertices[idx].neighbors) {
			uint v = neigbor.to_idx;
			float weight = _tree.GetDataset().GetEdges()[neigbor.edge_idx].attributes.FindFloat("weight", 1);
			if (!included[v] && weight < costs[v]) {
				// Found a cheaper edge to v
				parents[v] = idx;
				costs[v] = weight;
			}
		}
	}

	// Construct a tree
	_tree.m_root = std::make_shared<Node>(vertices[_root].id, _root);

	// Construct recursive lambda to create the tree
	static std::function<void(std::shared_ptr<Node>)> MakeTree = [&](std::shared_ptr<Node> n) {
		// Look through all vertices which have parent == n.vertex
		for (int i = 0; i < vertices.size(); i++)
			if (parents[i] == n->GetVertexIdx()) {
				// Add node to n.children
				auto child = std::make_shared<Node>(vertices[i].id, i, n);
				n->children.push_back(child);
				MakeTree(child);
			}
	};
	MakeTree(_tree.m_root);
	_tree.PostBuild();
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
	Parser::Cmdline(options, _cmdline_input);
	Radial(_tree, rd.step, rd.delta_angle);
}

//--------------------------------------------------------------
void Tree::Layout::Radial(Tree& _tree, float _step, float _delta_angle)
{
	auto& node = _tree.Root();
	node->SetNewPosition(glm::vec3(0));
	RadialSubTree(*node, 0, TWO_PI, 0, _step, _delta_angle);
}

//--------------------------------------------------------------
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
