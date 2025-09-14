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


float FNoiseUtil::FbmValueNoise2D(const FVector2D& Pos
								, const FFractalParams& Params)
{
	float amp = 1.f, freq = Params.BaseFreq, sum = 0.f, norm = 0.f;
	for (int i = 0; i < Params.Octaves; i++)
	{
		sum += amp * ValueNoise2D(Pos * freq, Params.Seed + i * 17u);
		norm += amp;
		amp *= Params.Gain;
		freq *= Params.Lacunarity;
	}
	return sum / FMath::Max(norm, 1e-6f);
}

float FNoiseUtil::RidgedMF(const FVector2D& Pos, const FFractalParams& Params
							, float Sharp)
{
	float amp = 0.5f, freq = Params.BaseFreq, sum = 0.f, norm = 0.f;
	for (int i = 0; i < Params.Octaves; i++)
	{
		float n = ValueNoise2D(Pos * freq, Params.Seed + 911u + i * 131u);
		n = 1.f - FMath::Abs(n);
		n = FMath::Pow(n, Sharp);
		sum += n * amp;
		norm += amp;
		amp *= Params.Gain;
		freq *= Params.Lacunarity;
	}
	return sum / FMath::Max(norm, 1e-6f) * 2.f - 1.f;
}

FVector2D FNoiseUtil::DomainWarp(const FVector2D& Pos
								, const FFractalParams& Base
								, const FWarpParams& W)
{
	float wx = FbmValueNoise2D(Pos, FFractalParams{
									Base.Octaves, Base.Lacunarity, Base.Gain
									, W.Freq, W.Seed
								});
	float wy = FbmValueNoise2D(Pos + FVector2D(1000.f, -2000.f), FFractalParams{
									Base.Octaves, Base.Lacunarity, Base.Gain
									, W.Freq * 1.87f, W.Seed + 123u
								});
	// 고주파 디테일 얹기
	float hx = FbmValueNoise2D(Pos * 3.1f, FFractalParams{
									3, 2.2f, 0.55f, W.Freq * 2.3f, W.Seed + 777u
								});
	float hy = FbmValueNoise2D(Pos * 2.7f, FFractalParams{
									3, 2.2f, 0.55f, W.Freq * 2.7f, W.Seed + 779u
								});
	wx = wx + W.Detail * hx;
	wy = wy + W.Detail * hy;
	return Pos + FVector2D(wx, wy) * W.Amp;
}

float FNoiseUtil::Height_Mountains(const FVector2D& Pos
									, const FFractalParams& Base
									, const FWarpParams& W, FVector2D RidgeDir)
{
	RidgeDir.Normalize();
	// 저주파 워프 → 장거리 굴곡
	FVector2D pw = DomainWarp(Pos, Base, W);
	// 방향성 늘리기(능선 정렬): 한 축 주파수만 살짝 키움
	FVector2D anisoP = FVector2D(FVector2D::DotProduct(pw, RidgeDir)
								, UKismetMathLibrary::CrossProduct2D(
									pw, RidgeDir));
	anisoP = FVector2D(anisoP.X * 1.6f, anisoP.Y * 0.8f); // X(능선 방향) 주파수 ↑, Y ↓

	float ridged = RidgedMF(anisoP, Base, /*Sharp*/2.3f); // 뾰족한 능선
	float detail = FbmValueNoise2D(pw * 2.3f, FFractalParams{
										4, 2.2f, 0.5f, Base.BaseFreq * 2.3f
										, Base.Seed + 99u
									});
	float h = FMath::Lerp(ridged, ridged * 0.6f + detail * 0.4f, 0.35f);
	return h; // [-1,1]
}
