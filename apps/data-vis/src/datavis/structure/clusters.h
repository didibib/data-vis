#pragma once

namespace DataVis
{
class Clusters : public IStructure
{
public:
	Clusters() = default;
	void Init(const std::shared_ptr<Dataset>) override;
	void DrawNodes() override;
    void Gui() override;

	bool InsideDraggable(const glm::vec3& pos) override;
	void Move(const glm::vec3& offset) override;
	void Select(const glm::vec3& pos) override;


    VectorOfEdgePaths inter_edges;
private:
    std::vector<std::shared_ptr<Graph>> m_sub_graphs;
	std::shared_ptr<Graph> m_focussed_graph;
	std::shared_ptr<Graph> m_dragging_graph;

    struct GuiData
	{
		glm::vec3 coloredit_inter_edge_color = glm::vec3(123);
	} m_gui_data;
};
}
