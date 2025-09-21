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

	const float SeedOffsetX = static_cast<float>(Seed) * 0.0001f;
	const float SeedOffsetY = static_cast<float>(Seed) * 0.00013f;

	return FMath::PerlinNoise2D(FVector2D(
		Pos.X * NoiseScale + NoiseOffset + SeedOffsetX
		, Pos.Y * NoiseScale + NoiseOffset + SeedOffsetY));
}

float FNoiseUtil::FbmPerlinNoise2D(const FVector2D& Pos, const int32 Seed)
{
	constexpr int Octaves = 4;
	constexpr float Lacunarity = 2.0f;
	constexpr float Persistence = 0.5f;

	float f = 1.0f;
	float a = 1.0f;
	float t = 0.0f;
	float norm = 0.0f;

	for (int i = 0; i < Octaves; ++i)
	{
		t += a * PerlinNoise2D(Pos * f, Seed);
		norm += a;
		f *= Lacunarity;
		a *= Persistence;
	}

	return norm > 0.f ? t / norm : 0.f;
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
	float Amp = 0.5f;
	float Frequency = Params.BaseFreq;
	float Sum = 0.f;
	float Normal = 0.f;

	// smaller epsilon to approximate abs but smoother
	constexpr float eps = 1e-5f;

	// Use a safer Sharp value range; if caller passes >2, clamp to 2.0 to avoid extreme spikes
	const float safeSharp = FMath::Clamp(Sharp, 1.0f, 2.0f);

	for (int i = 0; i < Params.Octaves; i++)
	{
		float n = ValueNoise2D(Pos * Frequency, Seed); // -1..1
		// smooth abs
		float smoothAbs = FMath::Sqrt(n * n + eps);
		float r = 1.f - smoothAbs; // ridge base in ~[0..1]
		// apply moderated sharpening
		r = FMath::Pow(r, safeSharp);
		Sum += r * Amp;
		Normal += Amp;
		Amp *= Params.Gain;
		Frequency *= Params.Lacunarity;
	}

	float out = (Normal > 0.f) ? (Sum / Normal) : 0.f;
	// remap to -1..1 if you need (original code did *2 -1), but prefer 0..1 for blending:
	return FMath::Clamp(out, 0.f, 1.f);
}

FVector2D FNoiseUtil::DomainWarp(const FVector2D& Pos, const int32 Seed
								, const FFractalParams& Base
								, const FWarpParams& W)
{
	// produce low-frequency warp (stable)
	float wx = FbmValueNoise2D(Pos, Seed, FFractalParams{
									Base.Octaves, Base.Lacunarity, Base.Gain
									, W.Freq
								});
	float wy = FbmValueNoise2D(Pos + FVector2D(1000.f, -2000.f), Seed
								, FFractalParams{
									Base.Octaves, Base.Lacunarity, Base.Gain
									, W.Freq * 1.87f
								});

	// reduce detail contribution: lower multiplier and freq
	float hx = FbmValueNoise2D(Pos * 2.0f, Seed, Base); // was 3.1
	float hy = FbmValueNoise2D(Pos * 1.8f, Seed, Base); // was 2.7

	// blend detail weaker (W.Detail should be small)
	wx = wx + W.Detail * 0.5f * hx;
	wy = wy + W.Detail * 0.5f * hy;

	// apply amplitude (reduce if previously large)
	return Pos + FVector2D(wx, wy) * (W.Amp * 0.6f);
}

float FNoiseUtil::Height_Mountains(const FVector2D& Pos, const int32 Seed
									, const FFractalParams& Base
									, const FWarpParams& W)
{
	const FVector2D DomainWarpPos = DomainWarp(Pos, Seed, Base, W);

	// Lower Sharp from 2.3 -> 1.4 (tuneable)
	const float Ridged = RidgedMF(Pos, Seed, Base, 1.4f);

	// Detail: reduce amplitude slightly for stability
	const float Detail = FbmValueNoise2D(DomainWarpPos * 2.0f, Seed, Base);

	// create a smooth "mountain mask" from the low-frequency ridged shape
	// mask will be around 0 for plains, 1 for strong mountains
	float lowFreq = FbmValueNoise2D(DomainWarpPos * 0.25f, Seed, FFractalParams{
										Base.Octaves, Base.Lacunarity, Base.Gain
										, Base.BaseFreq
									});
	// remap lowFreq from [-1,1] -> [0,1]
	float lowRemap = FMath::Clamp(lowFreq * 0.5f + 0.5f, 0.f, 1.f);
	// smoothstep to produce clearer separation
	float mask = FMath::SmoothStep(0.35f, 0.65f, lowRemap);
	// tune thresholds to preference

	// combine: where mask ~1 use ridged+detail, where mask ~0 use gentle plains (low amplitude detail)
	float mountain = Ridged * 1.0f + Detail * 0.35f;
	float plains = Detail * 0.15f; // gentle noise-only plains
	float h = FMath::Lerp(plains, mountain, mask);

	// small final curve to soften peaks slightly
	h = FMath::Pow(h, 1.05f);

	return FMath::Clamp(h, 0.f, 1.f);
}

void FNoiseUtil::BlurNxN(TArray<FVector>& Positions, const uint32 VertexCount
						, const uint8 N)
{
	// 혹시 모를 예외 처리
	if (Positions.Num() != VertexCount * VertexCount)
	{
		UE_LOG(LogTemp, Error, TEXT("경고: Vertex의 숫자와 위치 배열 정보가 일치하지 않습니다."));
		return;
	}

	TArray<FVector> Src = Positions;
	const int BlurSize = N / 2;
	const int MinBlurSize = -1 * BlurSize;
	const int MaxBlurSize = BlurSize;

	for (uint32 y = 0; y < VertexCount; y++)
	{
		for (uint32 x = 0; x < VertexCount; x++)
		{
			float acc = 0.0;
			float sumw = 0.0;

			for (int32 ky = MinBlurSize; ky <= MaxBlurSize; ++ky)
			{
				int32 CalcY = y + ky;
				if (CalcY < 0)
				{
					CalcY = 0;
				}

				if (CalcY >= static_cast<int32>(VertexCount))
				{
					CalcY = VertexCount - 1;
				}

				for (int32 kx = -1; kx <= 1; ++kx)
				{
					int32 CalcX = x + kx;
					if (CalcX < 0)
					{
						CalcX = 0;
					}
					if (CalcX >= static_cast<int32>(VertexCount))
					{
						CalcX = VertexCount - 1;
					}

					float w = 1.0;

					if (kx > MinBlurSize && kx < MaxBlurSize)
					{
						w *= 2;
					}

					if (ky > MinBlurSize && ky < MaxBlurSize)
					{
						w *= 2;
					}

					acc += Src[CalcY * VertexCount + CalcX].Z * w;
					sumw += w;
				}
			}

			Positions[y * VertexCount + x].Z = acc / sumw;
		}
	}
}
