#pragma once

struct ImGuiData
{
	int combo_graph_file_index = 3;
	int input_optimize_iterations = 10000;
	int combo_layout_function_index = 0;
};

class ofApp : public ofBaseApp{

	ofEasyCam m_camera;
	std::vector<std::unique_ptr<DataVis::ILayout>> m_layouts;

	ImGuiData m_imgui_data;
	std::vector<string> m_graph_file_names;
	std::string m_current_graph_file = "";

	void LoadDotFiles();
	glm::vec3 screenToWorld( glm::vec2 );

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
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		
};
