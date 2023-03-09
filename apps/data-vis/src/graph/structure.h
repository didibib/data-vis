#pragma once

namespace DataVis
{
//--------------------------------------------------------------
namespace po = boost::program_options;
class IStructure
{
	static const char* __RANDOM;
	static const char* __GRID;
protected:
	struct Data {
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
public:
	class Node {
	public:
		Node(std::string vertex_id, VertexIdx vertex_index, glm::vec3 position = glm::vec3(0));
		void EaseInEaseOut(float t, float speed = .2f);

		const std::string& GetVertexId() const;
		VertexIdx GetVertexIdx() const;
		const glm::vec3& GetPosition();
		void SetPosition(glm::vec3& position);
		void SetNewPosition(glm::vec3& new_position);
		const float& GetRadius();
		void SetRadius(float radius);
		bool Inside(glm::vec3 position);

		ofColor color;
		std::vector<std::shared_ptr<Node>> neighbors;

	protected:
		std::string m_vertex_id;
		VertexIdx m_vertex_idx;

		glm::vec3 m_position = glm::vec3(0);
		glm::vec3 m_new_position = glm::vec3(0);
		glm::vec3 m_old_position = glm::vec3(0);
		ofRectangle m_bounding_box;
		float m_radius = 10;
		float m_time = 0;
		bool m_animate = true;
	};

	//--------------------------------------------------------------
	IStructure();
	virtual ~IStructure();
	const int& Idx() const;
	Dataset& GetDataset() const;
	virtual void Init(const std::shared_ptr<Dataset>) = 0;
	virtual void HandleInput() = 0;
	virtual void Select(const glm::vec3&) = 0;
	virtual void Update(float delta_time) = 0;
	void Draw();
	virtual void Gui() = 0;
	virtual std::vector<std::shared_ptr<IStructure::Node>>& GetNodes() = 0;

	glm::vec3 GetPosition() const;
	void SetPosition(glm::vec3 new_position);
	void Move(glm::vec3 offset);
	const ofRectangle& GetAABB() const;
	const ofRectangle& GetMoveAABB() const;
	void UpdateAABB();

protected:
	std::shared_ptr<Dataset> m_dataset;
	glm::vec3 m_position;
	ofRectangle m_aabb;
	ofRectangle m_move_aabb;
	int m_move_aabb_size = 50;

	virtual void DrawLayout() = 0;
	virtual void PostBuild() = 0;
	virtual void SetAABB();
	void SetMoveAABB();

private:
	static int __idx;
	int m_idx = 0;

	//--------------------------------------------------------------
private:
	static std::unordered_map<std::string, std::string> InitLayoutDescriptions();
public:
	static std::unordered_map<std::string, std::string>& LayoutDescriptions();
	static const std::vector <std::pair<std::string, std::function<void(IStructure&, std::string)>>>& LayoutFunctions();
	static void RandomCmdline(IStructure&, std::string);
	static void GridCmdline(IStructure&, std::string);

	static void Random(IStructure&, int width, int height);
	static void Grid(IStructure&, int width, int height, float step);
};
} // namespace DataVis
