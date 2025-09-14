#include "HashUtil.h"

// PRNG 의사 난수에서 유명한 SplitMix 난수 생성기 값
int64 FHashUtil::SplitMix(int64 Seed)
{
	Seed += 0x9e3779b97f4a7c15ull;
	Seed = Seed ^ (Seed >> 30) * 0xbf58476d1ce4e5b9ull;
	Seed = Seed ^ (Seed >> 27) * 0x94d049bb133111ebull;
	Seed ^= Seed >> 31;

	return Seed;
}

int64 FHashUtil::WangHash(int64 X)
{
	X = X ^ 61u ^ X >> 16;
	X *= 9u;
	X *= 0x27d4eb2du;
	X = X ^ X >> 4;
	X = X ^ X >> 15;
	return X;
}

float FHashUtil::Hash01(const int64 X)
{
	return (WangHash(X) & 0x00FFFFFF) / 16777216.0f;
}

float FHashUtil::Smooth5(const float T)
{
	return FMath::Pow(T, 3) * (T * (T * 6.f - 15.f) + 10.f);
}

int64 FHashUtil::Hash32_2D(const FVector2D& Pos, const int64 Seed)
{
	const int64 kx = static_cast<int64>(Pos.X) * 0x517cc1b727220a95ull;
	const int64 ky = static_cast<int64>(Pos.Y) * 0x9e3779b97f4a7c15ull;
	const int64 ks = Seed * 0x94d049bb133111ebull;

	return SplitMix(kx ^ ky + 0x632be59bd9b4e019ull ^ ks);
}

float FHashUtil::Hash01_2D(const FVector2D& Pos, const int64 Seed)
{
	const int32 Hash = Hash32_2D(Pos, Seed);

	return static_cast<float>((Hash >> 11) * (1.0 / 9007199254740992.0));
}

int64 FHashUtil::HashCoord(const FVector2D& Pos, int Step, const int64& Seed)
{
	uint64 kx = static_cast<uint64>(Pos.X) * 0x517cc1b727220a95ull;
	uint64 ky = static_cast<uint64>(Pos.Y) * 0x9e3779b97f4a7c15ull;
	uint64 ks = static_cast<uint64>(Step) * 0x94d049bb133111ebull;
	return SplitMix(kx ^ (ky + 0x632be59bd9b4e019ull) ^ (Seed ^ ks));
}

float FHashUtil::RandDisp(const FVector2D& Pos, const int& Step
						, const float Magnitude, const int64& Seed)
{
	int64 h = HashCoord(Pos, Step, Seed);
	float u = static_cast<float>(h >> 11 & 0xFFFFFFFF) / 4294967295.0f;
	return (u * 2.0f - 1.0f) * Magnitude;
}
