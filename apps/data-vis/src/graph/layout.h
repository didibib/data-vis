#pragma once

namespace DataVis
{
namespace po = boost::program_options;
class Layout
{
	static const char* RANDOM;
	static const char* GRID;
	static const char* RADIAL;

	//--------------------------------------------------------------
	struct Data {
		virtual po::options_description Options() = 0;
		virtual ~Data() {}
	};
	struct RandomData : Data
	{
	public:
		int width = 1, height = 1;
		po::options_description Options() override
		{
			po::options_description desc(RANDOM);
			desc.add_options()
				("width, w", po::value(&width), "Range width")
				("height, h", po::value(&height), "Range Height");
			return desc;
		}
	};

	struct GridData : Data
	{
	public:
		int width = 1, height = 1;
		float step = 1;
		po::options_description Options() override
		{
			po::options_description desc(GRID);
			desc.add_options()
				("width, w", po::value(&width), "Width")
				("height, h", po::value(&height), "Height")
				("step, s", po::value(&step), "Step");
			return desc;
		}
	};

	struct RadialData : Data
	{
	public:
		float radius;
		po::options_description Options() override
		{
			po::options_description desc(GRID);
			desc.add_options()
				("radius, r", po::value(&radius), "Radius");
			return desc;
		}
	};

	//--------------------------------------------------------------
	static std::unordered_map<std::string, std::string> InitLayoutDescriptions();
	static void RadialSubTree(Tree::Node&, float radius, float angle_start, float angle_end);

public:
	static std::unordered_map<std::string, std::string>& LayoutDescriptions();
	static const std::vector <std::pair<std::string, std::function<void(Graph&, std::string)>>>& GraphLayoutFunctions();
	static const std::vector <std::pair<std::string, std::function<void(Tree&, std::string)>>>& TreeLayoutFunctions();

	//--------------------------------------------------------------
	static void RandomCmdline(Graph&, std::string);
	static void GridCmdline(Graph&, std::string);

	static void Random(Graph&, int width, int height);
	static void Grid(Graph&, int width, int height, float step);

	//--------------------------------------------------------------
	static void RadialCmdline(Tree&, std::string);
	static void Radial(Tree&, float radius);
};
}
