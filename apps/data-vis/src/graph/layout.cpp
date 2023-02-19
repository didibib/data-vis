#include "precomp.h"

namespace DataVis {
	const char* Layout::RANDOM = "Random";
	const char* Layout::GRID = "Grid";

	namespace po = boost::program_options;

	//--------------------------------------------------------------
	void Layout::RandomCmdline(Graph& _graph, std::string _cmdline_input)
	{
		RandomData rd;
		static auto options = rd.Options();
		ParseCmdline(options, _cmdline_input);
		Random(_graph, rd.width, rd.height);
	}

	void Layout::Random(Graph& _graph, int _width, int _height) {
		auto& nodes = _graph.Nodes();
		for (size_t i = 0; i < nodes.size(); i++)
		{
			auto& position = nodes[i].m_property.position;
			float x = RandomRangeF(_width);
			float y = RandomRangeF(_height);
			float z = 0;
			position = glm::vec3(x, y, z);
		}
	}

	//--------------------------------------------------------------
	void Layout::GridCmdline(Graph& _graph, std::string _cmdline_input)
	{
		GridData gd;
		static auto options = gd.Options();
		ParseCmdline(options, _cmdline_input);
		Grid(_graph, gd.width, gd.height, gd.step);
	}

	void Layout::Grid(Graph& _graph, int _width, int _height, float _step) {
		auto& nodes = _graph.Nodes();
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
			auto& position = nodes[i].m_property.position;
			position = grid[i];
		}
	}
}