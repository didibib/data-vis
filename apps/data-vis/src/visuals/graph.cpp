#include "precomp.h"

namespace DataVis {
	uint Node::NeighborCount() {
		return data.size() / stride;
	}

	void Graph::setup(std::string _filename) {
		std::string filepath = ofToDataPath(_filename, false);
		int nol = 0; std::string line;
		std::ifstream file(filepath);
		// Reserve space for amount of entries
		while (std::getline(file, line)) ++nol;
		file.close();
		nodes_.reserve(nol);
		// Create nodes
		file = std::ifstream(filepath);
		while (std::getline(file, line)) {
			std::replace(line.begin(), line.end(), '\t', ' ');
			std::stringstream ss(line);
			std::string word;
			std::vector<string> data;
			// Seperate each line by space
			while (std::getline(ss, word, ' ')) data.push_back(word);

			// Check data
			if (data.empty()) continue;
			if (data[0] == "%") {
				for (size_t i = 1; i < data.size(); i++)
					headers_.push_back(data[i]);
				continue;
			}

			// Create node
			std::vector<int> values;
			values.reserve(data.size());
			for (size_t i = 0; i < data.size(); i++)
			{
				int val;
				try {
					val = std::stoi(data[i]);
				}
				catch (std::invalid_argument const& ex) {
					std::cout << "std::invalid_argument::what(): " << ex.what() << '\n';
					break;
				}
				values.push_back(val);
			}

			// Find node if exists
			int id = values[0];
			auto it = nodes_.find(id);
			if (it != nodes_.end()) {
				for (size_t i = 1; i < values.size(); i++)
					it->second.data.push_back(values[i]);
				continue;
			}
			// Else create node
			nodes_[id] = Node();
			nodes_[id].stride = headers_.size() - 1;
			for (size_t i = 1; i < values.size(); i++)
				nodes_[id].data.push_back(values[i]);
		}
		file.close();
	}

	void Graph::update() {

	}

	void Graph::draw()
	{
		ofNoFill();
		ofDrawSphere(64);
		ofDrawCircle(0, 0, 72);

		if (ImGui::Begin("ImGui Window")) {
			ImGui::Text("Hello");
			ImGui::End();
		}
	}

	void Graph::exit() {

	}
}