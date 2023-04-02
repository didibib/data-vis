#pragma once

namespace DataVis
{
class ILayout;
namespace po = boost::program_options;

//--------------------------------------------------------------
class IStructure
{
public:
	IStructure();
	virtual ~IStructure();
	[[nodiscard]] const int& Idx() const;
	virtual void Init(const std::shared_ptr<Dataset>);
	virtual void Update(const float delta_time);
	[[nodiscard]] const glm::vec3& GetPosition() const;
	void SetPosition(const glm::vec3&);
	virtual void UpdateEdges(bool force = false);

	// AABB
	const AABB& GetAABB();
	virtual void UpdateAABB();
	[[nodiscard]] bool Inside(const glm::vec3&) const;
	[[nodiscard]] virtual bool InsideDraggable(const glm::vec3&);
	[[nodiscard]] float GetArea() const;

	// Interaction
	void Draw(bool is_focussed);
	virtual void Gui();
	virtual void Select(const glm::vec3&);
	virtual void Move(const glm::vec3& offset);
	void SetOnDeleteCallback(const std::function<void(IStructure&)>& callback);
	std::shared_ptr<Dataset> dataset;

	VectorOfNodes nodes;
	VectorOfEdgePaths edges;
	
	void SetNodesColor(ofColor color);
	void SetEdgeColor(const glm::vec4& color);

protected:
	virtual void DrawNodes() = 0;
	virtual void InitEdges();
	virtual void InitNodes();
	virtual void NodeInfoGui();
	void SetSelectedNode(const std::shared_ptr<Node>& _node);

	std::vector<std::shared_ptr<ILayout>> m_layouts;
	std::shared_ptr<ILayout> m_active_layout;
	AABB m_aabb;
	glm::vec3 m_position = glm::vec3(0);
	std::shared_ptr<Node> m_selected_node;

	struct GuiData
	{
		bool checkbox_node_labels = false;
		bool checkbox_draw_bounding_box = true;
		float slider_radius = 10;
		glm::vec4 coloredit_node_color = glm::vec4(0, 0, 0, 255);
		glm::vec4 coloredit_edge_color = glm::vec4(123, 123, 123, 255);
	} m_gui_data;

	std::function<void(IStructure&)> m_on_delete_callback;

private:
	static int __idx;
	int m_idx = 0;


};
} // namespace DataVis
