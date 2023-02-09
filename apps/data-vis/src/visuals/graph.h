#pragma once
namespace DataVis {
	class Node {
	public:
		Node() = default;
	};

	class Graph {
		std::vector<string> headers_;
		std::unordered_map<int, std::vector<int>> adjacency_;
		int stride_ = 0;

	public:
		Graph() = default;
		void Setup(std::string filename);
		void Update();
		void Draw();
		void Exit();

		int sphereSize = 3;
	};
}
