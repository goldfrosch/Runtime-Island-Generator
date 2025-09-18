#pragma once

struct FFractalParams
{
	int32 Octaves;
	float Lacunarity;
	float Gain;
	float BaseFreq;
};

struct FWarpParams
{
	float Amp; // 좌표 왜곡 강도(월드 유닛)
	float Freq; // 워프 필드 빈도
	float Detail; // 고주파 워프 가중
};

constexpr FFractalParams MountainBiomeFractal{7, 2.2f, 0.6f, 0.004f};
constexpr FWarpParams MountainBiomeWarp{1200.f, 0.0008f, 0.5f};

class FNoiseUtil
{
public:
	static float Height_Mountains(const FVector2D& Pos, const int32 Seed
								, const FFractalParams& Base
								, const FWarpParams& W, FVector2D RidgeDir);

private:
	static float ValueNoise2D(const FVector2D& Pos, const int32 Seed);
	static float FbmValueNoise2D(const FVector2D& Pos, const int32 Seed
								, const FFractalParams& Params);
	static float RidgedMF(const FVector2D& Pos, const int32 Seed
						, const FFractalParams& Params, const float Sharp);
	static FVector2D DomainWarp(const FVector2D& Pos, const int32 Seed
								, const FFractalParams& Base
								, const FWarpParams& W);

	static float PerlinNoise2D(const FVector2D& Pos, const int32 Seed);

	static float FbmPerlinNoise2D(const FVector2D& Pos, const int32 Seed);

	static float DomainWarpWithFbm(const FVector2D& Pos, const int32 Seed
									, float (*FbmNoise)(
										const FVector2D& InPos
										, const int32 InSeed));
};
