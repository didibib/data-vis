#pragma once

namespace DataVis
{
class Layout;
namespace po = boost::program_options;

//--------------------------------------------------------------
class IStructure
{
public:
	class Node : public Animator
	{
	public:
		Node(std::string vertex_id, VertexIdx vertex_index, const glm::vec3& position = glm::vec3(0));

		[[nodiscard]] const std::string& GetVertexId() const;
		[[nodiscard]] VertexIdx GetVertexIdx() const;
		const glm::vec3& GetPosition();
		void SetPosition(const glm::vec3& position);
		const glm::vec3& GetNewPosition();
		void SetNewPosition(const glm::vec3& new_position);
		void SetDisplacement(glm::vec3 displacement);
		const glm::vec3& GetDisplacement();
		const float& GetRadius() const;
		void SetRadius(float radius);
		bool Inside(glm::vec3 position) const;

		ofColor color = ofColor::black;
		std::vector<std::shared_ptr<Node>> neighbors;

	protected:
		virtual void OnStopAnimation() override;
		virtual void Interpolate(float percentage) override;

		std::string m_vertex_id;
		VertexIdx m_vertex_idx;

		glm::vec3 m_position = glm::vec3(0);
		glm::vec3 m_new_position = glm::vec3(0);
		glm::vec3 m_old_position = glm::vec3(0);
		glm::vec3 m_displacement = glm::vec3(0);
		ofRectangle m_aabb;
		float m_radius = 10;
	};

	class Edge : public Animator
	{
	public:
		Edge() = default;
		
	protected:
		void OnStopAnimation() override;
		void Interpolate(float percentage) override;
	};

	using VectorOfNodes = std::vector<std::shared_ptr<IStructure::Node>>;

	IStructure();
	virtual ~IStructure();
	[[nodiscard]] const int& Idx() const;
	virtual void Init(const std::shared_ptr<Dataset>);
	virtual void Update(float delta_time);
	[[nodiscard]] Dataset& GetDataset() const;
	VectorOfNodes& GetNodes();
	[[nodiscard]] const glm::vec3& GetPosition() const;
	void SetPosition(const glm::vec3&);

	// AABB
	AABB GetAABB();
	void UpdateAABB();
	bool Inside(const glm::vec3&) const;
	bool InsideDraggable(const glm::vec3&) const;
	float GetArea() const;

	// Interaction
	void Draw(bool is_focussed);
	void Gui();
	void Select(const glm::vec3&);
	void Move(glm::vec3 offset);
	void SetOnDeleteCallback(std::function<void(IStructure&)> callback);
	void SetNodesColor(ofColor color);

protected:
	virtual void DrawNodes() = 0;
	virtual void NodeInfoGui();
	void SetSelectedNode(std::shared_ptr<Node> _node);

	VectorOfNodes m_nodes;
	std::vector<std::shared_ptr<Layout>> m_layouts;
	std::shared_ptr<Layout> m_active_layout;
	std::shared_ptr<Dataset> m_dataset;
	AABB m_aabb;
	glm::vec3 m_position = glm::vec3(0);
	std::shared_ptr<Node> m_selected_node;

	struct GuiData
	{
		bool checkbox_node_labels = false;
		float slider_radius = 10;
		glm::vec3 coloredit_node_color = glm::vec3(0);
		glm::vec3 coloredit_edge_color = glm::vec3(123);
	} m_gui_data;

	std::function<void(IStructure&)> m_on_delete_callback;

private:
	static int __idx;
	int m_idx = 0;


};
} // namespace DataVis
