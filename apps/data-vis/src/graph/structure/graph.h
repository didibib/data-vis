#pragma once

namespace DataVis
{
class Graph : public IStructure
{
public:
	//--------------------------------------------------------------
	Graph() = default;
	void Init(const std::shared_ptr<Dataset>) override;
	void Load(const std::shared_ptr<Dataset>);
	void DrawNodes() override;

private:
	struct ImGuiData
	{
		int input_optimize_iterations = 10000;

		// Force directed related variables
		bool button_fd_enabled = false;
		int input_fd_iter_per_frame = 10;
		float input_fd_C = .5f;
		float input_fd_t = 0.002f;
	} m_imgui_data;
};
}
