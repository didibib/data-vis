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
		int width = 800;
		float height = 600;
		po::options_description Options() override
		{
			po::options_description desc(RANDOM);
			desc.add_options()
				("w", po::value(&width), "Range width, default = 800")
				("h", po::value(&height), "Range Height, default = 600");
			return desc;
		}
	};

	struct GridData : Data
	{
	public:
		int width = 800;
		float height = 600;
		float step = 100;
		po::options_description Options() override
		{
			po::options_description desc(GRID);
			desc.add_options()
				("w", po::value(&width), "Width, default = 800")
				("h", po::value(&height), "Height, default = 600")
				("s", po::value(&step), "Step, default = 100");
			return desc;
		}
	};

	struct RadialData : Data
	{
	public:
		float step = 100;
		float delta_angle = 100;
		po::options_description Options() override
		{
			po::options_description desc(GRID);
			desc.add_options()
				("r", po::value(&step), "Radius of the innermost concentric circle, default = 100")
				("d", po::value(&delta_angle), "Delta angle (degrees) constant for the drawing’s concentric circles, default = 100" );
			return desc;
		}
	};

	//--------------------------------------------------------------
	static std::unordered_map<std::string, std::string> InitLayoutDescriptions();
	// Pavlo, 2006 https://scholarworks.rit.edu/cgi/viewcontent.cgi?referer=&httpsredir=1&article=1355&context=theses
	static void RadialSubTree(Tree::Node&, float angle_start, float angle_end, int depth, float step, float delta_angle);

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
	static void Radial(Tree&, float step, float delta_angle );
};
}
