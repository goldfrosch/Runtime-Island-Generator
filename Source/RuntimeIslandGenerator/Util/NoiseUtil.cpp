#include "NoiseUtil.h"

#include "HashUtil.h"

float FNoiseUtil::DomainWarpWithFbm(const FVector2D& Pos, const int32 Seed
									, float (*FbmNoise)(
										const FVector2D& InPos
										, const int32 InSeed))
{
	FVector2D q = FVector2D(FbmNoise(Pos, Seed)
							, FbmNoise(Pos + FVector2D(5.2f, 1.3f), Seed));

	return FbmNoise(Pos + 4.0 * q, Seed);
}

float FNoiseUtil::PerlinNoise2D(const FVector2D& Pos, const int32 Seed)
{
	constexpr float NoiseScale = 0.00058f;
	constexpr float NoiseOffset = 0.907904f;
	constexpr float Amplitude = 50000.488281f;

	return FMath::PerlinNoise1D(NoiseScale * Pos.X + NoiseOffset * Amplitude) *
		FMath::PerlinNoise1D(NoiseScale * Pos.Y + NoiseOffset * Amplitude);
}

float FNoiseUtil::FbmPerlinNoise2D(const FVector2D& Pos, const int32 Seed)
{
	float G = exp2(-1);
	float f = 1.0;
	float a = 1.0;
	float t = 0.0;

	constexpr uint8 NoiseCount = 3;

	for (int i = 0; i < NoiseCount; i++)
	{
		t += a * PerlinNoise2D(f * Pos, Seed);
		f *= FHashUtil::Hash01_2D(Pos, Seed) * 2.0;
		a *= G;
	}

	return t;
}

float FNoiseUtil::ValueNoise2D(const FVector2D& Pos, const int32 Seed)
{
	int32 xi = FMath::FloorToInt(Pos.X);
	int32 yi = FMath::FloorToInt(Pos.Y);

	float tx = Pos.X - xi;
	float ty = Pos.Y - yi;

	float v00 = FHashUtil::Hash01_2D(
		FVector2d(static_cast<uint32>(xi), static_cast<uint32>(yi)), Seed);
	float v10 = FHashUtil::Hash01_2D(
		FVector2d(static_cast<uint32>(xi + 1), static_cast<uint32>(yi)), Seed);
	float v01 = FHashUtil::Hash01_2D(
		FVector2d(static_cast<uint32>(xi), static_cast<uint32>(yi + 1)), Seed);
	float v11 = FHashUtil::Hash01_2D(FVector2d(static_cast<uint32>(xi + 1)
												, static_cast<uint32>(yi + 1))
									, Seed);

	float sx = tx * tx * (3.f - 2.f * tx);
	float sy = ty * ty * (3.f - 2.f * ty);

	float ix0 = FMath::Lerp(v00, v10, sx);
	float ix1 = FMath::Lerp(v01, v11, sx);
	return FMath::Lerp(ix0, ix1, sy); // [0,1]
}
