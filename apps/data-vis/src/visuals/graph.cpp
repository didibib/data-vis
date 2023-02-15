#include "precomp.h"

// Graph parsing
// https://stackoverflow.com/questions/29496182/read-graphviz-in-boostgraph-pass-to-constructor/29501850#29501850
#include "read_graphviz_new.cpp"

namespace DataVis {
	void Graph::Load(std::string _filename) {
		std::string filepath = ofToDataPath(_filename, false);
		std::ifstream file( filepath );
		
		if ( !std::filesystem::exists( filepath ) ) {
			std::cout << "W/Graph::Setup: File doesn't exists: " << filepath << std::endl;
			return;
		}

		graph_.clear( );
		boost::dynamic_properties dp( boost::ignore_other_properties );
		dp.property( "node_id", get( &Vertex::name, graph_ ) );
		dp.property( "label", get( &Vertex::label, graph_ ) );
		dp.property( "shape", get( &Vertex::shape, graph_ ) );
		dp.property( "label", get( &Edge::label, graph_ ) );
		dp.property( "weight", get( &Edge::weight, graph_ ) );

		boost::read_graphviz( file, graph_, dp );
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