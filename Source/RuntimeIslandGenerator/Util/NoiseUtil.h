#pragma once

class FNoiseUtil
{
public:
	static float PerlinNoise2D(const FVector2D& Pos, const int32 Seed);

	static float ValueNoise2D(const FVector2D& Pos, const int32 Seed);

	static float FbmPerlinNoise2D(const FVector2D& Pos, const int32 Seed);

	static float DomainWarpWithFbm(const FVector2D& Pos, const int32 Seed
									, float (*FbmNoise)(
										const FVector2D& InPos
										, const int32 InSeed));

private:
	static int32 SplitMix(int32 Seed);
	static int32 Hash32_2D(const FVector2D& Pos, const int32 Seed);
	static float Hash01_2D(const FVector2D& Pos, const int32 Seed);
};
