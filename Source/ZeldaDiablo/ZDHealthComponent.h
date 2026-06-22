#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ZDHealthComponent.generated.h"

class UZDHealthComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FZDHealthChangedSignature, UZDHealthComponent*, HealthComponent, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FZDDeathSignature, UZDHealthComponent*, HealthComponent);

UCLASS(ClassGroup = (ZeldaDiablo), meta = (BlueprintSpawnableComponent))
class ZELDADIABLO_API UZDHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UZDHealthComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (ClampMin = "1.0"))
	float MaxHealth = 3.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Health")
	float CurrentHealth = 0.0f;

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FZDHealthChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FZDDeathSignature OnDeath;

	UFUNCTION(BlueprintCallable, Category = "Health")
	void ResetHealth();

	UFUNCTION(BlueprintCallable, Category = "Health")
	bool ApplyDamage(float DamageAmount);

	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsDead() const;

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealth() const { return CurrentHealth; }

protected:
	virtual void BeginPlay() override;
};

