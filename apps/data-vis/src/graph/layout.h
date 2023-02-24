#pragma once

namespace DataVis
{
namespace po = boost::program_options;
class ILayout
{
	static const char* __RANDOM;
	static const char* __GRID;
	//--------------------------------------------------------------
protected:
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
			po::options_description desc(__RANDOM);
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
			po::options_description desc(__GRID);
			desc.add_options()
				("w", po::value(&width), "Width, default = 800")
				("h", po::value(&height), "Height, default = 600")
				("s", po::value(&step), "Step, default = 100");
			return desc;
		}
	};

	//--------------------------------------------------------------
public:
	struct Node {
	public:
		Node(int _vertex_idx, glm::vec3 _position = glm::vec3(0)) {
			vertex_idx = _vertex_idx;
			position = _position;
		}
		int vertex_idx = -1;
		glm::vec3 position = glm::vec3(0);
	};

	//--------------------------------------------------------------
	ILayout();
	virtual ~ILayout();
	const int& Idx();
	virtual void HandleInput() = 0;
	virtual void Update(float delta_time) = 0;
	virtual void Draw() = 0;
	virtual void Gui() = 0;
	virtual std::vector<std::reference_wrapper<ILayout::Node>> Nodes() = 0;

private:
	static int __idx;
	int m_idx = 0;

	//--------------------------------------------------------------
private:
	static std::unordered_map<std::string, std::string> InitLayoutDescriptions();
public:
	static std::unordered_map<std::string, std::string>& LayoutDescriptions();
	static const std::vector <std::pair<std::string, std::function<void(ILayout&, std::string)>>>& LayoutFunctions();
	static void RandomCmdline(ILayout&, std::string);
	static void GridCmdline(ILayout&, std::string);

	static void Random(ILayout&, int width, int height);
	static void Grid(ILayout&, int width, int height, float step);
};
} // DataVis
