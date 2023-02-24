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
	m_camera.setScrollSensitivity(20);

	// Load
	LoadGraphFiles();
	m_graph.Load(m_current_graph_file);
	DataVis::Layout::Random(m_graph, 800, 600);
	DataVis::Optimizer::LocalSearch( m_graph, 50000 );
	m_tree = DataVis::Tree::Extract::MSP(m_graph, 0);
	DataVis::Layout::Radial(m_tree, 100, 150);
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
	m_tree.Update();

	//ofResetElapsedTimeCounter();
}

//--------------------------------------------------------------
void ofApp::draw() {
	ofBackgroundGradient(Color::palettePurple_3, Color::palettePurple_2, OF_GRADIENT_CIRCULAR);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	m_camera.begin();

	//m_graph.Draw();
	m_tree.Draw();

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
	ImGui::Begin( "Settings" );
	//--------------------------------------------------------------
	// Loading Graph 
	//--------------------------------------------------------------
	if (ImGui::TreeNode("Loading Graph")) {
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

		ImGui::TreePop();
		ImGui::Separator();
	}

	//--------------------------------------------------------------
	// Select Layout
	//--------------------------------------------------------------
	if (ImGui::TreeNode("Layout")) {
		auto& layout_functions = DataVis::Layout::GraphLayoutFunctions();
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

		ImGui::TreePop();
		ImGui::Separator();
	}
	//--------------------------------------------------------------
	// Optimize Layout
	//--------------------------------------------------------------
	if (ImGui::TreeNode("Optimizer")) {
		ImGui::InputInt("# of iterations", &(m_imgui_data.input_optimize_iterations));
		if (ImGui::Button("Optimize Graph"))
		{
			DataVis::Optimizer::LocalSearch(m_graph, m_imgui_data.input_optimize_iterations);
		}

		ImGui::TreePop();
		ImGui::Separator();
	}
	ImGui::End();

	if (m_tree.selected_node != nullptr)
	{
		ImGui::Begin( "Selected Node" );

		ImGui::Text( "Vertex: %i", m_tree.selected_node->vertex );
		ImGui::Text( "Position: (%f.0, %f.0)", m_tree.selected_node->position.x, m_tree.selected_node->position.y );

		if (ImGui::Button( "Make root" ))
		{
			//m_tree = DataVis::Tree::Extract::MSP( m_graph, m_tree.selected_node->vertex );
			m_tree.SwapRoot( m_tree.selected_node );
			DataVis::Layout::Radial( m_tree, 100, 150 );
		}
		ImGui::End();
	}
}

glm::vec3 ofApp::screenToWorld( glm::vec2 pos )
{
	glm::vec3 cam = m_camera.getGlobalPosition();
	auto world = m_camera.screenToWorld( glm::vec3( pos.x, pos.y, 0 ) ) - cam;
	auto world_z0 = glm::vec3( world.x * cam.z, world.y * cam.z, 0 ) / 10.f;
	world_z0.x += cam.x;
	world_z0.y += cam.y;
	return world_z0;
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
	const ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureMouse) return;

	if (button == OF_MOUSE_BUTTON_LEFT)
	{
		glm::vec3 world_pos = screenToWorld( { x, y } );
		printf( "Clicking at %f, %f\n", world_pos.x, world_pos.y );
		auto selected = m_tree.Select( world_pos );
		if (selected != nullptr)
		{
			printf( "Selected vertex %i\n", selected->vertex );
			//m_tree = DataVis::Tree::Extract::MSP( m_graph, selected->vertex );
			//DataVis::Layout::Radial( m_tree, 100, 150 );
		}
		m_tree.selected_node = selected;

			
	}
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
