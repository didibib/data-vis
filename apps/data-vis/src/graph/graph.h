#pragma once

namespace DataVis
{
class Graph : public IStructure
{
public:
	//--------------------------------------------------------------
	Graph() = default;
	void Init(const std::shared_ptr<Dataset>) override;
	void HandleInput() override;
	void Select(const glm::vec3&) override;
	void Update(float delta_time) override;
	void DrawLayout() override;
	void Gui() override;
	std::vector<std::shared_ptr<IStructure::Node>>& GetNodes() override;

	float radius = 10;
protected:
	void PostBuild() override {};
	std::vector<std::shared_ptr<Node>> m_nodes;

private:
};
}
