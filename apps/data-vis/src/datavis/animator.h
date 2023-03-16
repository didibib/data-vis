#pragma once

namespace DataVis
{
class Animator
{
public:
	Animator() = default;
	virtual auto Update(float delta_time, float speed = 1.f) -> void;
	virtual ~Animator() = default;

protected:
	void EaseInEaseOut(float t, float speed = 1.f);
	void StartAnimation();
	virtual void OnStopAnimation() = 0;
	virtual void Interpolate(float percentage) = 0;

private:
	void StopAnimation();
	float m_time = 0;
	bool m_animate = false;
};

class Rings : public Animator
{
public:
	Rings() = default;
	void Draw();
	void Set(int amount, float start, float step);

protected:
	void OnStopAnimation() override;
	void Interpolate(float percentage) override;

private:
	struct Value
	{
		float value = 0, old_value = 0, new_value = 0;
	};
	int m_amount{};
	std::vector<Value> m_alpha;
	std::vector<Value> m_radius;
};



} // DataVis