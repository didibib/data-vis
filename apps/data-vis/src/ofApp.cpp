#include "precomp.h"
#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
	// Init ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsLight();
	ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)ofGetWindowPtr()->getWindowContext(), true);
	ImGui_ImplOpenGL3_Init("#version 330");

	// Set OpenGL 
	ofEnableDepthTest();

	// Setup 2D camera https://gist.github.com/roymacdonald/cd92c6d5adfa2c8aeffb22fc6c293bcc
	m_camera.removeAllInteractions();
	m_camera.addInteraction(ofEasyCam::TRANSFORM_TRANSLATE_XY, OF_MOUSE_BUTTON_MIDDLE);
	m_camera.addInteraction(ofEasyCam::TRANSFORM_TRANSLATE_Z, OF_MOUSE_BUTTON_RIGHT);
	m_camera.setScrollSensitivity(20);
	m_camera.setVFlip(true);
	m_camera.setFarClip(1e30);
	m_camera.setGlobalPosition(glm::vec3(0, 0, 2500));

	// Load
	LoadDotFiles();
	// Dataset
	auto dataset = std::make_shared<DataVis::Dataset>();
	dataset->Load(m_current_graph_file);
	m_datasets.push_back(dataset);

	auto graph = std::make_unique<DataVis::Graph>();
	graph->Init(dataset);
	DataVis::Layout::Random(*graph, 800, 600);
	m_layouts.push_back(std::move(graph));

	auto tree = std::make_unique<DataVis::Tree>();
	tree->Init(dataset);
	DataVis::Tree::Extract::MSP(*tree, 0);
	DataVis::Tree::Layout::Radial(*tree, 100, 150);
	tree->UpdateAABB();

	tree->SetPosition({ 100, 50, 0 });
	m_layouts.push_back(std::move(tree));
}

//--------------------------------------------------------------
void ofApp::LoadDotFiles()
{
	string data_path = ofToDataPath("", false);
	for (const auto& entry : filesystem::directory_iterator(data_path))
	{
		if (entry.path().extension() == ".dot")
			m_graph_file_names.push_back(entry.path().filename().string());
	}
	m_current_graph_file = m_graph_file_names[m_imgui_data.combo_graph_file_index];
}

//--------------------------------------------------------------
void ofApp::update()
{
	const ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureMouse || io.WantCaptureKeyboard)
		m_camera.disableMouseInput();
	else m_camera.enableMouseInput();
	for (auto& layout : m_layouts) {
		float t = ofGetLastFrameTime();
		layout.get()->Update(t);
	}
}

//--------------------------------------------------------------
void ofApp::draw()
{
	ofBackgroundGradient(Color::paletteCold_0, Color::paletteCold_0, OF_GRADIENT_CIRCULAR);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	m_camera.begin();

	for (auto& layout : m_layouts) {
		layout.get()->Draw();
	}

	m_camera.end();

	Gui();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

//--------------------------------------------------------------
void ofApp::exit()
{
	// Destroy ImGui
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void ofApp::Gui()
{
	if (ImGui::BeginMainMenuBar()) {
		//--------------------------------------------------------------
		// Loading Dataset 
		//--------------------------------------------------------------
		if (ImGui::BeginMenu("Load")) {
			const char* select_file_preview = m_graph_file_names[m_imgui_data.combo_graph_file_index].c_str();  // Pass in the preview value visible before opening the combo (it could be anything)
			if (ImGui::BeginCombo("Select Dataset", select_file_preview))
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

			if (ImGui::Button("Load Dataset"))
			{
				std::string new_dataset_file = m_graph_file_names[m_imgui_data.combo_graph_file_index];
				std::unique_ptr<DataVis::Dataset> dataset = std::make_unique<DataVis::Dataset>();
				dataset->Load(new_dataset_file);
				m_datasets.push_back(std::move(dataset));
			}

			ImGui::EndMenu();
		}

		//--------------------------------------------------------------
		// Create IStructure 
		//--------------------------------------------------------------
		if (ImGui::BeginMenu("Create")) {
			const char* select_dataset_preview = m_datasets[m_imgui_data.combo_dataset_index]->GetFilename().c_str();
			if (ImGui::BeginCombo("Select Dataset", select_dataset_preview))
			{
				for (int n = 0; n < m_datasets.size(); n++)
				{
					const bool is_selected = (m_imgui_data.combo_dataset_index == n);
					if (ImGui::Selectable(m_datasets[n]->GetFilename().c_str(), is_selected))
						m_imgui_data.combo_dataset_index = n;
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			const char* select_structure_preview = "";
			if (ImGui::BeginCombo("Select Structure", select_dataset_preview))
			{
				for (int n = 0; n < m_datasets.size(); n++)
				{
					const bool is_selected = (m_imgui_data.combo_dataset_index == n);
					if (ImGui::Selectable(m_graph_file_names[n].c_str(), is_selected))
						m_imgui_data.combo_graph_file_index = n;
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			if (ImGui::Button("Create Structure"))
			{
				std::string new_dataset_file = m_graph_file_names[m_imgui_data.combo_graph_file_index];
				std::unique_ptr<DataVis::Dataset> dataset = std::make_unique<DataVis::Dataset>();
				dataset->Load(new_dataset_file);
				m_datasets.push_back(std::move(dataset));
			}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	for (auto& layout : m_layouts) {
		layout.get()->Gui();
	}
}

glm::vec3 ofApp::ScreenToWorld(const glm::vec2& _position)
{
	auto cam = m_camera.getGlobalPosition();
	auto world = m_camera.screenToWorld(glm::vec3(_position.x, _position.y, 0));
	// Ray from camera origin through mouse click
	auto dir = world - cam;
	// Make dir with z-length of 1
	dir *= -1 / dir.z;
	// World pos on z=0 plane
	return cam + cam.z * dir;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{

}

void ofApp::keyReleased(int key)
{

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y)
{

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button)
{
	if (button == OF_MOUSE_BUTTON_LEFT && m_dragging_layout_idx != -1)
	{
		auto layout = m_layouts[m_dragging_layout_idx].get();
		auto world = ScreenToWorld(glm::vec2(x, y));

		auto dif = world - m_prev_mouse_drag;
		layout->Move(dif);
		m_prev_mouse_drag = world;
	}
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button)
{
	const ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureMouse) return;

	if (button == OF_MOUSE_BUTTON_LEFT)
	{
		for (int i = 0; i < m_layouts.size(); i++)
		{
			auto& layout = m_layouts[i];
			auto world = ScreenToWorld(glm::vec2(x, y));
			auto transformed = world - layout.get()->GetPosition();
			if (layout.get()->GetMoveAABB().inside(transformed))
			{
				m_dragging_layout_idx = i;
				m_prev_mouse_drag = world;
				return;
			}
		}

		for (auto& layout : m_layouts)
		{
			// Transform it to local coordinate system
			auto transformed = ScreenToWorld(glm::vec2(x, y)) - layout.get()->GetPosition();
			if (layout.get()->GetAABB().inside(transformed))
				layout.get()->Select(transformed);
		}
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button)
{
	m_dragging_layout_idx = -1;
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y)
{

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y)
{

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h)
{

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg)
{

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo)
{

}
