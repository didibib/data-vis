#pragma once



class ofApp : public ofBaseApp {

private:
	struct ImGuiData
	{
		int combo_graph_file_index = 3;
		int combo_dataset_index = 0;
		int combo_structure_index = 0;
		int combo_layout_function_index = 0;
	} m_imgui_data;
	std::vector<std::unique_ptr<DataVis::IStructure>> m_structures;
	std::vector<std::shared_ptr<DataVis::Dataset>> m_datasets;

	ofEasyCam m_camera;

	std::vector<string> m_graph_file_names;
	std::string m_current_graph_file = "";

	glm::vec2 m_prev_mouse_drag = glm::vec2(1e30);
	
	std::shared_ptr<DataVis::IStructure> m_focussed_layout = nullptr;
	std::shared_ptr<DataVis::IStructure> m_dragging_layout = nullptr;

	void LoadDotFiles();
	glm::vec3 ScreenToWorld(const glm::vec2& pos);

public:
	// Our methods
	void Gui();

	// OF methods
	void setup();
	void update();
	void draw();
	void exit();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

	void DeleteStructure(DataVis::IStructure& structure);
};
