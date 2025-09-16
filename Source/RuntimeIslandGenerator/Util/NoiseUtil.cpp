#include "NoiseUtil.h"

#include "HashUtil.h"
#include "Kismet/KismetMathLibrary.h"

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
	const int32 x0 = FMath::FloorToInt(Pos.X);
	const int32 y0 = FMath::FloorToInt(Pos.Y);
	const int32 x1 = x0 + 1;
	const int32 y1 = y0 + 1;

	const float fx = Pos.X - static_cast<float>(x0);
	const float fy = Pos.Y - static_cast<float>(y0);

	const float u = FHashUtil::Smooth5(fx);
	const float v = FHashUtil::Smooth5(fy);

	const uint32 s = Seed * 1013904223u;

	const float a = FHashUtil::Hash01(
		(x0 * 374761393u) ^ (y0 * 668265263u) ^ s);
	const float b = FHashUtil::Hash01(
		(x1 * 374761393u) ^ (y0 * 668265263u) ^ s);
	const float c = FHashUtil::Hash01(
		(x0 * 374761393u) ^ (y1 * 668265263u) ^ s);
	const float d = FHashUtil::Hash01(
		(x1 * 374761393u) ^ (y1 * 668265263u) ^ s);

	const float i1 = FMath::Lerp(a, b, u);
	const float i2 = FMath::Lerp(c, d, u);
	return FMath::Lerp(i1, i2, v) * 2.f - 1.f;
}


float FNoiseUtil::FbmValueNoise2D(const FVector2D& Pos, const int32 Seed
								, const FFractalParams& Params)
{
	float amp = 1.f, freq = Params.BaseFreq, sum = 0.f, norm = 0.f;
	for (int i = 0; i < Params.Octaves; i++)
	{
		sum += amp * ValueNoise2D(Pos * freq, Seed);
		norm += amp;
		amp *= Params.Gain;
		freq *= Params.Lacunarity;
	}
	return sum / FMath::Max(norm, 1e-6f);
}

float FNoiseUtil::RidgedMF(const FVector2D& Pos, const int32 Seed
							, const FFractalParams& Params, const float Sharp)
{
	float Amp = 0.5f, Frequency = Params.BaseFreq, Sum = 0.f, Normal = 0.f;

	for (int i = 0; i < Params.Octaves; i++)
	{
		float n = ValueNoise2D(Pos * Frequency, Seed);
		// 역수
		n = 1.f - FMath::Abs(n);
		n = FMath::Pow(n, Sharp);
		Sum += n * Amp;
		Normal += Amp;
		Amp *= Params.Gain;
		Frequency *= Params.Lacunarity;
	}

	return Sum / FMath::Max(Normal, 1e-6f) * 2.f - 1.f;
}

FVector2D FNoiseUtil::DomainWarp(const FVector2D& Pos, const int32 Seed
								, const FFractalParams& Base
								, const FWarpParams& W)
{
	float wx = FbmValueNoise2D(Pos, Seed, FFractalParams{
									Base.Octaves, Base.Lacunarity, Base.Gain
									, W.Freq
								});
	float wy = FbmValueNoise2D(Pos + FVector2D(1000.f, -2000.f), Seed
								, FFractalParams{
									Base.Octaves, Base.Lacunarity, Base.Gain
									, W.Freq * 1.87f
								});
	// 고주파 디테일 얹기
	float hx = FbmValueNoise2D(Pos * 3.1f, Seed, Base);
	float hy = FbmValueNoise2D(Pos * 2.7f, Seed, Base);

	wx = wx + W.Detail * hx;
	wy = wy + W.Detail * hy;

	return Pos + FVector2D(wx, wy) * W.Amp;
}

float FNoiseUtil::Height_Mountains(const FVector2D& Pos, const int32 Seed
									, const FFractalParams& Base
									, const FWarpParams& W, FVector2D RidgeDir)
{
	RidgeDir.Normalize();

	const FVector2D DomainWarpPos = DomainWarp(Pos, Seed, Base, W);

	// 이방성을 위한 좌표 꼬기 작업 추가
	FVector2D AnisotropicPos = FVector2D(
		FVector2D::DotProduct(DomainWarpPos, RidgeDir)
		, UKismetMathLibrary::CrossProduct2D(DomainWarpPos, RidgeDir));

	// 임의로 좌표를 또 늘리기
	AnisotropicPos = FVector2D(AnisotropicPos.X * 1.6f
								, AnisotropicPos.Y * 0.8f);

	const float Ridged = RidgedMF(AnisotropicPos, Seed, Base, 2.3f);
	const float Detail = FbmValueNoise2D(DomainWarpPos * 2.3f, Seed, Base);

	return FMath::Lerp(Ridged, Ridged * 0.6f + Detail * 0.4f, 0.35f);
}
