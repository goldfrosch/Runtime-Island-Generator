#pragma once

struct FGeneratedDiamondSquareTileParams
{
	uint32 VertexCount;
	int64 Seed;

	FVector2D TileIndex;
};

struct FGeneratedSquareStepParams
{
	uint32 VertexCount;
	uint32 Size;
	int64 Seed;
	FVector2D CurrentPos;
};

struct FGeneratedDiamondStepParams
{
	uint32 VertexCount;
	uint32 Size;
	int64 Seed;
	FVector2D CurrentPos;
};

class FGenerateUtil
{
public:
	static void DiamondSquare(TArray<FVector>& Vertices, const uint32 Size
							, const FGeneratedDiamondSquareTileParams& Params);

private:
	static void SquareStep(TArray<FVector>& Vertices
							, const FGeneratedSquareStepParams& Params);

	static void DiamondStep(TArray<FVector>& Vertices
							, const FGeneratedDiamondStepParams& Params);

	// 유령 보더 값 (Normal 보간을 위한 값)를 제거하고 보는 Index 값
	static uint32 GetIndexWithoutApron(const FVector2D& Pos
										, const uint32 VertexCount);
};
