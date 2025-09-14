#pragma once

struct FFractalParams
{
	int32 Octaves = 5;
	float Lacunarity = 2.f;
	float Gain = 0.5f;
	float BaseFreq = 0.0008f;
	uint32 Seed = 1337u;
};

struct FWarpParams
{
	float Amp = 1200.f; // 좌표 왜곡 강도(월드 유닛)
	float Freq = 0.0008f; // 워프 필드 빈도
	uint32 Seed = 7u;
	float Detail = 0.5f; // 고주파 워프 가중
};

class FNoiseUtil
{
public:
	static float PerlinNoise2D(const FVector2D& Pos, const int32 Seed);

	static float FbmPerlinNoise2D(const FVector2D& Pos, const int32 Seed);

	static float DomainWarpWithFbm(const FVector2D& Pos, const int32 Seed
									, float (*FbmNoise)(
										const FVector2D& InPos
										, const int32 InSeed));

	static float Height_Mountains(const FVector2D& Pos
								, const FFractalParams& Base
								, const FWarpParams& W
								, FVector2D RidgeDir = FVector2D(1, 0));

private:
	static float ValueNoise2D(const FVector2D& Pos, const int32 Seed);
	static float FbmValueNoise2D(const FVector2D& Pos
								, const FFractalParams& Params);
	static float RidgedMF(const FVector2D& Pos, const FFractalParams& Params
						, float Sharp = 2.0f);
	static FVector2D DomainWarp(const FVector2D& Pos, const FFractalParams& Base
								, const FWarpParams& W);
};
