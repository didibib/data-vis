#include "precomp.h"

namespace DataVis {
	void Graph::Setup(std::string _filename) {
		std::string filepath = ofToDataPath(_filename, false);
		std::ifstream file( filepath );
		if ( file.is_open() ) {
			int x;
		}
		
		if ( !std::filesystem::exists( filepath ) ) {
			std::cout << "W/Graph::Setup: File doesn't exists: " << filepath << std::endl;
			return;
		}
		DotGraph graph = DotParser::ParseFromFile( filepath );
	}

	void Graph::Update() {

	}

	void Graph::Draw()
	{
		ofNoFill();
		ofDrawSphere(64);
		ofDrawCircle(0, 0, 72);

		if (ImGui::Begin("ImGui Window")) {
			ImGui::Text("Hello");
			ImGui::End();
		}
	}

	void Graph::Exit() {

	}
}