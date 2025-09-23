#include "LandscapeUtil.h"

#include "HashUtil.h"

float FLandscapeUtil::GetHeight_Mountain(const FVector2D& Pos
										, const uint32 VertexCount
										, const int32 Seed)
{
	return FbmPerlinNoise(Pos / VertexCount, Seed, MountainPerlinNoiseParams);
}

uint8 FLandscapeUtil::GetBiomeData(const FVector2D& Pos, const int32 Seed
									, const FBiomeParams& Params)
{
	if (Params.NoiseCount == 1)
	{
		return 1;
	}

	return VoronoiNoise(Pos, Seed, Params);
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

uint8 FLandscapeUtil::VoronoiNoise(const FVector2D& Pos, const int32 Seed
									, const FBiomeParams& Params)
{
	const int CellX = floor(Pos.X / Params.VertexCount);
	const int CellY = floor(Pos.Y / Params.VertexCount);

	float MinDist = 999999.0f;

	// 주변 9개 셀만 확인 (자신+이웃)
	for (int dy = -1; dy <= 1; ++dy)
	{
		for (int dx = -1; dx <= 1; ++dx)
		{
			int nx = CellX + dx;
			int ny = CellY + dy;

			// Seed 위치 (격자 중심 + 랜덤 오프셋)
			FVector2D SeedPos = FVector2D(
				nx + FHashUtil::Hash01_2D(FVector2D(nx, ny), Seed)
				, ny + FHashUtil::Hash01_2D(FVector2D(nx + 1, ny + 1), Seed));

			const float Distance = FVector2D::Distance(
				Pos, SeedPos * Params.VertexCount);
			if (Distance < MinDist)
			{
				MinDist = Distance;
			}
		}
	}

	// 0 ~ 1 정규화로 소숫점 자리 잡기
	return FMath::RoundToInt(MinDist / Params.VertexCount * Params.NoiseCount);
}
