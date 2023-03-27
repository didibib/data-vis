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
	void Update(const float delta_time) override;
	void UpdateEdges(bool force = false) override;
	void InitEdges() override;
	void UpdateAABB() override;
	

	bool InsideDraggable(const glm::vec3& pos) override;
	void Move(const glm::vec3& offset) override;
	void Select(const glm::vec3& pos) override;

	std::shared_ptr<ClusterDataset> dataset_clusters;
    std::vector<std::shared_ptr<Graph>> sub_graphs;
	
private:
	std::shared_ptr<Graph> m_focussed_graph;
	std::shared_ptr<Graph> m_dragging_graph;

    struct GuiData
	{
		glm::vec4 coloredit_node_color = glm::vec4(0, 0, 0, 255);
		glm::vec4 coloredit_intra_edge_color = glm::vec4(123, 123, 123, 255);
		glm::vec4 coloredit_inter_edge_color = glm::vec4(123, 123, 123, 255);
	} m_gui_data;

	std::shared_ptr<Graph> FindSubGraph(const std::string id);
};
}
