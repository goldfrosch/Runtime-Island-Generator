#include "IslandGenerator.h"

#include "ProceduralMeshComponent/Public/KismetProceduralMeshLibrary.h"
#include "ProceduralMeshComponent/Public/ProceduralMeshComponent.h"

#include "Util/GenerateUtil.h"
#include "Util/NoiseUtil.h"

AIslandGenerator::AIslandGenerator()
{
	RootComp = CreateDefaultSubobject<USceneComponent>("Root Comp");
	SetRootComponent(RootComp);

	TerrainMesh = CreateDefaultSubobject<UProceduralMeshComponent>(
		"Terrain Mesh");
	TerrainMesh->SetupAttachment(GetRootComponent());
}

void AIslandGenerator::GenerateTerrain()
{
	TerrainMesh->ClearAllMeshSections();
	TerrainMesh->ClearCollisionConvexMeshes();

	for (int Y = 0; Y < YTileSize; Y++)
	{
		for (int X = 0; X < XTileSize; X++)
		{
			TArray<FVector> Vertices;
			// Normal과 Tangents 계산용 삼각형 폴리곤
			TArray<int32> CalcTriangles;
			// 실제 Mesh에 사용할 삼각형 폴리곤
			TArray<int32> Triangles;
			TArray<FVector2d> UV0s;
			TArray<FVector> Normals;
			TArray<FProcMeshTangent> Tangents;

			CalculateTerrainData_Internal(Vertices, UV0s, X, Y);
			CalculateTriangle_Internal(CalcTriangles, Triangles);

			UKismetProceduralMeshLibrary::CalculateTangentsForMesh(
				Vertices, CalcTriangles, UV0s, Normals, Tangents);

			FilterTerrainData_Internal(Vertices, UV0s, Normals, Tangents);

			// Create Mesh Section 쪽만 비동기로 처리해도 됨.
			const uint16 SectionIndex = Y * XTileSize + X;
			TerrainMesh->CreateMeshSection(SectionIndex, Vertices, Triangles
											, Normals, UV0s, TArray<FColor>()
											, Tangents, true);
		}
	}

	UE_LOG(LogTemp, Display, TEXT("지형 생성 완료: %s")
			, *FDateTime::Now().ToString());
}

void AIslandGenerator::CalculateTerrainData_Internal(TArray<FVector>& Vertices
	, TArray<FVector2d>& UV0s, const uint16 XTileIndex
	, const uint16 YTileIndex) const
{
	for (int32 y = -1; y <= VertexCount; y++)
	{
		for (int32 x = -1; x <= VertexCount; x++)
		{
			// 타일 섹션 위치에 따른 X, Y Index 값
			const int32 XIndex = (VertexCount - 1) * XTileIndex + x;
			const int32 YIndex = (VertexCount - 1) * YTileIndex + y;

			// Vertices에 넣을 포지션 정보 먼저 구함
			const int32 XPos = CellSize * XIndex;
			const int32 YPos = CellSize * YIndex;

			const float Height = FNoiseUtil::Height_Mountains(
				FVector2D(XPos, YPos) * 0.006f, Seed, MountainBiomeFractal
				, MountainBiomeWarp, FVector2D(1, 0));

			const FVector VertexPos = FVector(XPos, YPos, Height * MaxHeight);

			Vertices.Add(VertexPos);

			// UV 값 추가
			UV0s.Add(FVector2d(XIndex, YIndex));
		}
	}

	FGenerateUtil::DiamondSquare(Vertices, VertexCount, {
									VertexCount, Seed
									, FVector2D(XTileIndex, YTileIndex)
									, CellSize
								});
}

void AIslandGenerator::CalculateTriangle_Internal(TArray<int32>& CalcTriangles
												, TArray<int32>& Triangles)
const
{
	// 뒤로, 앞으로 한칸씩 땡겨서 값을 저장했기 때문에 0부터 VertexCount까지 가도 1칸이 남는다.
	// 마지막 줄에서는 순회하면서 삼각형을 만들지 않기 때문에 이렇게 처리하는 것이 맞음.
	for (int32 y = 0; y <= VertexCount; y++)
	{
		for (int32 x = 0; x <= VertexCount; x++)
		{
			GenerateCalcTriangle_Internal(CalcTriangles, x, y);

			if (y < VertexCount - 1 && x < VertexCount - 1)
			{
				GenerateTerrainTriangle_Internal(Triangles, x, y);
			}
		}
	}
}

void AIslandGenerator::GenerateCalcTriangle_Internal(TArray<int32>& Triangles
	, const int32 XIndex, const int32 YIndex) const
{
	const uint32 CurrentTriangleVertexCount = VertexCount + 2;
	const uint32 ArrayIndex = CurrentTriangleVertexCount * YIndex + XIndex;

	// 각 꼭지점 값을 3개 전달해 줘야함, 처음에는 시작 위치 점
	Triangles.Add(ArrayIndex);
	// 그 다음 우측으로 갈 지점이기에 기존 Index에 X Vertex를 더해 옆으로 한칸 이동
	Triangles.Add(ArrayIndex + CurrentTriangleVertexCount);
	// 현재 Index에 위로 한칸 이동해 삼각형을 만듬
	Triangles.Add(ArrayIndex + 1);

	// 아래 3개의 점은 삼각형을 반대로 해서 채워주기 위함
	Triangles.Add(ArrayIndex + 1);
	Triangles.Add(ArrayIndex + CurrentTriangleVertexCount);
	Triangles.Add(ArrayIndex + CurrentTriangleVertexCount + 1);
}

void AIslandGenerator::GenerateTerrainTriangle_Internal(TArray<int32>& Triangles
	, const int32 XIndex, const int32 YIndex) const
{
	const uint32 ArrayIndex = VertexCount * YIndex + XIndex;

	// 각 꼭지점 값을 3개 전달해 줘야함, 처음에는 시작 위치 점
	Triangles.Add(ArrayIndex);
	// 그 다음 우측으로 갈 지점이기에 기존 Index에 X Vertex를 더해 옆으로 한칸 이동
	Triangles.Add(ArrayIndex + VertexCount);
	// 현재 Index에 위로 한칸 이동해 삼각형을 만듬
	Triangles.Add(ArrayIndex + 1);

	// 아래 3개의 점은 삼각형을 반대로 해서 채워주기 위함
	Triangles.Add(ArrayIndex + 1);
	Triangles.Add(ArrayIndex + VertexCount);
	Triangles.Add(ArrayIndex + VertexCount + 1);
}

void AIslandGenerator::FilterTerrainData_Internal(TArray<FVector>& Vertices
												, TArray<FVector2d>& UV0s
												, TArray<FVector>& Normals
												, TArray<FProcMeshTangent>&
												Tangents) const
{
	TArray<FVector> TempVertices = MoveTemp(Vertices);
	TArray<FVector2d> TempUV0s = MoveTemp(UV0s);
	TArray<FVector> TempNormals = MoveTemp(Normals);
	TArray<FProcMeshTangent> TempTangents = MoveTemp(Tangents);

	// 순회 하면서 기존 값들 전체 제거 처리
	uint32 Index = 0;
	for (int32 y = -1; y <= VertexCount; y++)
	{
		for (int32 x = -1; x <= VertexCount; x++)
		{
			if (x >= 0 && y >= 0 && x < VertexCount && y < VertexCount)
			{
				Vertices.Add(TempVertices[Index]);
				UV0s.Add(TempUV0s[Index]);
				Normals.Add(TempNormals[Index]);
				Tangents.Emplace(TempTangents[Index]);
			}
			Index += 1;
		}
	}
}
