#include "ZDGameMode.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "ZDCombatCameraActor.h"
#include "ZDCombatRoom.h"
#include "ZDPlayerCharacter.h"

AZDGameMode::AZDGameMode()
{
	DefaultPawnClass = AZDPlayerCharacter::StaticClass();
}

void AZDGameMode::BeginPlay()
{
	Super::BeginPlay();
	EnsurePrototypeActors();
}

void AZDGameMode::EnsurePrototypeActors()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	bool bHasRoom = false;
	for (TActorIterator<AZDCombatRoom> RoomIterator(World); RoomIterator; ++RoomIterator)
	{
		bHasRoom = true;
		break;
	}

	bool bHasCamera = false;
	for (TActorIterator<AZDCombatCameraActor> CameraIterator(World); CameraIterator; ++CameraIterator)
	{
		bHasCamera = true;
		break;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (!bHasRoom)
	{
		World->SpawnActor<AZDCombatRoom>(AZDCombatRoom::StaticClass(), FTransform::Identity, SpawnParameters);
	}

	if (!bHasCamera)
	{
		World->SpawnActor<AZDCombatCameraActor>(AZDCombatCameraActor::StaticClass(), FTransform::Identity, SpawnParameters);
	}
}

