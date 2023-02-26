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

		const float& GetRadius() {
			return m_radius;
		}

		void SetRadius(float _radius) {
			m_radius = _radius;
			m_bounding_box.setSize(_radius * 2, _radius * 2);
		}

		bool Inside(const ofCamera& _camera, glm::vec3 _position) {
			glm::vec3 bottom_left = _camera.worldToScreen(m_bounding_box.getBottomLeft());
			glm::vec3 bottom_right = _camera.worldToScreen(m_bounding_box.getBottomRight());
			glm::vec3 top_right = _camera.worldToScreen(m_bounding_box.getTopRight());
			glm::vec3 top_left = _camera.worldToScreen(m_bounding_box.getTopLeft());

			ofPolyline p;
			p.addVertex(bottom_left);
			p.addVertex(bottom_right);
			p.addVertex(top_right);
			p.addVertex(top_left);

			bool inside = p.inside(_position);
			if (inside) color = ofColor::green;
			else color = ofColor::white;
			return inside;
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
		float m_radius = 10;
		float m_time = 0;
		bool m_animate = true;
	};

	//--------------------------------------------------------------
	ILayout();
	virtual ~ILayout();
	const int& Idx();
	virtual void HandleInput() = 0;
	virtual void Select( const ofCamera&, const glm::vec3& ) = 0;
	virtual void Select( const glm::vec3&) = 0;
	virtual void Update(float delta_time) = 0;
	void Draw();
	virtual void DrawLayout() = 0;
	virtual void Gui() = 0;
	virtual std::vector<std::reference_wrapper<ILayout::Node>> Nodes() = 0;

	glm::vec3 GetPosition() { return m_pos; };
	void SetPosition( glm::vec3 new_pos ) { m_pos = new_pos; };
	ofPolyline GetBounds() { return m_bounds; };

protected:
	virtual void PostBuild() = 0;
	glm::vec3 m_pos;

	ofPolyline m_bounds;
	virtual void SetBounds();

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
