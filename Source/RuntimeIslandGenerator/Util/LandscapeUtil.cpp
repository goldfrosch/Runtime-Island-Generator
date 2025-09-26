#include "LandscapeUtil.h"

#include "HashUtil.h"

float FLandscapeUtil::GetHeight_Mountain(const FVector2D& Pos, const int32 Seed
										, const FLandscapeOptions& Params)
{
	return FbmPerlinNoise(Pos / Params.VertexCount, Seed
						, MountainPerlinNoiseParams);
}

float FLandscapeUtil::GetHeight_PlainHill(const FVector2D& Pos, const int32 Seed
										, const FLandscapeOptions& Params)
{
	return FbmPerlinNoise(Pos / Params.VertexCount, Seed
						, PlainHillPerlinNoiseParams);
}

float FLandscapeUtil::FbmPerlinNoise(const FVector2D& Pos, const int32 Seed
									, const FFbmNoiseParams& Params)
{
	float Result = 0.f;

	// 근사치 랜덤 조절
	// TODO: 근사치를 위한 값도 추후 Params로 넣기
	float Amplitude = Params.Amplitude;
	float Frequency = Params.Frequency;

	const float Persistence = Params.Persistence + 0.1f * (
		FHashUtil::Hash01(Seed) - 0.5f);

	// 도메인 워핑 적용
	FVector2D InPos = Pos;
	if (Params.DomainWarpStrength > KINDA_SMALL_NUMBER)
	{
		const float sfx = FHashUtil::Hash01(Seed * 1664525 + 1013904223);
		const float sfy = FHashUtil::Hash01(Seed * 22695477 + 1);

		const FVector2D SeedOffset = FVector2D(sfx - 0.5f, sfy - 0.5f);

		const float WarpingX = FMath::PerlinNoise2D(
			Pos * (Frequency * 0.8f) + SeedOffset);
		const float WarpingY = FMath::PerlinNoise2D(
			Pos * (Frequency * 0.8f) + SeedOffset);

		InPos += FVector2D(WarpingX, WarpingY) * Params.DomainWarpStrength;
	}

	for (int i = 0; i < Params.OctaveCount; i++)
	{
		Result += Amplitude * FMath::PerlinNoise2D(InPos * Frequency);
		Amplitude *= Persistence;
		Frequency *= Params.Lacunarity;
	}

	return Result;
}

float FLandscapeUtil::SquareGradient(const FVector2D& Pos
									, const FLandscapeOptions& Params)
{
	const FVector2D Center(Params.XTileVertexCount * 0.5f
							, Params.YTileVertexCount * 0.5f);

	const float X = FMath::Abs(Pos.X - Center.X);
	const float Y = FMath::Abs(Pos.Y - Center.Y);

	const float Dist = FMath::Max(X, Y);

	const float MaxDist = FMath::Max(Params.XTileVertexCount
									, Params.YTileVertexCount) * 0.5f;
	const float DistRatio = Dist / MaxDist;

	float Mask = 1.0f - DistRatio;
	Mask = FMath::Clamp(Mask, 0.0f, 1.0f);

	return Mask;
}
