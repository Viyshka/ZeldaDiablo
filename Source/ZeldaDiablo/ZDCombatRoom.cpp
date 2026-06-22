#include "ZDCombatRoom.h"

#include "Engine/StaticMesh.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "ZDBasicEnemy.h"

AZDCombatRoom::AZDCombatRoom()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	FloorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Floor"));
	FloorMesh->SetupAttachment(SceneRoot);

	NorthWallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NorthWall"));
	NorthWallMesh->SetupAttachment(SceneRoot);

	SouthWallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SouthWall"));
	SouthWallMesh->SetupAttachment(SceneRoot);

	EastWallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EastWall"));
	EastWallMesh->SetupAttachment(SceneRoot);

	WestWallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WestWall"));
	WestWallMesh->SetupAttachment(SceneRoot);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		FloorMesh->SetStaticMesh(CubeMesh.Object);
		NorthWallMesh->SetStaticMesh(CubeMesh.Object);
		SouthWallMesh->SetStaticMesh(CubeMesh.Object);
		EastWallMesh->SetStaticMesh(CubeMesh.Object);
		WestWallMesh->SetStaticMesh(CubeMesh.Object);
	}

	EnemyClass = AZDBasicEnemy::StaticClass();
	EnemySpawnLocations.Add(FVector(450.0f, 0.0f, 100.0f));
	EnemySpawnLocations.Add(FVector(700.0f, 320.0f, 100.0f));
	EnemySpawnLocations.Add(FVector(700.0f, -320.0f, 100.0f));

	UpdateRoomGeometry();
}

void AZDCombatRoom::BeginPlay()
{
	Super::BeginPlay();

	MovePlayerToSpawn();

	if (bSpawnEnemiesOnBeginPlay)
	{
		SpawnEnemies();
	}
}

void AZDCombatRoom::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	UpdateRoomGeometry();
}

void AZDCombatRoom::UpdateRoomGeometry()
{
	if (!FloorMesh || !NorthWallMesh || !SouthWallMesh || !EastWallMesh || !WestWallMesh)
	{
		return;
	}

	FloorMesh->SetRelativeLocation(FVector::ZeroVector);
	FloorMesh->SetRelativeScale3D(FVector(RoomWidth / 100.0f, RoomHeight / 100.0f, 0.08f));
	FloorMesh->SetCollisionProfileName(TEXT("BlockAll"));

	const float HalfWidth = RoomWidth * 0.5f;
	const float HalfHeight = RoomHeight * 0.5f;
	const float WallZ = WallHeight * 0.5f;

	NorthWallMesh->SetRelativeLocation(FVector(0.0f, HalfHeight + WallThickness * 0.5f, WallZ));
	NorthWallMesh->SetRelativeScale3D(FVector((RoomWidth + WallThickness * 2.0f) / 100.0f, WallThickness / 100.0f, WallHeight / 100.0f));
	NorthWallMesh->SetCollisionProfileName(TEXT("BlockAll"));

	SouthWallMesh->SetRelativeLocation(FVector(0.0f, -HalfHeight - WallThickness * 0.5f, WallZ));
	SouthWallMesh->SetRelativeScale3D(FVector((RoomWidth + WallThickness * 2.0f) / 100.0f, WallThickness / 100.0f, WallHeight / 100.0f));
	SouthWallMesh->SetCollisionProfileName(TEXT("BlockAll"));

	EastWallMesh->SetRelativeLocation(FVector(HalfWidth + WallThickness * 0.5f, 0.0f, WallZ));
	EastWallMesh->SetRelativeScale3D(FVector(WallThickness / 100.0f, RoomHeight / 100.0f, WallHeight / 100.0f));
	EastWallMesh->SetCollisionProfileName(TEXT("BlockAll"));

	WestWallMesh->SetRelativeLocation(FVector(-HalfWidth - WallThickness * 0.5f, 0.0f, WallZ));
	WestWallMesh->SetRelativeScale3D(FVector(WallThickness / 100.0f, RoomHeight / 100.0f, WallHeight / 100.0f));
	WestWallMesh->SetCollisionProfileName(TEXT("BlockAll"));
}

void AZDCombatRoom::MovePlayerToSpawn() const
{
	if (ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0))
	{
		PlayerCharacter->SetActorLocation(PlayerSpawnLocation);
		PlayerCharacter->SetActorRotation(FRotator::ZeroRotator);
	}
}

void AZDCombatRoom::SpawnEnemies()
{
	UWorld* World = GetWorld();
	if (!World || !EnemyClass)
	{
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	for (int32 Index = 0; Index < EnemyCount; ++Index)
	{
		const FVector SpawnLocation = GetEnemySpawnLocation(Index);
		World->SpawnActor<AZDBasicEnemy>(EnemyClass, SpawnLocation, FRotator(0.0f, 180.0f, 0.0f), SpawnParameters);
	}
}

FVector AZDCombatRoom::GetEnemySpawnLocation(int32 SpawnIndex) const
{
	if (EnemySpawnLocations.IsValidIndex(SpawnIndex))
	{
		return EnemySpawnLocations[SpawnIndex];
	}

	const float StepY = 220.0f;
	const float StartY = -StepY * FMath::Max(0, EnemyCount - 1) * 0.5f;
	return FVector(RoomWidth * 0.22f, StartY + StepY * SpawnIndex, 100.0f);
}

