#include "precomp.h"

namespace DataVis
{
void Tree::Init(const std::shared_ptr<Dataset> _dataset)
{
	IStructure::Init(_dataset);
	m_layouts.push_back(std::make_unique<Radial>());
}

std::shared_ptr<Tree::Node> Tree::Root()
{
	return m_root;
}
//--------------------------------------------------------------
// Properties
//--------------------------------------------------------------
uint Tree::Leaves(std::shared_ptr<Node> node)
{
	uint leaves = 0;
	if (node->children.size() == 0) leaves++;
	for (auto& child : node->children) leaves += Leaves(child);
	return leaves;
}

//--------------------------------------------------------------
uint Tree::Depth(std::shared_ptr<Node> node)
{
	uint max = 0;
	for (auto& child : node->children)
	{
		uint child_depth = Depth(child);
		if (child_depth > max) max = child_depth;
	}
	return max + 1;
}

//--------------------------------------------------------------
void Tree::SwapRoot(std::shared_ptr<Node> _new_root)
{
	std::shared_ptr<Node> node = _new_root;
	while (node->GetVertexIdx() != m_root->GetVertexIdx())
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
	UpdateProperties();
}

//--------------------------------------------------------------
void Tree::UpdateProperties()
{
	leaves = Leaves(m_root);
	depth = Depth(m_root);
}

// //--------------------------------------------------------------
// void Tree::SetAABB()
// {
// 	// Still uses hardcoded radius and delta_angle
// 	int r = (depth - 1) * 150;
// 	m_aabb = { {-r, -r}, {r, r} };
// }

//--------------------------------------------------------------
void Tree::DrawNodes()
{
	ofSetDrawBitmapMode(OF_BITMAPMODE_SIMPLE);

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

		//ofDrawBitmapStringHighlight(ofToString(node->GetVertexId()), node->GetPosition() + glm::vec3(10, 10, -1));

		for (auto& child : node->children) stack.push(child);
	}
}

void Tree::NodeInfoGui()
{
	if (m_selected_node != nullptr)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
		ImGui::BeginChild("SelectedNode", ImVec2(0, 85), true);
		
		ImGui::Text("Selected Node:");
		ImGui::Text("Vertex: %s", m_selected_node->GetVertexId().c_str());
		ImGui::Text("Position: (%f.0, %f.0)", m_selected_node->GetPosition().x, m_selected_node->GetPosition().y);

		if (ImGui::Button("Make root"))
		{
			SwapRoot(std::static_pointer_cast<Tree::Node>(m_selected_node));
		}

		ImGui::EndChild();
		ImGui::PopStyleVar();
	}
}

//--------------------------------------------------------------
// MSP
//--------------------------------------------------------------
void MSP::Init(const std::shared_ptr<Dataset> _dataset)
{
	Tree::Init(_dataset);
	m_nodes.resize(_dataset->GetVertices().size());
	Create(0);
}

//--------------------------------------------------------------
void MSP::Create(VertexIdx _root)
{
	auto& vertices = m_dataset->GetVertices();

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
			float weight = m_dataset->GetEdges()[neigbor.edge_idx].attributes.FindFloat("weight", 1);
			if (!included[v] && weight < costs[v]) {
				// Found a cheaper edge to v
				parents[v] = idx;
				costs[v] = weight;
			}
		}
	}

	// Construct a tree
	m_root = std::make_shared<Node>(vertices[_root].id, _root);
	m_nodes[_root] = m_root;

	// Construct recursive lambda to create the tree
	static std::function<void(std::shared_ptr<Node>, VectorOfNodes&)> MakeTree =
		[&](std::shared_ptr<Node> n, VectorOfNodes& nodes) {
		// Look through all vertices which have parent == n.vertex
		for (int i = 0; i < vertices.size(); i++)
			if (parents[i] == n->GetVertexIdx()) {
				// Add node to n.children
				auto child = std::make_shared<Node>(vertices[i].id, i, n);
				nodes[i] = child;
				n->children.push_back(child);
				MakeTree(child, nodes);
			}
	};
	MakeTree(m_root, m_nodes);
	UpdateProperties();
	UpdateAABB();
}

} // DataVis
