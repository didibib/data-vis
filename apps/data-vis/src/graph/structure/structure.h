#pragma once

namespace DataVis
{
class Layout;
namespace po = boost::program_options;

//--------------------------------------------------------------
class IStructure
{
public:
	class Node
	{
	public:
		Node(std::string vertex_id, VertexIdx vertex_index, glm::vec3 position = glm::vec3(0));
		void EaseInEaseOut(float t, float speed = .2f);

		const std::string& GetVertexId() const;
		VertexIdx GetVertexIdx() const;
		const glm::vec3& GetPosition();
		void SetPosition(glm::vec3& position);
		const glm::vec3& GetNewPosition();
		void SetNewPosition(glm::vec3& new_position);
		void SetDisplacement(glm::vec3 displacement);
		const glm::vec3& GetDisplacement();
		const float& GetRadius();
		void SetRadius(float radius);
		bool Inside(glm::vec3 position);

		ofColor color = ofColor::black;
		std::vector<std::shared_ptr<Node>> neighbors;

	protected:
		std::string m_vertex_id;
		VertexIdx m_vertex_idx;

		glm::vec3 m_position = glm::vec3(0);
		glm::vec3 m_new_position = glm::vec3(0);
		glm::vec3 m_old_position = glm::vec3(0);
		glm::vec3 m_displacement = glm::vec3(0);
		ofRectangle m_bounding_box;
		float m_radius = 10;
		float m_time = 0;
		bool m_animate = true;
	};

	using VectorOfNodes = std::vector<std::shared_ptr<IStructure::Node>>;
	//--------------------------------------------------------------
	IStructure();
	virtual ~IStructure();
	const int& Idx() const;
	Dataset& GetDataset() const;
	VectorOfNodes& GetNodes();
	glm::vec3 GetPosition() const;
	void SetPosition(glm::vec3 new_position);
	virtual void Init(const std::shared_ptr<Dataset>);
	virtual void Update(float delta_time);

	void Draw(bool is_focussed);
	void Gui();
	void Select(const glm::vec3&);
	void Move(glm::vec3 offset);
	const ofRectangle& GetAABB() const;
	const ofRectangle& GetMoveAABB() const;
	bool InsideAABB(glm::vec3 position);
	bool InsideMoveAABB(glm::vec3 position);
	void UpdateAABB();
	void SetOnDeleteCallback(std::function<void(IStructure&)> callback);

protected:
	VectorOfNodes m_nodes;
	std::vector<std::shared_ptr<Layout>> m_layouts;
	std::shared_ptr<Layout> m_active_layout;
	std::shared_ptr<Dataset> m_dataset;
	glm::vec3 m_position = glm::vec3(0);
	ofRectangle m_aabb;
	ofRectangle m_move_aabb;
	int m_move_aabb_size = 50;
	std::shared_ptr<Node> m_selected_node;

	struct GuiData
	{
		bool checkbox_node_labels = false;
		float slider_radius = 10;
	} m_gui_data;

	std::function<void(IStructure&)> m_on_delete_callback;

	virtual void DrawNodes() = 0;
	virtual void NodeInfoGui();

	virtual void SetAABB();
	void SetMoveAABB();
	void SetSelectedNode(std::shared_ptr<Node> _node);

private:
	static int __idx;
	int m_idx = 0;


};
} // namespace DataVis
