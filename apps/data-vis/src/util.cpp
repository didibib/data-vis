#include "precomp.h"

//--------------------------------------------------------------
namespace DataVis
{
namespace Random
{
static uint seed = 0x12345678;
uint UInt()
{
	seed ^= seed << 13;
	seed ^= seed >> 17;
	seed ^= seed << 5;
	return seed;
}

uint UInt(uint& _seed)
{
	_seed ^= _seed << 13;
	_seed ^= _seed >> 17;
	_seed ^= _seed << 5;
	return _seed;
}

uint Range(uint min, uint max)
{
	std::uniform_int_distribution<std::mt19937::result_type> dist(min, max);
	return dist(Random::MT19937);
}

uint Range(uint max)
{
	return Range(0, max);
}

float Float() { return UInt() * 2.3283064365387e-10f; }
float RangeF(float _range) { return Float() * _range; }

} // namespace Random

namespace Curves
{
float Bezier(float _t)
{
	return _t * _t * (3.0f - 2.0f * _t);
}
} // namespace Curves

namespace ImGuiExtensions
{
	bool ColorEdit3(char* _label, glm::vec3& _color, int _flags)
	{
		_color /= 255.f;
		bool val = ImGui::ColorEdit3(_label, (float*)&_color, _flags);
		_color *= 255;
		return val;
	}
} // namespace ImGuiExtensions


} // namespace DataVis
