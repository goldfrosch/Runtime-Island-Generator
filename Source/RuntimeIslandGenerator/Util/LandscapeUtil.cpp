#include "LandscapeUtil.h"

#include "HashUtil.h"

float FLandscapeUtil::GetHeight_Mountain(const FVector2D& Pos
										, const uint32 VertexCount
										, const int32 Seed)
{
	return FbmPerlinNoise(Pos / VertexCount, Seed, MountainPerlinNoiseParams);
}

float FLandscapeUtil::FbmPerlinNoise(const FVector2D& Pos, const int32 Seed
									, const FFbmNoiseParams& Params)
{
	float Result = 0.f;

	// 근사치 랜덤 조절
	// TODO: 근사치를 위한 값도 추후 Params로 넣기
	float Amplitude = Params.Amplitude;
	float Frequency = Params.Frequency;

	const float Persistence = Params.Persistence + 0.1 * (
		FHashUtil::Hash01(Seed) - 0.5f);

	// 도메인 워핑 적용
	FVector2D InPos = Pos;
	if (Params.DomainWarpStrength > KINDA_SMALL_NUMBER)
	{
		const float WarpingX = FMath::PerlinNoise2D(
			Pos * (Frequency * 0.8f) + FVector2D(Seed * 0.37f, Seed * 0.73f));
		const float WarpingY = FMath::PerlinNoise2D(
			Pos * (Frequency * 0.8f) + FVector2D(Seed * 1.13f, Seed * 0.29f));
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

