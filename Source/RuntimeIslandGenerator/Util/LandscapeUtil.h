#pragma once

struct FLandscapeOptions
{
	bool IsTileInfinite = false;
	uint32 VertexCount;
	uint64 XTileVertexCount;
	uint64 YTileVertexCount;
};

struct FBiomeParams
{
	uint8 NoiseCount = 1;
	uint32 VertexCount;
};

enum class EBiome : uint8;
enum class EVoronoiMetric : uint8;

struct FFbmNoiseParams;
struct FVoronoiBiomeParams;
struct FVoronoiSample;

class FLandscapeUtil
{
public:
	static float GetHeight_Mountain(const FVector2D& Pos, const int32 Seed
									, const FLandscapeOptions& Params);
	static float GetHeight_PlainHill(const FVector2D& Pos, const int32 Seed
									, const FLandscapeOptions& Params);

	static float SquareGradient(const FVector2D& Pos
								, const FLandscapeOptions& Params);

	// 좌표 -> 바이옴 (가장 단순)
	static EBiome GetBiomeAt_Voronoi(const FVector2D& Pos
									, const FVoronoiBiomeParams& Params);

private:
	static float FbmPerlinNoise(const FVector2D& Pos, const int32 Seed
								, const FFbmNoiseParams& Params);

	static FVector2D JitterInCell(const FVector2D& Pos, const uint32 Seed
								, const float JitterAmp = 0.45f);

	static float MetricDistSq(const FVector2D& A, const FVector2D& B);

	static FVoronoiSample SampleVoronoi(const FVector2D& Pos
										, const FVoronoiBiomeParams& Params);

	// 셀 해시 -> 바이옴 (가중치 분포를 사용)
	static EBiome PickBiomeFromHash(const uint32 CellHash
									, const FVoronoiBiomeParams& Params);

	// (F2^2 - F1^2) 기반 간단 경계 마스크
	static FORCEINLINE float
	EdgeMask01(const FVoronoiSample& S, float K = 4.0f);
};
