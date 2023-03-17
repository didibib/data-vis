#pragma once

namespace DataVis
{
class Layout;
namespace po = boost::program_options;

//--------------------------------------------------------------
class IStructure
{
public:
	IStructure();
	virtual ~IStructure();
	[[nodiscard]] const int& Idx() const;
	virtual void Init(const std::shared_ptr<Dataset>);
	virtual void Update(float delta_time);
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
	virtual void Gui();
	void Select(const glm::vec3&);
	void Move(const glm::vec3& offset);
	void SetOnDeleteCallback(std::function<void(IStructure&)> callback);
	std::shared_ptr<Dataset> dataset;
	VectorOfNodes nodes;
	VectorOfEdgePaths edges;
	
protected:
	virtual void DrawNodes() = 0;
	virtual void NodeInfoGui();
	void SetSelectedNode(std::shared_ptr<Node> _node);

	std::vector<std::shared_ptr<Layout>> m_layouts;
	std::shared_ptr<Layout> m_active_layout;
	AABB m_aabb;
	glm::vec3 m_position = glm::vec3(0);
	std::shared_ptr<Node> m_selected_node;

	struct GuiData
	{
		bool checkbox_node_labels = false;
		float slider_radius = 10;
	} m_gui_data;

	std::function<void(IStructure&)> m_on_delete_callback;

private:
	static int __idx;
	int m_idx = 0;


};
} // namespace DataVis
