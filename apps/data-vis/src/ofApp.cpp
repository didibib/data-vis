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
	m_camera.setFarClip( 1e30 );
	m_camera.setGlobalPosition( glm::vec3( 0, 0, 2500 ) );

	// Load
	LoadDotFiles();
	// Dataset
	auto dataset = std::make_shared<DataVis::Dataset>();
	dataset->Load(m_current_graph_file);
	m_datasets.push_back(dataset);
	auto graph = std::make_shared<DataVis::Graph>();
	graph->Init(dataset);
	DataVis::Layout::Random(*graph, 800, 600);
	graph->SetOnDeleteCallback(std::bind(&ofApp::DeleteStructure, this, std::placeholders::_1));
	m_structures.push_back(std::move(graph));

	auto tree = std::make_shared<DataVis::Tree>();
	tree->Init(dataset);
	tree->SetOnDeleteCallback(std::bind(&ofApp::DeleteStructure, this, std::placeholders::_1));
	DataVis::Tree::Extract::MSP(*tree, 0);
	DataVis::Tree::Layout::Radial(*tree, 100, 150);
	tree->UpdateAABB();

	tree->SetPosition({ 100, 50, 0 });
	m_structures.push_back(std::move(tree));
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
	for (auto& layout : m_structures) {
		float t = ofGetLastFrameTime();
		layout.get()->Update(t);
	}
}

//--------------------------------------------------------------
void ofApp::draw() {
	ofBackgroundGradient(Color::paletteCold_0, Color::paletteCold_0, OF_GRADIENT_CIRCULAR);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	m_camera.begin();

	for (auto& layout : m_structures) {
		layout.get()->Draw(m_focussed_layout == layout);
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
	ImGui::Begin("Settings");
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

			auto dataset = std::make_shared<DataVis::Dataset>();
			dataset->Load(m_graph_file_names[m_imgui_data.combo_graph_file_index]);
			m_datasets.push_back(dataset);

			auto graph = std::make_shared<DataVis::Graph>();
			graph->Init(dataset);
			graph->SetOnDeleteCallback(std::bind(&ofApp::DeleteStructure, this, std::placeholders::_1));
			DataVis::Layout::Random(*graph, 800, 600);
			m_structures.push_back(std::move(graph));
		}
		ImGui::TreePop();
		ImGui::Separator();
	}

	//--------------------------------------------------------------
	// Select Layout
	//--------------------------------------------------------------
	if (ImGui::TreeNode("Layout")) {
		auto& layout_functions = DataVis::Layout::Functions();
		std::string layout_chosen = layout_functions[m_imgui_data.combo_layout_function_index].first;
		const char* layout_preview = layout_chosen.c_str();
		if (ImGui::BeginCombo("Select Layout", layout_preview))
		{
			for (int n = 0; n < layout_functions.size(); n++)
			{
				const bool is_selected = ( m_imgui_data.combo_layout_function_index == n );
				if( ImGui::Selectable( layout_functions[n].first.c_str(), is_selected ) ) {
					m_imgui_data.combo_layout_function_index = n;
					layout_chosen = layout_functions[n].first;
				}
				if( is_selected )
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		auto& layout_descriptions = DataVis::Layout::Descriptions();
		ImGui::TextWrapped( layout_descriptions[layout_chosen].c_str() );

		static char options[256] = "";
		ImGui::InputText( "Options", options, IM_ARRAYSIZE( options ) );
		if( ImGui::Button( "Apply Layout" ) )
		{
			auto function = layout_functions[m_imgui_data.combo_layout_function_index].second;
			// TODO: function(m_graph, options);
		}

		ImGui::TreePop();
		ImGui::Separator();
	}

	ImGui::End();

	if(m_focussed_layout)
		m_focussed_layout.get()->Gui();
}

void ofApp::DeleteStructure(DataVis::IStructure& _structure)
{
	auto it = m_structures.begin();
	while (it != m_structures.end())
	{
		if (it->get() == addressof(_structure))
		{
			m_structures.erase(it);
			if (m_focussed_layout.get() == addressof(_structure))
				m_focussed_layout = nullptr;
			return;
		}
		it++;
	}
}

glm::vec3 ofApp::ScreenToWorld( const glm::vec2& _position )
{
	auto cam = m_camera.getGlobalPosition();
	auto world = m_camera.screenToWorld( glm::vec3( _position.x, _position.y, 0 ) );
	// Ray from camera origin through mouse click
	auto dir = world - cam;
	// Make dir with z-length of 1
	dir *= -1 / dir.z;
	// World pos on z=0 plane
	return cam + cam.z * dir;
}

//--------------------------------------------------------------
void ofApp::keyPressed( int key )
{

}

void ofApp::keyReleased( int key )
{

}

//--------------------------------------------------------------
void ofApp::mouseMoved( int x, int y )
{

}

//--------------------------------------------------------------
void ofApp::mouseDragged( int x, int y, int button )
{
	if( button == OF_MOUSE_BUTTON_LEFT && m_dragging_layout != nullptr )
	{
		auto world = ScreenToWorld( glm::vec2( x, y ) );

		auto dif = world - m_prev_mouse_drag;
		m_dragging_layout->Move( dif );
		m_prev_mouse_drag = world;
	}
}

//--------------------------------------------------------------
void ofApp::mousePressed( int x, int y, int button )
{
	const ImGuiIO& io = ImGui::GetIO();
	if( io.WantCaptureMouse ) return;

	if( button == OF_MOUSE_BUTTON_LEFT )
	{
		auto world = ScreenToWorld(glm::vec2(x, y));

		// Check if we are beginning to drag a layout
		for(auto& layout : m_structures)
		{
			auto transformed = world - layout.get()->GetPosition();
			if (layout.get()->GetMoveAABB().inside(transformed))
			{
				m_dragging_layout = layout;
				m_prev_mouse_drag = world;
				return;
			}
		}

		// Check if click is inside focussed layout
		if (m_focussed_layout != nullptr && m_focussed_layout.get()->InsideAABB(world))
		{
			m_focussed_layout.get()->Select(world);
		}
		else
		{
			m_focussed_layout = nullptr;
			// Check all layouts
			for (auto& layout : m_structures)
			{
				if (layout.get()->InsideAABB(world))
				{
					m_focussed_layout = layout;
					break;
				}
			}
		}
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button)
{
	m_dragging_layout = nullptr;
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
