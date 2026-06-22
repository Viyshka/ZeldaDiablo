#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ZDCombatLibrary.generated.h"

UCLASS()
class ZELDADIABLO_API UZDCombatLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "ZeldaDiablo|Combat")
	static bool IsTargetInCone(const FVector& Origin, const FVector& Forward, const FVector& TargetLocation, float Radius, float AngleDegrees);

	UFUNCTION(BlueprintPure, Category = "ZeldaDiablo|Combat")
	static FVector FlattenDirection(const FVector& Direction);
};

