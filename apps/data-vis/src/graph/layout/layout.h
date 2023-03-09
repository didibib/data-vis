#pragma once

namespace DataVis
{
class Layout
{
	static const char* __RANDOM;
	static const char* __GRID;
public:
	struct Data
	{
		virtual po::options_description Options() = 0;
		virtual ~Data() {}
	};
	
	struct RandomData : Data
	{
		int width = 800;
		float height = 600;
		po::options_description Options() override
		{
			po::options_description desc(__RANDOM);
			desc.add_options()
				("w", po::value(&width), "Range width, default = 800")
				("h", po::value(&height), "Range Height, default = 600");
			return desc;
		}
	};

	struct GridData : Data
	{
		int width = 800;
		float height = 600;
		float step = 100;
		po::options_description Options() override
		{
			po::options_description desc(__GRID);
			desc.add_options()
				("w", po::value(&width), "Width, default = 800")
				("h", po::value(&height), "Height, default = 600")
				("s", po::value(&step), "Step, default = 100");
			return desc;
		}
	};

	//--------------------------------------------------------------
	static std::unordered_map<std::string, std::string>& Descriptions();
	static const std::vector <std::pair<std::string, std::function<void(IStructure&, std::string)>>>& Functions();
	static void RandomCmdline(IStructure&, std::string);
	static void GridCmdline(IStructure&, std::string);

	static void Random(IStructure&, int width, int height);
	static void Grid(IStructure&, int width, int height, float step);
private:
	static std::unordered_map<std::string, std::string> InitDescriptions();
};
} // namespace DataVis