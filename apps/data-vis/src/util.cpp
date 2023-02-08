#include "precomp.h"

static uint seed = 0x12345678;
uint RandomUInt()
{
	seed ^= seed << 13;
	seed ^= seed >> 17;
	seed ^= seed << 5;
	return seed;
}

uint RandomUInt(uint& _seed)
{
	_seed ^= _seed << 13;
	_seed ^= _seed >> 17;
	_seed ^= _seed << 5;
	return _seed;
}

float RandomFloat() { return RandomUInt() * 2.3283064365387e-10f; }
float RandomRange(float _range) { return RandomFloat() * _range; }