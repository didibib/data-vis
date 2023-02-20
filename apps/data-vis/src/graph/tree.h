#pragma once

namespace DataVis
{
namespace Tree
{
class Node
{
	int vertex;
	//std::vector<Node&> children;
};

class Extractor
{
	static Node& MSP( DataVis::Graph& );
};
} // Tree
} // DataVis