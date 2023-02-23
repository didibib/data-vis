#include "precomp.h"

namespace DataVis {
const char* Layout::RANDOM = "Random";
const char* Layout::GRID = "Grid";
const char* Layout::RADIAL = "Radial";

//--------------------------------------------------------------
std::unordered_map<std::string, std::string> Layout::InitLayoutDescriptions() {
	std::unordered_map<std::string, std::string> layout_descriptions;
	std::vector<Data*> data = {
		new RandomData(),
		new GridData(),
		new RadialData()
	};
	for (auto& d : data)
	{
		auto& desc = d->Options();
		std::stringstream ss;
		desc.print(ss);
		layout_descriptions.insert({ desc.caption(), ss.str() });
	}
	// Free memory
	for (auto& d : data) delete d;
	data.clear();

	return layout_descriptions;
}

std::unordered_map<std::string, std::string>& Layout::LayoutDescriptions() {
	static std::unordered_map<std::string, std::string> layout_descriptions = Layout::InitLayoutDescriptions();
	return layout_descriptions;
}

const std::vector <std::pair<std::string, std::function<void(Graph&, std::string)>>>& Layout::GraphLayoutFunctions() {
	static std::vector <std::pair<std::string, std::function<void(Graph&, std::string)>>> layout_functions = {
		{ RANDOM, Layout::RandomCmdline },
		{ GRID, Layout::GridCmdline }
	};
	return layout_functions;
}
const std::vector <std::pair<std::string, std::function<void(Tree&, std::string)>>>& Layout::TreeLayoutFunctions() {
	static std::vector <std::pair<std::string, std::function<void(Tree&, std::string)>>> layout_functions = {
		{ RADIAL, Layout::RadialCmdline }
	};
	return layout_functions;
}

//--------------------------------------------------------------
void Layout::RandomCmdline(Graph& _graph, std::string _cmdline_input)
{
	static RandomData rd;
	static auto options = rd.Options();
	ParseCmdline(options, _cmdline_input);
	Random(_graph, rd.width, rd.height);
}

void Layout::Random(Graph& _graph, int _width, int _height) {
	auto& nodes = _graph.Vertices();
	for (size_t i = 0; i < nodes.size(); i++)
	{
		auto& current_position = nodes[i].m_property.current_position;
		float x = Random::RangeF(_width);
		float y = Random::RangeF(_height);
		float z = 0;
		current_position = glm::vec3(x, y, z);
	}
}

//--------------------------------------------------------------
void Layout::GridCmdline(Graph& _graph, std::string _cmdline_input)
{
	static GridData gd;
	static auto options = gd.Options();
	ParseCmdline(options, _cmdline_input);
	Grid(_graph, gd.width, gd.height, gd.step);
}

void Layout::Grid(Graph& _graph, int _width, int _height, float _step) {
	auto& nodes = _graph.Vertices();
	std::vector<glm::vec3> grid;
	// Increment width and height if there are more nodes then positions
	while (nodes.size() > _width * _height) _width++, _height++;
	grid.reserve(_width * _height);
	// Generate positions
	for (size_t j = 0; j < _height; j++)
		for (size_t i = 0; i < _width; i++)
		{
			float x = i * _step;
			float y = j * _step;
			float z = 0;
			grid.push_back(glm::vec3(x, y, z));
		}
	// Shuffle vector
	std::shuffle(std::begin(grid), std::end(grid), Random::MT19937);
	// Assign positions
	for (size_t i = 0; i < nodes.size(); i++)
	{
		auto& current_position = nodes[i].m_property.current_position;
		current_position = grid[i];
	}
}

//--------------------------------------------------------------
void Layout::RadialCmdline(Tree& _tree, std::string _cmdline_input) {
	static RadialData rd;
	static auto options = rd.Options();
	ParseCmdline(options, _cmdline_input);
	Radial(_tree, rd.step);
}

void Layout::Radial(Tree& _tree, float _step) {
	auto& node = _tree.Root();
	node.position.x = 0; node.position.y = 0;
	RadialSubTree(node, 0, TWO_PI, 0, 100);
}

void Layout::RadialSubTree(Tree::Node& _node, float _alpha, float _beta, int _depth, float _step) {
	std::function<int(std::shared_ptr < Tree::Node>)> BFS = [&](std::shared_ptr < Tree::Node> node) {
		std::set<int> visited;
		std::queue<std::shared_ptr<Tree::Node>> queue;
		visited.insert(node->vertex);
		queue.push(node);
		int leaves = 0;
		while (queue.empty() == false) {
			auto& node = queue.front(); queue.pop();
			if (node->children.size() == 0) leaves++;
			for (auto& child : node->children) {
				if (visited.find(child->vertex) != visited.end()) {
					visited.insert(child->vertex);
					queue.push(child);
				}
			}
		}
		return leaves;
	};

	float theta = _alpha;
	float radius = _step + (TWO_PI * _depth);
	int leaves = BFS(std::make_shared<Tree::Node>(_node));
	for (auto& child : _node.children) {
		int lambda = BFS(child);

		float mi = theta + (lambda / leaves * (_beta - _alpha));
		float angle = (theta + mi) * .5f;
		child->position.x = radius * glm::cos(angle);
		child->position.y = radius * glm::sin(angle);

		for (auto& subchild : child->children)
			RadialSubTree(*subchild, theta, mi, _depth + 1, _step);
		theta = mi;
	}
}
} // DataVis