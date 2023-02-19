#pragma once

namespace DataVis {
	namespace po = boost::program_options;
	class Layout {
		static const char* RANDOM;
		static const char* GRID;

		struct RandomData {
		public:
			int width = 1, height = 1;
			po::options_description Options() {
				po::options_description desc("Random");
				desc.add_options()
					("w", po::value(&width), "Range width")
					("h", po::value(&height), "Range Height");
				return desc;
			}
		};

		struct GridData {
		public:
			int width = 1, height = 1;
			float step = 1;
			po::options_description Options() {
				po::options_description desc("Grid");
				desc.add_options()
					("w", po::value(&width), "Width")
					("h", po::value(&height), "Height")
					("s", po::value(&step), "Step");
				return desc;
			}
		};

	public:
		static auto& LayoutFunctions() {
			static std::vector <std::pair<std::string, std::function<void(DataVis::Graph&, std::string)>>> layout_functions = {
				{ RANDOM, DataVis::Layout::RandomCmdline },
				{ GRID, DataVis::Layout::GridCmdline }
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

		static void RandomCmdline(Graph&, std::string);
		static void GridCmdline(Graph&, std::string);
		static void Random(Graph&, int width, int height);
		static void Grid(Graph&, int width, int height, float step);
	};
}
