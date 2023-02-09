#include "precomp.h"

namespace DataVis {
	void Graph::Setup(std::string _filename) {
		std::string filepath = ofToDataPath(_filename, false);
		int nol = 0; std::string line;
		std::ifstream file(filepath);
#if 0 // Not using a vector atm
		// Reserve space for amount of entries
		while (std::getline(file, line)) ++nol;
		file.close();
		// Create nodes
		file = std::ifstream(filepath);
#endif
		while (std::getline(file, line)) {
			// Replace tabs with spaces
			std::replace(line.begin(), line.end(), '\t', ' ');

			// Seperate each line by space
			std::stringstream ss(line);
			std::string word;
			std::vector<string> data;
			while (std::getline(ss, word, ' ')) data.push_back(word);

			// Check data
			if (data.empty()) continue;
			if (data[0] == "%") {
				stride_ = data.size() - 1;
				for (size_t i = 1; i < data.size(); i++)
					headers_.push_back(data[i]);
				continue;
			}

			// Build adjaceny
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
			auto it = adjacency_.find(id);
			if (it != adjacency_.end()) {
				std::pair<int, std::vector<int>> entry(id, {});
				adjacency_.insert(entry);
			}
			// Add values
			for (size_t i = 1; i < values.size(); i++)
				adjacency_[id].push_back(values[i]);
		}
		file.close();
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