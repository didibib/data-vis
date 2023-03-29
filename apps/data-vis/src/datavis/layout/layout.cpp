#include "precomp.h"

namespace DataVis
{
//--------------------------------------------------------------
// Random
//--------------------------------------------------------------
#pragma region Random
bool RandomLayout::Gui(IStructure& _structure)
{
	bool active = false;
	if (ImGui::TreeNode("Random Layout"))
	{
		ImGui::InputInt("Width", &m_width);
		ImGui::InputInt("Height", &m_height);

		if (ImGui::Button("Apply"))
		{
			Apply(_structure, m_width, m_height);
			active = true;
		}

		ImGui::TreePop();
		ImGui::Separator();
	}
	return active;
}

//--------------------------------------------------------------
void RandomLayout::Apply(IStructure& _structure, int _width, int _height)
{
	auto& nodes = _structure.nodes;
	for (size_t i = 0; i < nodes.size(); i++) {
		float x = Random::RangeF(_width);
		float y = Random::RangeF(_height);
		float z = 0;
		nodes[i]->SetNewPosition(glm::vec3(x, y, z));
	}
	_structure.UpdateAABB();
	_structure.UpdateEdges();
}
#pragma endregion // Random

//--------------------------------------------------------------
// Grid
//--------------------------------------------------------------
#pragma region Grid
bool GridLayout::Gui(IStructure& _structure)
{
	bool active = false;
	if (ImGui::TreeNode("Grid Layout"))
	{
		ImGui::InputInt("Width", &m_width);
		ImGui::InputInt("Height", &m_height);
		ImGui::InputFloat("Step", &m_step);

		if (ImGui::Button("Apply"))
		{
			Apply(_structure, m_width, m_height, m_step);
			active = true;
		}

		ImGui::TreePop();
		ImGui::Separator();
	}
	return active;
}

//--------------------------------------------------------------
void GridLayout::Apply(IStructure& _structure, int _width, int _height, float _step)
{
	const auto& nodes = _structure.nodes;
	std::vector<glm::vec3> grid;
	// Increment width and height if there are more nodes then positions
	while (nodes.size() > _width * _height) _width++, _height++;
	grid.reserve(_width * _height);
	// Generate positions
	for (size_t j = 0; j < _height; j++)
		for (size_t i = 0; i < _width; i++) {
			float x = i * _step;
			float y = j * _step;
			float z = 0;
			grid.push_back(glm::vec3(x, y, z));
		}
	// Shuffle vector
	std::shuffle(std::begin(grid), std::end(grid), Random::MT19937);
	// Assign positions
	for (size_t i = 0; i < nodes.size(); i++) {
		nodes[i]->SetNewPosition(grid[i]);
	}
	_structure.UpdateAABB();
	_structure.UpdateEdges();
}
#pragma endregion // Grid


//--------------------------------------------------------------
// Radial
//--------------------------------------------------------------
#pragma region Radial
bool RadialLayout::Gui(IStructure& _structure)
{
	bool active = false;
	if (ImGui::TreeNode("Radial Layout"))
	{
		ImGui::InputFloat("Start", &m_start);
		ImGui::InputFloat("Step", &m_step);

		if (ImGui::Button("Apply"))
		{
			try {
				auto& tree = dynamic_cast<Tree&>(_structure);
				Apply(tree, m_start, m_step);
				m_rings.Set(tree.Depth(tree.Root()), m_start, m_step);
				active = true;
			}
			catch (std::exception& e) {
				std::cout << e.what() << std::endl;
			}
		}

		ImGui::TreePop();
		ImGui::Separator();
	}
	return active;
}

void RadialLayout::Update(float _delta_time)
{
	m_rings.Update(_delta_time);
}

void RadialLayout::Draw()
{
	m_rings.Draw();
}

//--------------------------------------------------------------
void RadialLayout::Apply(Tree& _tree, float _start, float _step)
{
	auto& node = _tree.Root();
	node->SetNewPosition(glm::vec3(0));
	SubTree(*node, 0, TWO_PI, 0, _start, _step);
	_tree.UpdateAABB();
	_tree.UpdateEdges();
}

//--------------------------------------------------------------
void RadialLayout::SubTree(Tree::TreeNode& _node, float _wedge_start, float _wedge_end, int _depth, float _start, float _step)
{
	float new_wedge_start = _wedge_start;
	const float radius = _start + (_step * _depth);
	const uint parent_leaves = Tree::Leaves(std::make_shared<Tree::TreeNode>(_node));
	for (auto& child : _node.children) {
		const uint child_leaves = Tree::Leaves(child);
		const float new_wedge_end = new_wedge_start +
			static_cast<float>(child_leaves) / static_cast<float>(parent_leaves) *
				(_wedge_end - _wedge_start);
		const float angle = (new_wedge_start + new_wedge_end) * .5f;
		child->SetNewPosition(glm::vec3(radius * glm::cos(angle), radius * glm::sin(angle), 0));
		if (not child->children.empty())
			SubTree(*child, new_wedge_start, new_wedge_end, _depth + 1, _start, _step);
		new_wedge_start = new_wedge_end;
	}
}
#pragma endregion // Radial

//--------------------------------------------------------------
// Force Directed
//--------------------------------------------------------------
#pragma region Force Directed
bool ForceDirectedLayout::Gui(IStructure& _structure)
{
	bool active = false;
	if (ImGui::TreeNode("Force Directed Layout"))
	{
		ImGui::InputFloat("C", &m_C);
		ImGui::InputFloat("T", &m_T);
		ImGui::InputInt("Iterations/Frame", &m_iterations);

		ImGui::Checkbox("Enabled", &m_enabled);

		ImGui::TreePop();
		ImGui::Separator();
	}
	if (m_enabled) {
		Apply(_structure, m_C, m_T, m_iterations);
		_structure.UpdateEdges(true);
		active = true;
	}
	return active;
}

//--------------------------------------------------------------
void ForceDirectedLayout::Apply(IStructure& _structure, float _C, float _t, int _iterations)
{
	const float area = _structure.GetArea();
	const float k = _C * sqrtf(area / _structure.nodes.size());
	const float k2 = k * k;

	for (int i = 0; i < _iterations; i++)
	{
		// Repulsion
		for (const auto& v : _structure.nodes)
		{
			v->SetDisplacement(glm::vec3(0));
			for (const auto& u : _structure.nodes)
			{
				if (v->GetVertexIdx() == u->GetVertexIdx()) continue;

				glm::vec3 delta = v->GetPosition() - u->GetPosition();
				const float delta_l = length(delta);
				const float fr = k2 / delta_l;
				v->SetDisplacement(v->GetDisplacement() + (delta / delta_l) * fr);
			}
		}

		// Attraction
		for (const auto& e : _structure.dataset->edges)
		{
			const auto& v = _structure.nodes[e.from_idx];
			const auto& u = _structure.nodes[e.to_idx];
			glm::vec3 delta = v->GetPosition() - u->GetPosition();
			const float delta_l = length(delta);
			glm::vec3 offset = (delta / delta_l) * (delta_l * delta_l / k);
			v->SetDisplacement(v->GetDisplacement() - offset);
			u->SetDisplacement(u->GetDisplacement() + offset);
		}

		for (const auto& node : _structure.nodes)
		{
			glm::vec3 new_pos = node->GetPosition() + node->GetDisplacement() * _t;
			node->SetPosition( _structure.GetAABB().Clamp( new_pos ) );
			//node->SetPosition(node->GetPosition() + node->GetDisplacement() * _t);
		}
		_t *= 0.9999f;
	}
}
#pragma endregion // Force Directed
} // namespace DataVis