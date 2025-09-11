#include "GenerateUtil.h"

#include "HashUtil.h"

void FGenerateUtil::DiamondSquare(TArray<FVector>& Vertices, const uint32 Size
								, const FGeneratedDiamondSquareTileParams&
								Params)
{
	// 초기 상태에서 높이 값 랜덤으로 지정
	if (Size == Params.VertexCount)
	{
		Vertices[GetIndexWithoutApron(FVector2D(0, 0), Params.VertexCount)].Z =
			FHashUtil::RandDisp(
				FVector2D(Params.VertexCount * Params.TileIndex.X
						, Params.VertexCount * Params.TileIndex.Y)
				, Params.VertexCount, Params.BaseDisplacement, Params.Seed);

		Vertices[GetIndexWithoutApron(FVector2D(0, Params.VertexCount - 1)
									, Params.VertexCount)].Z =
			FHashUtil::RandDisp(
				FVector2D(
					Params.VertexCount * Params.TileIndex.X + (Params.
						VertexCount -
						1), Params.VertexCount * Params.TileIndex.Y)
				, Params.VertexCount, Params.BaseDisplacement, Params.Seed);

		Vertices[
			GetIndexWithoutApron(FVector2D(Params.VertexCount - 1, 0)
								, Params.VertexCount)].Z = FHashUtil::RandDisp(
			FVector2D(Params.VertexCount * Params.TileIndex.X
					, Params.VertexCount * Params.TileIndex.Y + (Params.
						VertexCount - 1)), Params.VertexCount
			, Params.BaseDisplacement, Params.Seed);
		Vertices[
			GetIndexWithoutApron(FVector2D(Params.VertexCount - 1
											, Params.VertexCount - 1)
								, Params.VertexCount)].Z = FHashUtil::RandDisp(
			FVector2D(
				Params.VertexCount * Params.TileIndex.X + (Params.VertexCount -
					1), Params.VertexCount * Params.TileIndex.Y + (Params.
					VertexCount - 1)), Params.VertexCount
			, Params.BaseDisplacement, Params.Seed);
	}

	// 만약 더 이상 Diamond 계산이 필요없는 경우는 진행하지 않음
	const int Half = Size / 2;
	if (Half < 1)
	{
		return;
	}

	// 변위 진폭 크기
	float DispAmp = Params.BaseDisplacement;

	// 순차적으로 좌표 기준 중심 값을 조정하는 단계
	for (int y = Half; y < Params.VertexCount; y += Size)
	{
		for (int x = Half; x < Params.VertexCount; x += Size)
		{
			SquareStep(Vertices
						, FVector2D(x % Params.VertexCount
									, y % Params.VertexCount), {
							Params.VertexCount, Half, Params.Seed, DispAmp
						});
		}
	}

	

	// 진폭 크기 조정하기, 거칠기 정도 조정
	DispAmp *= Params.Roughness;

	// 마지막 재귀 호출
	DiamondSquare(Vertices, Half, Params);
}

void FGenerateUtil::SquareStep(TArray<FVector>& Vertices, const FVector2D& Pos
								, const FGeneratedSquareStepParams& Params)
{
	int Count = 0;
	int Avg = 0.f;

	if (Pos.X - Params.Size >= 0 && Pos.Y - Params.Size >= 0)
	{
		Avg += Vertices[GetIndexWithoutApron(
			FVector2D(Pos.X - Params.Size, Pos.Y - Params.Size)
			, Params.VertexCount)].Z;

		Count += 1;
	}
	if (Pos.X - Params.Size >= 0 && Pos.Y + Params.Size < Params.VertexCount)
	{
		Avg += Vertices[GetIndexWithoutApron(
			FVector2D(Pos.X - Params.Size, Pos.Y + Params.Size)
			, Params.VertexCount)].Z;

		Count += 1;
	}
	if (Pos.X + Params.Size < Params.VertexCount && Pos.Y - Params.Size >= 0)
	{
		Avg += Vertices[GetIndexWithoutApron(
			FVector2D(Pos.X + Params.Size, Pos.Y - Params.Size)
			, Params.VertexCount)].Z;

		Count += 1;
	}
	if (Pos.X + Params.Size < Params.VertexCount && Pos.Y + Params.Size < Params
		.VertexCount)
	{
		Avg += Vertices[GetIndexWithoutApron(
			FVector2D(Pos.X + Params.Size, Pos.Y + Params.Size)
			, Params.VertexCount)].Z;

		Count += 1;
	}

	Avg /= Count;

	Vertices[GetIndexWithoutApron(Pos, Params.VertexCount)].Z =
		FHashUtil::RandDisp(Pos, Params.Size, Avg, Params.Seed);
}

int FGenerateUtil::GetIndexWithoutApron(const FVector2D& Pos
										, const uint32 VertexCount)
{
	return (Pos.Y + 1) * (VertexCount + 2) + Pos.X + 1;
}
