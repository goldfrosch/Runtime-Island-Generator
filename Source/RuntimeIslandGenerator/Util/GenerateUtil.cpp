#include "GenerateUtil.h"

#include "HashUtil.h"

void FGenerateUtil::DiamondSquare(TArray<FVector>& Vertices, const uint32 Size
								, const FGeneratedDiamondSquareTileParams&
								Params)
{
	const float HeightScaleMeters = (Params.VertexCount - 1) * Params.CellSize *
		Params.HeightScale;
	float BaseDispMeters = HeightScaleMeters * Params.BaseDisp;

	BaseDispMeters *= FMath::Pow(Params.Roughness
								, Size / Params.VertexCount - 1 < 0
									? 0
									: Size / Params.VertexCount - 1);

	const uint32 Half = Size / 2;
	if (Half < 1)
	{
		return;
	}

	for (uint32 y = Half; y < Params.VertexCount; y += Size)
	{
		for (uint32 x = Half; x < Params.VertexCount; x += Size)
		{
			SquareStep(Vertices, {
							Params.VertexCount, Size, Params.Seed
							, FVector2D(x, y), BaseDispMeters
						});
		}
	}

	int Col = 0;
	for (uint32 x = 0; x < Params.VertexCount; x += Half)
	{
		Col++;
		if (Col % 2 == 1)
		{
			for (uint32 y = Half; y < Params.VertexCount; y += Size)
			{
				DiamondStep(Vertices, {
								Params.VertexCount, Half, Params.Seed
								, FVector2D(x % Params.VertexCount
											, y % Params.VertexCount)
								, BaseDispMeters
							});
			}
		}
		else
		{
			for (uint32 y = 0; y < Params.VertexCount; y += Size)
			{
				DiamondStep(Vertices, {
								Params.VertexCount, Half, Params.Seed
								, FVector2D(x % Params.VertexCount
											, y % Params.VertexCount)
								, BaseDispMeters
							});
			}
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
		Avg += static_cast<int>(FHashUtil::Hash01_2D(
			FVector2D(X, Y), Params.Seed) * Size) % (Size * 2) - Size;
		Avg = Avg / Count;
	}

	const float Disp = FHashUtil::RandDisp(FVector2D(X, Y), Size, Params.Disp
											, Params.Seed);

	Vertices[GetIndexWithoutApron(FVector2D(X, Y), VertexCount)].Z = Avg + Disp;
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
		Avg += static_cast<int>(FHashUtil::Hash01_2D(
			FVector2D(X, Y), Params.Seed) * Size) % (Size * 2) - Size;
		Avg = Avg / Count;
	}

	const float Disp = FHashUtil::RandDisp(FVector2D(X, Y), Size, Params.Disp
											, Params.Seed);

	Vertices[GetIndexWithoutApron(FVector2D(X, Y), VertexCount)].Z = Avg + Disp;
}

uint32 FGenerateUtil::GetIndexWithoutApron(const FVector2D& Pos
											, const uint32 VertexCount)
{
	return (Pos.Y + 1) * (VertexCount + 2) + Pos.X + 1;
}
