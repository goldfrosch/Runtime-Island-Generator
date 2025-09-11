#pragma once

struct FGeneratedDiamondSquareTileParams
{
	int VertexCount;
	int64 Seed;
	// 기본 변위 값
	float BaseDisplacement;
	float Roughness;
	float HeightScale;

	FVector2D TileIndex;
};

struct FGeneratedSquareStepParams
{
	int VertexCount;
	int Size;
	int64 Seed;
	// 변위 값
	float Displacement;
};

class FGenerateUtil
{
public:
	static void DiamondSquare(TArray<FVector>& Vertices, const uint32 Size
							, const FGeneratedDiamondSquareTileParams& Params);

private:
	static void SquareStep(TArray<FVector>& Vertices, const FVector2D& Pos
							, const FGeneratedSquareStepParams& Params);

	// 유령 보더 값 (Normal 보간을 위한 값)를 제거하고 보는 Index 값
	static int GetIndexWithoutApron(const FVector2D& Pos
									, const uint32 VertexCount);
};
