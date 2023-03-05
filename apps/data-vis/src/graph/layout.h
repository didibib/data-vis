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
			m_position = _position;
			m_old_position = _position;
			m_bounding_box = ofRectangle(_position - glm::vec2(m_radius), m_radius * 2, m_radius * 2);
		}

		void EaseInEaseOut(float _t, float _speed = .2f) {
			if (m_time > 1) {
				m_animate = false;
				m_time = 0;
			}
			if (m_animate) {
				m_time += _t * _speed;
				float p = Curve::Bezier(m_time);
				SetPosition((1 - p) * m_old_position + p * m_new_position);
				if (p >= .999f) {
					m_animate = false;
					m_time = 0;
					SetPosition(m_new_position);
					m_old_position = m_new_position;
				}
			}
		}

		const glm::vec3& GetPosition() {
			return m_position;
		}

		void SetPosition(glm::vec3& _position) {
			m_position = _position;
			m_bounding_box.setPosition(_position - glm::vec2(m_radius));
		}

		void SetNewPosition(glm::vec3& _new_position) {
			m_new_position = _new_position;
			m_animate = true;
		}

		void SetDisplacement( glm::vec3& _displacement ) {
			m_displacement = _displacement;
		}

		const glm::vec3 GetDisplacement() {
			return m_displacement;
		}

		const float& GetRadius() {
			return m_radius;
		}

		void SetRadius(float _radius) {
			m_radius = _radius;
			m_bounding_box.setSize(_radius * 2, _radius * 2);
		}

		bool Inside( glm::vec3 _position )
		{
			return length( m_position - _position ) < m_radius;
		}

		int vertex_idx = -1;
		ofColor color;
		ofRectangle m_bounding_box;
	protected:
		glm::vec3 m_position = glm::vec3(0);
		glm::vec3 m_new_position = glm::vec3(0);
		glm::vec3 m_old_position = glm::vec3(0);
		glm::vec3 m_displacement = glm::vec3(0);
		float m_radius = 10;
		float m_time = 0;
		bool m_animate = true;
	};

	//--------------------------------------------------------------
	ILayout();
	virtual ~ILayout();
	const int& Idx();
	virtual void HandleInput() = 0;
	virtual void Select( const glm::vec3&) = 0;
	virtual void Update(float delta_time) = 0;
	void Draw();
	virtual void DrawLayout() = 0;
	virtual void Gui() = 0;
	virtual std::vector<std::reference_wrapper<ILayout::Node>> Nodes() = 0;

	glm::vec3 GetPosition() { return m_position; };
	void SetPosition( glm::vec3 new_position ) { m_position = new_position; };
	void Move( glm::vec3 _offset ) { m_position += _offset; };
	const ofRectangle& GetBounds() { return m_bounds; };
	const ofRectangle& GetMoveBounds() { return m_move_bounds; };
	void UpdateBounds() { SetBounds(); SetMoveBounds(); };

protected:
	virtual void PostBuild() = 0;
	glm::vec3 m_position;

	ofRectangle m_bounds;
	virtual void SetBounds();

	ofRectangle m_move_bounds;
	int m_move_bounds_size = 50;
	void SetMoveBounds();

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
