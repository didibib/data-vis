#pragma once

namespace DataVis
{
//--------------------------------------------------------------
namespace po = boost::program_options;
class IStructure
{
	//--------------------------------------------------------------
public:
	class Node
	{
	public:
		Node(std::string vertex_id, VertexIdx vertex_index, glm::vec3 position = glm::vec3(0));
		void EaseInEaseOut(float t, float speed = .2f);

		const std::string& GetVertexId() const;
		VertexIdx GetVertexIdx() const;
		const glm::vec3& GetPosition();
		void SetPosition(glm::vec3& position);
		void SetNewPosition(glm::vec3& new_position);
		void SetDisplacement(glm::vec3 displacement);
		const glm::vec3& GetDisplacement();
		const float& GetRadius();
		void SetRadius(float radius);
		bool Inside(glm::vec3 position);

		ofColor color;
		std::vector<std::shared_ptr<Node>> neighbors;

	protected:
		std::string m_vertex_id;
		VertexIdx m_vertex_idx;

		glm::vec3 m_position = glm::vec3(0);
		glm::vec3 m_new_position = glm::vec3(0);
		glm::vec3 m_old_position = glm::vec3(0);
		glm::vec3 m_displacement = glm::vec3(0);
		ofRectangle m_bounding_box;
		float m_radius = 10;
		float m_time = 0;
		bool m_animate = true;
	};

	using VectorOfNodes = std::vector<std::shared_ptr<IStructure::Node>>;
	//--------------------------------------------------------------
	IStructure();
	virtual ~IStructure();
	const int& Idx() const;
	Dataset& GetDataset() const;
	virtual void Init(const std::shared_ptr<Dataset>) = 0;
	virtual void HandleInput() = 0;
	virtual void Select(const glm::vec3&) = 0;
	virtual void Update(float delta_time) = 0;
	void Draw();
	virtual void Gui() = 0;
	virtual VectorOfNodes& GetNodes() = 0;

	glm::vec3 GetPosition() const;
	void SetPosition(glm::vec3 new_position);
	void Move(glm::vec3 offset);
	const ofRectangle& GetAABB() const;
	const ofRectangle& GetMoveAABB() const;
	void UpdateAABB();

protected:
	std::shared_ptr<Dataset> m_dataset;
	glm::vec3 m_position = glm::vec3(0);
	ofRectangle m_aabb;
	ofRectangle m_move_aabb;
	int m_move_aabb_size = 50;

	virtual void DrawLayout() = 0;
	virtual void SetAABB();
	void SetMoveAABB();

private:
	static int __idx;
	int m_idx = 0;


};
} // namespace DataVis
