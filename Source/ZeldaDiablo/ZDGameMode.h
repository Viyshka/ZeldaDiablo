#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ZDGameMode.generated.h"

UCLASS()
class ZELDADIABLO_API AZDGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AZDGameMode();

protected:
	virtual void BeginPlay() override;

	void EnsurePrototypeActors();
};

