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

	m_graph = new DataVis::Graph();
	m_graph->Load("LesMiserables.dot");
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

	m_graph->Draw();

	m_camera.end();

	ImGui::Render();	
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

//--------------------------------------------------------------
void ofApp::exit() {
	m_graph->Exit();
	delete m_graph;
	
	// Destory ImGui
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
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
