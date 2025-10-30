#include "LandscapeUtil.h"
#include "NoiseData.h"
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

FVector2D FLandscapeUtil::JitterInCell(const FVector2D& Pos, const uint32 Seed
										, const float JitterAmp)
{
	const uint32 NewHash = FHashUtil::Hash32_2D(Pos, Seed);

	// 두 개의 다른 난수 축출
	// 난수 추출의 값 범위를 줄이기 위해 비트 연산자로 앞의 값을 자르고
	// 최대 비트 값으로 나누어 값 자체를 한정 시킨다.
	const float Rx = (NewHash & 0x0000FFFF) / 65536.0f;
	const float Ry = (NewHash >> 16 & 0x0000FFFF) / 65536.0f;

	return FVector2D(Rx * JitterAmp, Ry * JitterAmp);
}

float FLandscapeUtil::MetricDistSq(const FVector2D& A, const FVector2D& B)
{
	const FVector2D d = A - B;

	return FMath::Square(d.X) + FMath::Square(d.Y);
}

FVoronoiSample FLandscapeUtil::SampleVoronoi(const FVector2D& Pos
											, const FVoronoiBiomeParams& Params)
{
	FVoronoiSample Result;

	// 임의로 3 * 3으로 범위 할당
	for (int y = -1; y <= 1; y++)
	{
		for (int x = -1; x <= 1; x++)
		{
			const float NewX = Pos.X + x;
			const float NewY = Pos.Y + y;

			// 랜덤 난수를 활용해 가져온 결과 값 0 ~ 1을 가져옴
			const FVector2D JitterResult = JitterInCell(
				FVector2D(NewX, NewY), Params.Seed, Params.JitterAmp);

			// 난수를 통해 나온 값을 기반으로 셀 중심 좌표 계산
			const FVector2D CellCenter = FVector2D(NewX + JitterResult.X
													, NewY + JitterResult.Y);

			// 두 좌표간의 거리의 제곱한 값을 구함, 즉 가장 가까운 셀 정보를 구하기 위함으로
			// Pos는 현재 좌표, Feature는 셀 중심 정보를 의미함.
			const float Distance = MetricDistSq(Pos, CellCenter);

			// 현재 거리 값이 가장 가까운 기존 거리보다 가까운 경우
			// 새로운 중심점 변경
			if (Distance < Result.F1Sq)
			{
				// 기존의 최소 거리는 차선으로 이전
				Result.F2Sq = Result.F1Sq;
				Result.F1Sq = Distance;
				Result.NearestCell = FVector2D(NewX, NewY);
			}
			else if (Distance < Result.F2Sq)
			{
				Result.F2Sq = Distance;
			}
		}
	}

	return Result;
}

float FLandscapeUtil::EdgeMask01(const FVoronoiSample& S, float K)
{
	const float diff = FMath::Max(0.0f, S.F2Sq - S.F1Sq);
	return 1.0f - FMath::Exp(-K * diff);
}

EBiome FLandscapeUtil::PickBiomeFromHash(const uint32 CellHash
										, const FVoronoiBiomeParams& Params)
{
	// 분포가 없으면 기본 분포(예: 평지40 / 숲30 / 사막20 / 툰드라10)
	const TArray<FVoronoiBiomeRule> DefaultDist = {
		{EBiome::Plains, 40}, {EBiome::Forest, 30}, {EBiome::Desert, 20}
		, {EBiome::Tundra, 10}
	};

	int32 total = 0;
	for (const auto& r : DefaultDist)
	{
		total += FMath::Max(0, r.Weight);
	}
	if (total <= 0)
	{
		return EBiome::Plains;
	}

	const uint32 pick = CellHash % static_cast<uint32>(total);

	int32 acc = 0;
	for (const auto& r : DefaultDist)
	{
		const int32 w = FMath::Max(0, r.Weight);
		if (pick < static_cast<uint32>(acc + w))
		{
			return r.Biome;
		}
		acc += w;
	}
	return (DefaultDist)[0].Biome; // fallback
}

EBiome FLandscapeUtil::GetBiomeAt_Voronoi(const FVector2D& Pos
										, const FVoronoiBiomeParams& Params)
{
	const FVoronoiSample VoronoiBiomeResult = SampleVoronoi(Pos, Params);
	const uint32 CellHash = FHashUtil::Hash32_2D(
		VoronoiBiomeResult.NearestCell, Params.Seed);

	return PickBiomeFromHash(CellHash, Params);
}
