#include "precomp.h"

namespace DataVis
{
//--------------------------------------------------------------
// Animator
//--------------------------------------------------------------
void Animator::EaseInEaseOut(float _t, float _speed)
{
	if (m_time > 1)
		StopAnimation();

	if (m_animate)
	{
		m_time += _t * _speed;
		float p = Curves::Bezier(m_time);
		Interpolate(p);
		if (p >= .999f) 
			StopAnimation();
	}
}

//--------------------------------------------------------------
void Animator::StartAnimation()
{
	m_animate = true;
	m_time = 0;
}

//--------------------------------------------------------------
void Animator::StopAnimation()
{
	m_animate = false;
	m_time = 0;
	OnStopAnimation();
}

//--------------------------------------------------------------
// Rings
//--------------------------------------------------------------
void Rings::Draw()
{
	// Draw radial circles
	ofNoFill();
	ofSetCircleResolution(50);
	for (int i = 0; i < m_amount; i++)
	{
		ofSetColor(ofColor::lightGrey, m_alpha[i].value);
		ofDrawCircle(glm::vec3(0, 0, -1), m_radius[i].value);
	}
}

//--------------------------------------------------------------
void Rings::Set(int _new_amount, float _start, float _step)
{
	if (_new_amount > m_amount)
	{
		m_alpha.resize(_new_amount);
		m_radius.resize(_new_amount);

		for (int i = m_amount; i < _new_amount; i++)
		{
			m_alpha[i].new_value = 255;
			m_radius[i].old_value = max(0, i - 1) * _step;
			m_radius[i].new_value = i * _step;
		}
	}
	else
	{
		for (int i = m_amount - 1; i > _new_amount; i--)
		{
			m_alpha[i].new_value = 0;
			m_radius[i].new_value = max(0, i - 1) * _step;
		}
	}
	m_amount = _new_amount;
	StartAnimation();
}

//--------------------------------------------------------------
void Rings::OnStopAnimation()
{
	for (int i = 0; i < m_amount; i++)
	{
		m_alpha[i].value = m_alpha[i].new_value;
		m_alpha[i].old_value = m_alpha[i].new_value;
		m_radius[i].value = m_radius[i].new_value;
		m_radius[i].old_value = m_radius[i].new_value;
	}
}

//--------------------------------------------------------------
void Rings::Interpolate(float _p)
{
	for (int i = 0; i < m_amount; i++)
	{
		m_alpha[i].value = (1 - _p) * m_alpha[i].old_value + _p * m_alpha[i].new_value;
		m_radius[i].value = (1 - _p) * m_radius[i].old_value + _p * m_radius[i].new_value;
	}
}
} // DataVis