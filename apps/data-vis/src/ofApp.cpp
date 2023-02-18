#include "precomp.h"

//--------------------------------------------------------------
void ofApp::setup(){
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

	// Load graph files
	string data_path = ofToDataPath( "", false );
	for ( const auto& entry : filesystem::directory_iterator( data_path ) )
	{
		string path = entry.path().string();
		path.replace( 0, data_path.length(), "" );
		graph_file_names.push_back( path );
	}
	current_graph_file = graph_file_names[imgui_data.combo_graph_file_index];

	m_graph.Load(current_graph_file);
	//DataVis::Optimizer::LocalSearchSimple(m_graph, 10000);
}

//--------------------------------------------------------------
void ofApp::update(){
	const ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureMouse || io.WantCaptureKeyboard)
		m_camera.disableMouseInput();
	else m_camera.enableMouseInput();
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofBackgroundGradient(palettePurple_3, palettePurple_2, OF_GRADIENT_CIRCULAR);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	m_camera.begin();

	m_graph.Draw();

	m_camera.end();

	gui();
	ImGui::Render();	
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

//--------------------------------------------------------------
void ofApp::exit() {
	m_graph.Exit();
	
	// Destroy ImGui
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void ofApp::gui()
{
	// File selecting dropdown box
	static int item_current_idx = 3;
	const char* combo_preview_value = graph_file_names[imgui_data.combo_graph_file_index].c_str();  // Pass in the preview value visible before opening the combo (it could be anything)
	if ( ImGui::BeginCombo( "Select File", combo_preview_value ) )
	{
		for ( int n = 0; n < graph_file_names.size(); n++ )
		{
			const bool is_selected = (imgui_data.combo_graph_file_index == n);
			if ( ImGui::Selectable( graph_file_names[n].c_str(), is_selected ) )
			{
				imgui_data.combo_graph_file_index = n;
				
				//printf( "Selected %s \n", graph_file_names[imgui_data.combo_graph_file_index].c_str() );
			}

			if ( is_selected )
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	if ( ImGui::Button( "Load Graph" ) )
	{
		string new_graph_file = graph_file_names[imgui_data.combo_graph_file_index];
		if(new_graph_file != current_graph_file )
		{
			current_graph_file = new_graph_file;
			m_graph.Load( current_graph_file );
		}
	}

	ImGui::InputInt( "# of iterations", &(imgui_data.input_optimize_iterations) );
	if ( ImGui::Button( "Optimize Graph" ) )
	{
		DataVis::Optimizer::LocalSearchSimple( m_graph, imgui_data.input_optimize_iterations );
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
