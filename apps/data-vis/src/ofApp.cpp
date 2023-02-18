#include "precomp.h"

//--------------------------------------------------------------
void ofApp::setup() {
	// Init ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsClassic();
	ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)ofGetWindowPtr()->getWindowContext(), true);
	ImGui_ImplOpenGL3_Init("#version 330");

	// Set OpenGL 
	ofEnableDepthTest();

	// Setup 2D camera https://gist.github.com/roymacdonald/cd92c6d5adfa2c8aeffb22fc6c293bcc
	m_camera.removeAllInteractions();
	m_camera.addInteraction(ofEasyCam::TRANSFORM_TRANSLATE_XY, OF_MOUSE_BUTTON_MIDDLE);
	m_camera.addInteraction(ofEasyCam::TRANSFORM_TRANSLATE_Z, OF_MOUSE_BUTTON_RIGHT);
	m_camera.setVFlip(true);

	// Load
	LoadGraphFiles();
	m_graph.Load(m_current_graph_file);
	DataVis::Layout::Random(m_graph, "--w 800 --h 600");
}

void ofApp::LoadGraphFiles() {
	string data_path = ofToDataPath("", false);
	for (const auto& entry : filesystem::directory_iterator(data_path))
	{
		string path = entry.path().string();
		path.replace(0, data_path.length(), "");
		m_graph_file_names.push_back(path);
	}
	m_current_graph_file = m_graph_file_names[m_imgui_data.combo_graph_file_index];
}

//--------------------------------------------------------------
void ofApp::update() {
	const ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureMouse || io.WantCaptureKeyboard)
		m_camera.disableMouseInput();
	else m_camera.enableMouseInput();
}

//--------------------------------------------------------------
void ofApp::draw() {
	ofBackgroundGradient(palettePurple_3, palettePurple_2, OF_GRADIENT_CIRCULAR);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	m_camera.begin();

	m_graph.Draw();

	m_camera.end();

	Gui();
	//ImGui::ShowDemoWindow();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

//--------------------------------------------------------------
void ofApp::exit() {
	// Destroy ImGui
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void ofApp::Gui()
{
	// Loading Graph ============================================
	const char* select_file_preview = m_graph_file_names[m_imgui_data.combo_graph_file_index].c_str();  // Pass in the preview value visible before opening the combo (it could be anything)
	if (ImGui::BeginCombo("Select File", select_file_preview))
	{
		for (int n = 0; n < m_graph_file_names.size(); n++)
		{
			const bool is_selected = (m_imgui_data.combo_graph_file_index == n);
			if (ImGui::Selectable(m_graph_file_names[n].c_str(), is_selected))
				m_imgui_data.combo_graph_file_index = n;
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	if (ImGui::Button("Load Graph"))
	{
		string new_graph_file = m_graph_file_names[m_imgui_data.combo_graph_file_index];
		if (new_graph_file != m_current_graph_file)
		{
			m_current_graph_file = new_graph_file;
			m_graph.Load(m_current_graph_file);
		}
	}

	// Select Layout ============================================
	auto& layout_functions = DataVis::Layout::LayoutFunctions();
	std::string layout_chosen = layout_functions[m_imgui_data.combo_layout_function_index].first;
	const char* layout_preview = layout_chosen.c_str();
	if (ImGui::BeginCombo("Select Layout", layout_preview))
	{
		for (int n = 0; n < layout_functions.size(); n++)
		{
			const bool is_selected = (m_imgui_data.combo_layout_function_index == n);
			if (ImGui::Selectable(layout_functions[n].first.c_str(), is_selected)) {
				m_imgui_data.combo_layout_function_index = n;
				layout_chosen = layout_functions[n].first;
			}
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	auto& layout_descriptions = DataVis::Layout::LayoutDescriptions();
	ImGui::TextWrapped(layout_descriptions[layout_chosen].c_str());

		static char options[256] = "";
	ImGui::InputText("Options", options, IM_ARRAYSIZE(options));
	if (ImGui::Button("Apply Layout"))
	{
		auto function = layout_functions[m_imgui_data.combo_layout_function_index].second;
		function(m_graph, options);
	}

	// Optimizer
	ImGui::InputInt("# of iterations", &(m_imgui_data.input_optimize_iterations));
	if (ImGui::Button("Optimize Graph"))
	{
		DataVis::Optimizer::LocalSearch(m_graph, m_imgui_data.input_optimize_iterations);
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}
