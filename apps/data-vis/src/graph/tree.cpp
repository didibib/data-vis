#include "precomp.h"

namespace DataVis
{
Tree Tree::Extractor::MSP(Graph& _graph)
{
	// Implement Prim's Algorithm
	// https://www.wikiwand.com/en/Prim%27s_algorithm
	auto& vertices = _graph.Vertices();
	// Initialize lowest cost connections and corresponding edges
	std::vector<float> costs;
	std::vector<Edge*> edges;
	costs.resize(vertices.size());
	edges.resize(vertices.size());

	Tree n;
	return n;
}

void Tree::Draw() {
	ofFill();
	ofSetColor(255);
	ofSetDrawBitmapMode(OF_BITMAPMODE_SIMPLE);
	ofDrawCircle(m_root->position, radius);
	ofDrawBitmapStringHighlight(ofToString(m_root->subtree_count), m_root->position + glm::vec3(10, 10, -1));
	std::stack<std::shared_ptr<Node>> stack;
	for (auto& child : m_root->children) stack.push(child);

	while (stack.empty() == false) {
		auto node = stack.top(); stack.pop();
		auto parent = node->parent;
		static glm::vec3 sub = { 0, 0, -1 }; // To draw edge behind nodes
		ofSetColor(123);
		ofDrawLine(parent->position + sub, node->position + sub);

		ofSetColor(255);
		ofDrawCircle(node->position, radius);
		ofDrawBitmapStringHighlight(ofToString(node->subtree_count), node->position + glm::vec3(10, 10, -1));

		for (auto& child : node->children) stack.push(child);
	}
}

} // DataVis
