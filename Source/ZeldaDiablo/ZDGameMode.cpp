#include "ZDGameMode.h"

#include "GameFramework/Controller.h"
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

void AZDGameMode::RestartPlayer(AController* NewPlayer)
{
	if (bUsePlacedPlayerCharacter && NewPlayer)
	{
		if (APawn* CurrentPawn = NewPlayer->GetPawn())
		{
			return;
		}

		if (AZDPlayerCharacter* PlacedPlayer = FindPlacedPlayerCharacter(NewPlayer))
		{
			NewPlayer->Possess(PlacedPlayer);
			return;
		}
	}

	Super::RestartPlayer(NewPlayer);
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

AZDPlayerCharacter* AZDGameMode::FindPlacedPlayerCharacter(AController* ForController) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	AZDPlayerCharacter* FirstAvailablePlayer = nullptr;

	for (TActorIterator<AZDPlayerCharacter> PlayerIterator(World); PlayerIterator; ++PlayerIterator)
	{
		AZDPlayerCharacter* PlayerCharacter = *PlayerIterator;
		if (!PlayerCharacter || PlayerCharacter->IsPendingKillPending())
		{
			continue;
		}

		if (PlayerCharacter->GetController() == ForController)
		{
			return PlayerCharacter;
		}

		if (PlayerCharacter->GetController())
		{
			continue;
		}

		if (!FirstAvailablePlayer)
		{
			FirstAvailablePlayer = PlayerCharacter;
		}

		if (PlayerCharacter->AutoPossessPlayer == EAutoReceiveInput::Player0)
		{
			return PlayerCharacter;
		}
	}

	return FirstAvailablePlayer;
}
