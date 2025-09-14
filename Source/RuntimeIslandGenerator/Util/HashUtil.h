#pragma once

class FHashUtil
{
public:
	static int64 Hash32_2D(const FVector2D& Pos, const int64 Seed);
	static float Hash01_2D(const FVector2D& Pos, const int64 Seed);
	static float Hash01(const int64 X);
	static float RandDisp(const FVector2D& Pos, const int& Step
						, const float Magnitude, const int64& Seed);

	static int64 SplitMix(int64 Seed);
	static int64 WangHash(int64 X);
	static int64 HashCoord(const FVector2D& Pos, int Step, const int64& Seed);
	static float Smooth5(const float T);
};
