#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IslandGenerator.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnIslandInitializeSuccess);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnIslandChunkLoadedSuccess
											, const uint16, X, const uint16, Y);

class UProceduralMeshComponent;
struct FProcMeshTangent;

UCLASS()
class RUNTIMEISLANDGENERATOR_API AIslandGenerator : public AActor
{
	GENERATED_BODY()

public:
	AIslandGenerator();

	UFUNCTION(CallInEditor)
	void InitializeChunks();

	UFUNCTION(CallInEditor)
	void RandomSeed();

	void LoadChunk(const uint16 X, const uint16 Y);

	FOnIslandInitializeSuccess OnIslandInitializeSuccess;
	FOnIslandChunkLoadedSuccess OnIslandChunkLoadedSuccess;

protected:
	UPROPERTY()
	TObjectPtr<USceneComponent> RootComp;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UProceduralMeshComponent> TerrainMesh;

	UPROPERTY(EditInstanceOnly, Category = "Options|Terrain"
		, meta = (AllowPrivateAccess = true))
	int32 Seed = 0;

	UPROPERTY(EditInstanceOnly, Category = "Options|Terrain|Base"
		, meta = (AllowPrivateAccess = true, ClampMin = "1"))
	uint16 VertexCount = 1;

	UPROPERTY(EditInstanceOnly, Category = "Options|Terrain|Base"
		, meta = (AllowPrivateAccess = true, ClampMin = "1"))
	uint16 MaxHeight = 512;

	UPROPERTY(EditInstanceOnly, Category = "Options|Terrain|Base"
		, meta = (AllowPrivateAccess = true))
	uint16 XTileSize = 4;

	UPROPERTY(EditInstanceOnly, Category = "Options|Terrain|Base"
		, meta = (AllowPrivateAccess = true))
	uint16 YTileSize = 4;

	UPROPERTY(EditInstanceOnly, Category = "Options|Terrain|Base"
		, meta = (AllowPrivateAccess = true, ClampMin = "0"))
	int32 CellSize = 8;

	UPROPERTY(EditInstanceOnly, Category = "Options|Terrain|Base"
		, meta = (AllowPrivateAccess = true))
	int32 MinHeight = -128;

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

private:
	TArray<bool> InitializedChunkInfo;

	bool IsInitialized = false;

	bool IsLoading = false;
};
