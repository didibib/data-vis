#pragma once

namespace DataVis
{
//--------------------------------------------------------------
// Animator
//--------------------------------------------------------------
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
    void StopAnimation();
    virtual void OnStopAnimation() = 0;
    virtual void Interpolate(float percentage) = 0;

    struct InterpolateValue
    {
        float value = 0, old_value = 0, new_value = 0;
    };

    struct InterpolateValue3
    {
        InterpolateValue3() = default;
        InterpolateValue3(const glm::vec3& _new_value)
        {
            value = _new_value;
            new_value = _new_value;
            old_value = _new_value;
        }
        glm::vec3 value, old_value, new_value;

        //--------------------------------------------------------------
        // Binary operators
        //--------------------------------------------------------------
        InterpolateValue3& operator+=(const InterpolateValue3& rhs)
        {
            value += rhs.value;
            return *this;
        }

        //--------------------------------------------------------------
        InterpolateValue3 operator+(const InterpolateValue3& rhs) const
        {
            auto lhs = *this;
            return lhs += rhs;
        }

        //--------------------------------------------------------------
        InterpolateValue3& operator-=(const InterpolateValue3& rhs)
        {
            value -= rhs.value;
            return *this;
        }

        //--------------------------------------------------------------
        InterpolateValue3 operator-(const InterpolateValue3& rhs) const
        {
            auto lhs = *this;
            return lhs -= rhs;
        }
    };

private:
    float m_time = 0;
    bool m_animate = false;
};

//--------------------------------------------------------------
// Rings
//--------------------------------------------------------------
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
    float m_start, m_step;
    std::vector<InterpolateValue> m_alpha;
    std::vector<InterpolateValue> m_radius;
};
} // DataVis
