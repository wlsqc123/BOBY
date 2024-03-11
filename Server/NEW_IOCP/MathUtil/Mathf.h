#pragma once
class Mathf
{
public:
    static const float pi;
    static const float epsilon;

    static float Deg2Rad(float deg);
    static float Rad2Deg(float rad);

    static bool Approximately(float a, float b);

    static float Max(float a, float b);
    static float Min(float a, float b);

    static float Clamp(float value, float min, float max);
    static float Clamp01(float value);

    static float InverseLerp(float a, float b, float value);
    static float Lerp(float a, float b, float t);
    static float LerpUnclamped(float a, float b, float t);

    static float MoveTowards(float current, float target, float max_delta);

    static float RandF();
    static float RandF(float, float);
    static int RandF(int, int);
};
