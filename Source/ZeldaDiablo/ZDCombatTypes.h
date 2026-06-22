#pragma once

#include "CoreMinimal.h"
#include "ZDCombatTypes.generated.h"

UENUM(BlueprintType)
enum class EZDPlayerCombatState : uint8
{
	Idle,
	Moving,
	Attacking,
	Blocking,
	CounterAttacking,
	Hurt,
	Dead
};

UENUM(BlueprintType)
enum class EZDEnemyState : uint8
{
	Idle,
	Chasing,
	Windup,
	Attacking,
	Hurt,
	Dead
};

