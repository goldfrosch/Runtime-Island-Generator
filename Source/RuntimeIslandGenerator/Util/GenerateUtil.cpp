#include "GenerateUtil.h"

#include "HashUtil.h"

void FGenerateUtil::DiamondSquare(TArray<FVector>& Vertices, const uint32 Size
								, const FGeneratedDiamondSquareTileParams&
								Params)
{
	const uint32 VC = Params.VertexCount; // 2^k
	const int WorldStride = VC - 1;
	const float TileWorld = WorldStride * Params.CellSize;
	const float HeightU = TileWorld * Params.HeightScale;
	const float BaseAmp = HeightU * Params.BaseDisp;

	const uint32 Half = Size / 2;
	if (Half < 1)
	{
		return;
	}

	const float k = FMath::Log2(
		static_cast<float>(VC) / static_cast<float>(Size));
	const float DispAmp = BaseAmp * FMath::Pow(Params.Roughness, k);

	for (uint32 y = Half; y < VC; y += Size)
	{
		for (uint32 x = Half; x < VC; x += Size)
		{
			SquareStep(Vertices, {
							Params.VertexCount, Size, Params.Seed
							, FVector2D(x, y), DispAmp
						});
		}
	}

	for (uint32 x = 0, col = 0; x < VC; x += Half, ++col)
	{
		const uint32 yStart = (col % 2 == 1) ? Half : 0;
		for (uint32 y = yStart; y < VC; y += Size)
		{
			DiamondStep(Vertices, {
							Params.VertexCount, Half, Params.Seed
							, FVector2D(x % VC, y % VC), DispAmp
						});
		}
	}

	DiamondSquare(Vertices, Half, Params);
}

void FGenerateUtil::SquareStep(TArray<FVector>& Vertices
								, const FGeneratedSquareStepParams& Params)
{
	float Count = 0;
	float Avg = 0;

	const int X = Params.CurrentPos.X;
	const int Y = Params.CurrentPos.Y;
	const int Size = Params.Size;
	const int VertexCount = Params.VertexCount;

	if (X - Size >= 0 && Y - Size >= 0)
	{
		Avg += Vertices[GetIndexWithoutApron(FVector2D(X - Size, Y - Size)
											, VertexCount)].Z;
		Count += 1;
	}

	if (X - Size >= 0 && Y + Size < VertexCount)
	{
		Avg += Vertices[GetIndexWithoutApron(FVector2D(X - Size, Y + Size)
											, VertexCount)].Z;
		Count += 1;
	}

	if (X + Size < VertexCount && Y - Size >= 0)
	{
		Avg += Vertices[GetIndexWithoutApron(FVector2D(X + Size, Y - Size)
											, VertexCount)].Z;
		Count += 1;
	}

	if (X + Size < VertexCount && Y + Size < VertexCount)
	{
		Avg += Vertices[GetIndexWithoutApron(FVector2D(X + Size, Y + Size)
											, VertexCount)].Z;
		Count += 1;
	}

	if (Count > 0)
	{
		Avg = Avg / Count;
	}

	Vertices[GetIndexWithoutApron(FVector2D(X, Y), VertexCount)].Z = Avg +
		FHashUtil::RandDisp(FVector2D(X, Y), Size, Params.Disp, Params.Seed);
}

void FGenerateUtil::DiamondStep(TArray<FVector>& Vertices
								, const FGeneratedDiamondStepParams& Params)
{
	float Count = 0;
	float Avg = 0;

	const int X = Params.CurrentPos.X;
	const int Y = Params.CurrentPos.Y;
	const int Size = Params.Size;
	const int VertexCount = Params.VertexCount;

	if (X - Size >= 0)
	{
		Avg += Vertices[GetIndexWithoutApron(FVector2D(X - Size, Y)
											, VertexCount)].Z;
		Count += 1;
	}
	if (X + Size < VertexCount)
	{
		Avg += Vertices[GetIndexWithoutApron(FVector2D(X + Size, Y)
											, VertexCount)].Z;
		Count += 1;
	}
	if (Y - Size >= 0)
	{
		Avg += Vertices[GetIndexWithoutApron(FVector2D(X, Y - Size)
											, VertexCount)].Z;
		Count += 1;
	}
	if (Y + Size < VertexCount)
	{
		Avg += Vertices[GetIndexWithoutApron(FVector2D(X, Y + Size)
											, VertexCount)].Z;
		Count += 1;
	}

	if (Count > 0)
	{
		Avg = Avg / Count;
	}

	Vertices[GetIndexWithoutApron(FVector2D(X, Y), VertexCount)].Z = Avg +
		FHashUtil::RandDisp(FVector2D(X, Y), Size, Params.Disp, Params.Seed);
}

uint32 FGenerateUtil::GetIndexWithoutApron(const FVector2D& Pos
											, const uint32 VertexCount)
{
	return (Pos.Y + 1) * (VertexCount + 2) + Pos.X + 1;
}
