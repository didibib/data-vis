#include "precomp.h"

namespace DataVis {
	const char* Layout::RANDOM = "Random";
	const char* Layout::GRID = "Grid";
	const char* Layout::RADIAL = "Radial";
	std::unordered_map<std::string, std::string> Layout::m_layout_descriptions = Layout::InitLayoutDescriptions();

	//--------------------------------------------------------------
	std::unordered_map<std::string, std::string> Layout::InitLayoutDescriptions() {
		std::unordered_map<std::string, std::string> layout_descriptions;
		RandomData rd;
		GridData gd;
		RadialData rad;
		std::vector < po::options_description > descriptions;
		descriptions.push_back(rd.Options());
		descriptions.push_back(gd.Options());
		descriptions.push_back( rad.Options( ) );

		for (auto &desc : descriptions)
		{
			std::stringstream ss;
			desc.print(ss);
			layout_descriptions.insert({ desc.caption(), ss.str()});
		}
		return layout_descriptions;
	}

	const std::vector <std::pair<std::string, std::function<void(DataVis::Graph&, std::string)>>>& Layout::LayoutFunctions() {
		static std::vector <std::pair<std::string, std::function<void(DataVis::Graph&, std::string)>>> layout_functions = {
			{ RANDOM, DataVis::Layout::RandomCmdline },
			{ GRID, DataVis::Layout::GridCmdline }
		};
		return layout_functions;
	}

	std::unordered_map<std::string, std::string>& Layout::LayoutDescriptions() {
		return m_layout_descriptions;
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
	void Layout::Radial(Tree&, float radius) {

	}
}