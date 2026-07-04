#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZDCombatRoom.generated.h"

class AZDBasicEnemy;
class UStaticMeshComponent;
class USceneComponent;

UCLASS()
class ZELDADIABLO_API AZDCombatRoom : public AActor
{
	GENERATED_BODY()

public:
	AZDCombatRoom();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* SceneRoot = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* FloorMesh = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* NorthWallMesh = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* SouthWallMesh = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* EastWallMesh = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* WestWallMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room", meta = (ClampMin = "500.0"))
	float RoomWidth = 2200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room", meta = (ClampMin = "500.0"))
	float RoomHeight = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room", meta = (ClampMin = "10.0"))
	float WallThickness = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room", meta = (ClampMin = "10.0"))
	float WallHeight = 220.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	FVector PlayerSpawnLocation = FVector(-650.0f, 0.0f, 100.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	bool bMovePlayerToSpawnOnBeginPlay = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	TSubclassOf<AZDBasicEnemy> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning", meta = (ClampMin = "0"))
	int32 EnemyCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	TArray<FVector> EnemySpawnLocations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	bool bSpawnEnemiesOnBeginPlay = true;

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	void UpdateRoomGeometry();
	void MovePlayerToSpawn() const;
	void SpawnEnemies();
	FVector GetEnemySpawnLocation(int32 SpawnIndex) const;
};
