#pragma once
namespace DataVis {
	class Node {
	public:
		Node() = default;
		uint NeighborCount();

		ofVec3f position;
		int stride;
		std::vector<int> data;
	};

	class Graph {
		std::vector<string> headers_;
		std::unordered_map<int, Node> nodes_;

	public:
		Graph() = default;
		void setup(std::string filename);
		void update();
		void draw();
		void exit();

		int sphereSize = 3;
	};
}
