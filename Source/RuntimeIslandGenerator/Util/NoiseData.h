#pragma once

#include "CoreMinimal.h"
#include "NoiseData.generated.h"

UENUM(BlueprintType)
enum class EBiome : uint8
{
	Plains, Forest, Desert, Tundra
};

struct FVoronoiBiomeRule
{
	EBiome Biome = EBiome::Plains;
	int32 Weight = 1;
};

// 바이옴 파라미터 (필요 최소치만)
struct FVoronoiBiomeParams
{
	int32 CellSize = 1024; // 셀 하나의 월드 길이(정수 단위)
	float JitterAmp = 0.45f; // 셀 내부 특성점 흔들기(0~0.5 권장)
	int32 Seed = 1337;

	// 가중치 분포(비워두면 기본 분포 사용)
	TArray<FVoronoiBiomeRule> Distribution;
};

struct FVoronoiSample // 내부 계산 중간값(원하면 바깥에서 재사용)
{
	// 최선과 차선을 같이 저장해둠으로써, 바이옴 값을 섞을 때 유리하게 작용 가능
	float F1Sq = FLT_MAX; // 최근접 제곱거리
	float F2Sq = FLT_MAX; // 차근접 제곱거리

	FVector2D NearestCell = FVector2D::ZeroVector;
};

USTRUCT(BlueprintType)
struct FFbmNoiseParams
{
	GENERATED_BODY()
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


constexpr FFbmNoiseParams PlainHillPerlinNoiseParams{
	2.5f, 4, 0.8f, 0.55f, 2.0f, 0.0f, 0.0f
};

constexpr FFbmNoiseParams MountainPerlinNoiseParams{
	1.2f, 5, 1.0f, 0.5f, 2.0f, 0.25f, 0.12f
};
