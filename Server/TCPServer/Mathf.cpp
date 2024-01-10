#include "stdafx.h"
#include <limits>
#include "Mathf.h"

const float Mathf::epsilon{ std::numeric_limits<float>::epsilon() };

const float Mathf::pi = 3.141592653589793238463f;

float Mathf::Deg2Rad(float deg)
{
	return deg * pi / 180.0f;
}

float Mathf::Rad2Deg(float rad)
{
	return rad * 180.0f / pi;
}

bool Mathf::Approximately(float a, float b)
{
	return abs(a-b) < epsilon;
}

float Mathf::Max(float a, float b)
{
	return (a < b) ? b : a;
}

float Mathf::Min(float a, float b)
{
	return (a < b) ? a : b;
}

float Mathf::Clamp(float value, float min, float max)
{
	if (value < min)
		return min;
	else if (value > max)
		return max;
	else
		return value;
}

float Mathf::Clamp01(float value)
{
	return Clamp(value, 0.0f, 1.0f);
}

float Mathf::InverseLerp(float a, float b, float value)
{
	return (value - a) / (b - a);
}

float Mathf::Lerp(float a, float b, float t)
{
	return LerpUnclamped(a, b, Clamp01(t));
}

float Mathf::LerpUnclamped(float a, float b, float t)
{
	return a + (b - a) * t;
}


float Mathf::MoveTowards(float current, float target, float max_delta)
{
	if (target - current < max_delta)
		return target;

	return current + max_delta;
}
