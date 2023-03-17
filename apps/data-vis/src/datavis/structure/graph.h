#pragma once

namespace DataVis
{
class Graph : public IStructure
{
public:

	//--------------------------------------------------------------
	Graph() = default;
	void Init(const std::shared_ptr<Dataset>) override;
	void Load(const std::shared_ptr<Dataset>);
	void DrawNodes() override;

private:
};
}
