#include "precomp.h"

namespace DataVis
{
const char* Layout::__RANDOM = "Random";
const char* Layout::__GRID = "Grid";
//--------------------------------------------------------------
// Layout
//--------------------------------------------------------------
std::unordered_map<std::string, std::string> Layout::InitDescriptions()
{
	std::unordered_map<std::string, std::string> layout_descriptions;
	std::vector<Data*> data = {
		new RandomData(),
		new GridData()
	};
	for (const auto& d : data) {
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

//--------------------------------------------------------------
std::unordered_map<std::string, std::string>& Layout::Descriptions()
{
	static std::unordered_map<std::string, std::string> layout_descriptions = Layout::InitDescriptions();
	return layout_descriptions;
}

//--------------------------------------------------------------
const std::vector <std::pair<std::string, std::function<void(IStructure&, std::string)>>>& Layout::Functions()
{
	static std::vector <std::pair<std::string, std::function<void(IStructure&, std::string)>>> layout_functions = {
		{ __RANDOM, Layout::RandomCmdline },
		{ __GRID, Layout::GridCmdline }
	};
	return layout_functions;
}

//--------------------------------------------------------------
// Random
//--------------------------------------------------------------
void Layout::RandomCmdline(IStructure& _layout, std::string _cmdline_input)
{
	static RandomData rd;
	static auto options = rd.Options();
	Parser::Cmdline(options, _cmdline_input);
	Random(_layout, rd.width, rd.height);
}

void Layout::Random(IStructure& _layout, int _width, int _height)
{
	auto& nodes = _layout.GetNodes();
	for (size_t i = 0; i < nodes.size(); i++) {
		float x = Random::RangeF(_width);
		float y = Random::RangeF(_height);
		float z = 0;
		nodes[i]->SetPosition(glm::vec3(x, y, z));
	}
	_layout.UpdateAABB();
}

//--------------------------------------------------------------
// Grid
//--------------------------------------------------------------
void Layout::GridCmdline(IStructure& _layout, std::string _cmdline_input)
{
	static GridData gd;
	static auto options = gd.Options();
	Parser::Cmdline(options, _cmdline_input);
	Grid(_layout, gd.width, gd.height, gd.step);
}

void Layout::Grid(IStructure& _layout, int _width, int _height, float _step)
{
	auto& nodes = _layout.GetNodes();
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
		nodes[i]->SetPosition(grid[i]);
	}
	_layout.UpdateAABB();
}
} // namespace DataVis