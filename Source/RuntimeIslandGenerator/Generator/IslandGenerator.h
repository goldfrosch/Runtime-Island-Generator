#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IslandGenerator.generated.h"

class UProceduralMeshComponent;
struct FProcMeshTangent;

UCLASS()
class RUNTIMEISLANDGENERATOR_API AIslandGenerator : public AActor
{
	GENERATED_BODY()

public:
	AIslandGenerator();

	UFUNCTION(CallInEditor)
	void GenerateTerrain();

protected:
	UPROPERTY()
	TObjectPtr<USceneComponent> RootComp;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UProceduralMeshComponent> TerrainMesh;

	UPROPERTY(EditInstanceOnly, Category = "Options|Terrain"
		, meta = (AllowPrivateAccess = true))
	bool IsDebugMode = true;

	UPROPERTY(EditInstanceOnly, Category = "Options|Terrain"
		, meta = (AllowPrivateAccess = true))
	int32 Seed = 0;

	UPROPERTY(EditInstanceOnly, Category = "Options|Terrain|Base"
		, meta = (AllowPrivateAccess = true, ClampMin = "1"))
	uint16 VertexCount = 1;

	UPROPERTY(EditInstanceOnly, Category = "Options|Terrain|Base"
		, meta = (AllowPrivateAccess = true))
	uint16 XTileSize = 4;

	UPROPERTY(EditInstanceOnly, Category = "Options|Terrain|Base"
		, meta = (AllowPrivateAccess = true))
	uint16 YTileSize = 4;

	UPROPERTY(EditInstanceOnly, Category = "Options|Terrain|Base"
		, meta = (AllowPrivateAccess = true, ClampMin = "0.01"))
	float CellSize = 8.f;

	UPROPERTY(EditInstanceOnly, Category = "Options|Terrain|Base"
		, meta = (AllowPrivateAccess = true, ClampMin = "0"))
	float MaxHeight = 100.f;

	void CalculateTerrainData_Internal(TArray<FVector>& Vertices
										, TArray<FVector2d>& UV0s
										, const uint16 XTileIndex
										, const uint16 YTileIndex) const;

	void CalculateTriangle_Internal(TArray<int32>& CalcTriangles
									, TArray<int32>& Triangles) const;

	void GenerateCalcTriangle_Internal(TArray<int32>& Triangles
										, const int32 XIndex
										, const int32 YIndex) const;
	void GenerateTerrainTriangle_Internal(TArray<int32>& Triangles
										, const int32 XIndex
										, const int32 YIndex) const;

	void FilterTerrainData_Internal(TArray<FVector>& Vertices
									, TArray<FVector2d>& UV0s
									, TArray<FVector>& Normals
									, TArray<FProcMeshTangent>& Tangents) const;
};
