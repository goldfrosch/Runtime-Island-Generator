#pragma once

class FNoiseUtil
{
public:
	static float PerlinNoise2D(const FVector2D& Pos, const int32 Seed);

	static float ValueNoise2D(const FVector2D& Pos, const int32 Seed);

	static float FbmPerlinNoise2D(const FVector2D& Pos, const int32 Seed
								, float (*Noise)(const FVector2D& InPos
												, const int32 Seed));

	static float DomainWarpFbmCellular(const FVector2D& Pos, const int32 Seed);

private:
	static int32 SplitMix(int32 Seed);
	static int32 Hash32_2D(const FVector2D& Pos, const int32 Seed);
	static float Hash01_2D(const FVector2D& Pos, const int32 Seed);

	static void DomainWarp(FVector2D& Pos, const int32 Seed);

	static float FbmCellularNoise(const FVector2D& Pos, const int32 Seed);
	static float CellularF1(const FVector2D& Pos, const int32 Seed);
};
