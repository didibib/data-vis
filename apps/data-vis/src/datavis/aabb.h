#pragma once 

namespace DataVis
{
class AABB : public Animator
{
public:
	AABB() = default;
	void Draw() override;
	void Draw(bool is_focussed);
	float GetArea() const;
	bool Inside(const glm::vec3& position)  const;
	bool InsideDraggable(const glm::vec3& position) const;
	void SetNewBounds(const glm::vec3& top_left, const glm::vec3& bottom_right);
	glm::vec3 Clamp( const glm::vec3& ) const;

protected:
	virtual void OnStopAnimation() override;
	virtual void Interpolate(float percentage) override;

	ofRectangle m_bounds;
	glm::vec3 m_old_top_left;
	glm::vec3 m_old_bottom_right;
	glm::vec3 m_new_top_left;
	glm::vec3 m_new_bottom_right;

	ofRectangle m_draggable;
	int m_draggable_size = 50;
	bool m_is_focussed = false;
};
} // namespace DataVis