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
};
