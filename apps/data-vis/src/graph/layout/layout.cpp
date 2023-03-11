#include "precomp.h"

namespace DataVis
{
const char* Layout::__RANDOM = "Random";
const char* Layout::__GRID = "Grid";

//--------------------------------------------------------------
// Random
//--------------------------------------------------------------
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