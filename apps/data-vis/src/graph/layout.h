#pragma once

namespace DataVis {
	namespace po = boost::program_options;
	class Layout {
		static const char* RANDOM;
		static const char* GRID;

	public:
		static auto& LayoutFunctions() {
			static std::vector <std::pair<std::string, std::function<void(DataVis::Graph&, std::string)>>> layout_functions = {
				{ RANDOM, DataVis::Layout::Random },
				{ GRID, DataVis::Layout::Grid }
			};
			return layout_functions;
		}
		static auto& LayoutDescriptions() {
			static std::unordered_map<std::string, std::string> layout_descriptions = {
				{ RANDOM, "--w [range width]\n--h [range height]" },
				{ GRID, "--w [width]\n--h [height]\n--s [step]"}
			};
			return layout_descriptions;
		}

		static void Random(Graph&, std::string);
		static void Grid(Graph&, std::string);
	};
}
