#pragma once

namespace DataVis
{
class Animator
{
public:
    Animator() = default;
    virtual void Update(float delta_time, float speed = 1.f);
    virtual void Draw() = 0;
    virtual ~Animator() = default;

protected:
    void EaseInEaseOut(float t, float speed = 1.f);
    void StartAnimation();
    virtual void OnStopAnimation() = 0;
    virtual void Interpolate(float percentage) = 0;

    struct InterpolateValue
    {
        float value = 0, old_value = 0, new_value = 0;
    };

    struct InterpolateValue3
    {
        glm::vec3 value, old_value, new_value;
    };

private:
    void StopAnimation();
    float m_time = 0;
    bool m_animate = false;
};

class Rings : public Animator
{
public:
    Rings() = default;
    void Draw() override;
    void Set(int amount, float start, float step);

protected:
    void OnStopAnimation() override;
    void Interpolate(float percentage) override;

private:
    int m_amount{};
    std::vector<InterpolateValue> m_alpha;
    std::vector<InterpolateValue> m_radius;
};
} // DataVis
