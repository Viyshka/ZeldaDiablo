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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	bool bUsePlacedPlayerCharacter = true;

protected:
	virtual void BeginPlay() override;
	virtual void RestartPlayer(AController* NewPlayer) override;

	void EnsurePrototypeActors();
	class AZDPlayerCharacter* FindPlacedPlayerCharacter(AController* ForController) const;
};
