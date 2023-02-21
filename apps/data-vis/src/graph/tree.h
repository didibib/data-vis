#pragma once

namespace DataVis
{
class Tree
{

public:
	//--------------------------------------------------------------
	struct Node
	{
	public:
		int vertex;
		int subtree_count;
		std::shared_ptr <Node> parent;
		std::vector<std::shared_ptr<Node>> children;

		glm::vec3 position;
	};

	class Extractor
	{
	public:
		static Tree MSP(Graph&);
	};

	//--------------------------------------------------------------
	Tree() {
		m_root = std::make_shared<Node>();
		m_root->subtree_count = 10;
		for (size_t i = 0; i < 3; i++)
			m_root->children.push_back(std::make_shared<Node>());
		for (auto& child : m_root->children) {
			child->parent = m_root;
			child->subtree_count = 1;
		}

		auto& child1 = m_root->children[0];
		child1->subtree_count = 4;
		for (size_t i = 0; i < 3; i++)
			child1->children.push_back(std::make_shared<Node>());
		for (auto& child : child1->children) {
			child->parent = child1;
			child->subtree_count = 1;
		}

		auto& child2 = m_root->children[1];
		child2->subtree_count = 4;
		for (size_t i = 0; i < 3; i++)
			child2->children.push_back(std::make_shared<Node>());
		for (auto& child : child2->children) {
			child->parent = child2;
			child->subtree_count = 1;
		}

	}

	void Draw();
	Node& Root() { return *m_root; }

	float radius = 5;

private:
	std::shared_ptr<Node> m_root;
};

} // DataVis