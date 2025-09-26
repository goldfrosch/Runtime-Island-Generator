#pragma once

struct FLandscapeOptions
{
	bool IsTileInfinite = false;
	uint32 VertexCount;
	uint64 XTileVertexCount;
	uint64 YTileVertexCount;
};

struct FFbmNoiseParams
{
	// 1에서 7을 권장함
	float Frequency;
	// FBM 순회 카운트
	uint8 OctaveCount;
	// 초기 진폭 값
	float Amplitude;
	// 영속성 값, 진폭 값의 값을 조정하는 것에 사용함.
	float Persistence;
	// 프랙탈의 채움 정도 값
	float Lacunarity;
	// Ridge 강도: 0 ~ 1 사이로 값 지정하기
	float RidgeIntensity;
	// 도메인 워핑의 강도
	float DomainWarpStrength;
};

struct FBiomeParams
{
	uint8 NoiseCount = 1;
	uint32 VertexCount;
};

constexpr FFbmNoiseParams PlainHillPerlinNoiseParams{
	2.5f, 4, 0.8f, 0.55f, 2.0f, 0.0f, 0.0f
};

constexpr FFbmNoiseParams MountainPerlinNoiseParams{
	1.2f, 5, 1.0f, 0.5f, 2.0f, 0.25f, 0.12f
};

class FLandscapeUtil
{
public:
	static float GetHeight_Mountain(const FVector2D& Pos, const int32 Seed
									, const FLandscapeOptions& Params);
	static float GetHeight_PlainHill(const FVector2D& Pos, const int32 Seed
									, const FLandscapeOptions& Params);

	static float SquareGradient(const FVector2D& Pos
								, const FLandscapeOptions& Params);

private:
	static float FbmPerlinNoise(const FVector2D& Pos, const int32 Seed
								, const FFbmNoiseParams& Params);
};
