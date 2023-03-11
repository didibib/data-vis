#pragma once

class ofApp;
namespace DataVis
{
class Graph : public IStructure
{
public:
	struct ImGuiData 
	{
		int input_optimize_iterations = 10000;
		
		// Force directed related variables
		bool button_fd_enabled = false;
		int input_fd_iter_per_frame = 10;
		float input_fd_C = .5f;
		float input_fd_t = 0.002f;
	};
	//--------------------------------------------------------------
	Graph() = default;
	void Init(const std::shared_ptr<Dataset>) override;
	void HandleInput() override;
	void Select(const glm::vec3&) override;
	void Update(float delta_time) override;
	void DrawLayout() override;
	void Gui() override;
	VectorOfNodes& GetNodes() override;

	float radius = 10;
	ImGuiData m_imgui_data;
protected:
	VectorOfNodes m_nodes;

private:
};
}
