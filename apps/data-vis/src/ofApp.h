#pragma once

struct ImGuiData
{
	int combo_graph_file_index = 3;
	int input_optimize_iterations = 10000;
};

class ofApp : public ofBaseApp{
	ImGuiData imgui_data;

	DataVis::Graph m_graph;
	ofEasyCam m_camera;

	std::vector<string> graph_file_names;
	string current_graph_file;

	public:
		void setup();
		void update();
		void draw();
		void exit();
		void gui();

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
