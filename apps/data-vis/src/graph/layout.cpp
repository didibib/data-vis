#include "precomp.h"

namespace DataVis {
	const char* Layout::RANDOM = "Random";
	const char* Layout::GRID = "Grid";

	namespace po = boost::program_options;

	void Layout::Random(Graph& _graph, std::string _cmdline_input)
	{
		int width = 1, height = 1;
		// Parse input
		po::options_description desc("Random");
		desc.add_options()
			("w", po::value(&width), "Range width")
			("h", po::value(&height), "Range Height");
		po::variables_map vm;
		try {
			po::store(po::command_line_parser(po::split_unix(_cmdline_input)).options(desc).run(), vm);
		}
		catch (std::exception& e) {
			std::cout << e.what();
		}
		po::notify(vm);
		// Create layout
		auto& nodes = _graph.Nodes();
		for (size_t i = 0; i < nodes.size(); i++)
		{
			auto& position = nodes[i].m_property.position;
			float x = RandomRangeF(width);
			float y = RandomRangeF(height);
			float z = 0;
			position = glm::vec3(x, y, z);
		}
	}

	void Layout::Grid(Graph& _graph, std::string _cmdline_input)
	{
		int width = 1, height = 1;
		float step = 1;
		// Parse input
		po::options_description desc("Grid");
		desc.add_options()
			("w", po::value(&width), "Width")
			("h", po::value(&height), "Height")
			("s", po::value(&step), "Step");
		po::variables_map vm;
		try {
			po::store(po::command_line_parser(po::split_unix(_cmdline_input)).options(desc).run(), vm);
		}
		catch (std::exception& e) {
			std::cout << e.what();
		}
		po::notify(vm);
		// Create layout
		auto& nodes = _graph.Nodes();
		std::vector<glm::vec3> grid;
		// increment width and height if there are more nodes then positions
		while (nodes.size() > width * height)
			width++, height++;
		grid.reserve(width * height);
		// generate positions
		for (size_t j = 0; j < height; j++)
			for (size_t i = 0; i < width; i++)
			{
				float x = i * step;
				float y = j * step;
				float z = 0;
				grid.push_back(glm::vec3(x, y, z));
			}
		// shuffle vector
		std::shuffle(std::begin(grid), std::end(grid), random);

		for (size_t i = 0; i < nodes.size(); i++)
		{
			auto& position = nodes[i].m_property.position;
			position = grid[i];
		}
	}
}