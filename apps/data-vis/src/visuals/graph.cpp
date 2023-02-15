#include "precomp.h"

// Graph parsing
#include "read_graphviz_new.cpp"

namespace DataVis {
	void Graph::Setup(std::string _filename) {
		std::string filepath = ofToDataPath(_filename, false);
		std::ifstream file( filepath );
		
		if ( !std::filesystem::exists( filepath ) ) {
			std::cout << "W/Graph::Setup: File doesn't exists: " << filepath << std::endl;
			return;
		}

		graph_u graph( 0 );
		boost::dynamic_properties dp( boost::ignore_other_properties );
		dp.property( "node_id", get( &Vertex::name, graph ) );
		dp.property( "label", get( &Vertex::label, graph ) );
		dp.property( "shape", get( &Vertex::shape, graph ) );
		dp.property( "label", get( &Edge::label, graph ) );
		dp.property( "weight", get( &Edge::weight, graph ) );

		boost::read_graphviz( file, graph, dp );
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