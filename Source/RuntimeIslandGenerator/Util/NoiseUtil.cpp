#include "NoiseUtil.h"

// PRNG 의사 난수에서 유명한 SplitMix 난수 생성기 값
int32 FNoiseUtil::SplitMix(int32 Seed)
{
	Seed += 0x9e3779b97f4a7c15l;
	Seed = Seed ^ (Seed >> 30) * 0xbf58476d1ce4e5b9l;
	Seed = Seed ^ (Seed >> 27) * 0x94d049bb133111ebl;
	Seed ^= Seed >> 31;

	return Seed;
}

int32 FNoiseUtil::Hash32_2D(const FVector2D& Pos, const int32 Seed)
{
	const int32 kx = static_cast<int32>(Pos.X) * 0x517cc1b727220a95l;
	const int32 ky = static_cast<int32>(Pos.Y) * 0x9e3779b97f4a7c15l;
	const int32 ks = Seed * 0x94d049bb133111ebl;

	return SplitMix(kx ^ ky + 0x632be59bd9b4e019ull ^ ks);
}

float FNoiseUtil::Hash01_2D(const FVector2D& Pos, const int32 Seed)
{
	const int32 Hash = Hash32_2D(Pos, Seed);

	return static_cast<float>((Hash >> 11) * (1.0 / 9007199254740992.0));
}

float FNoiseUtil::DomainWarpWithFbm(const FVector2D& Pos, const int32 Seed
									, float (*FbmNoise)(
										const FVector2D& InPos
										, const int32 InSeed))
{
	FVector2D q = FVector2D(FbmNoise(Pos + FVector2D(0.0, 0.0), Seed)
							, FbmNoise(Pos + FVector2D(5.2, 1.3), Seed));

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
		f *= Hash01_2D(Pos, Seed) * 2.0;
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

	float v00 = Hash01_2D(
		FVector2d(static_cast<uint32>(xi), static_cast<uint32>(yi)), Seed);
	float v10 = Hash01_2D(
		FVector2d(static_cast<uint32>(xi + 1), static_cast<uint32>(yi)), Seed);
	float v01 = Hash01_2D(
		FVector2d(static_cast<uint32>(xi), static_cast<uint32>(yi + 1)), Seed);
	float v11 = Hash01_2D(FVector2d(static_cast<uint32>(xi + 1)
									, static_cast<uint32>(yi + 1)), Seed);

	float sx = tx * tx * (3.f - 2.f * tx);
	float sy = ty * ty * (3.f - 2.f * ty);

	float ix0 = FMath::Lerp(v00, v10, sx);
	float ix1 = FMath::Lerp(v01, v11, sx);
	return FMath::Lerp(ix0, ix1, sy); // [0,1]
}
