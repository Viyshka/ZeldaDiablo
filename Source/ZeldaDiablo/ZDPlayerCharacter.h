#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ZDCombatTypes.h"
#include "ZDPlayerCharacter.generated.h"

class AZDBasicEnemy;
class UStaticMeshComponent;
class UZDHealthComponent;

UCLASS()
class ZELDADIABLO_API AZDPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AZDPlayerCharacter();

	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UZDHealthComponent* HealthComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* PrototypeMesh = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	EZDPlayerCombatState CombatState = EZDPlayerCombatState::Idle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0"))
	float MovementSpeed = 550.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BlockingMovementMultiplier = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bCanMoveWhileBlocking = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (ClampMin = "0.0"))
	float BasicAttackDamage = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (ClampMin = "0.0"))
	float BasicAttackRadius = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (ClampMin = "0.0", ClampMax = "360.0"))
	float BasicAttackAngle = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (ClampMin = "0.0"))
	float BasicAttackDuration = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (ClampMin = "0.0"))
	float BasicAttackRecovery = 0.12f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	bool bHoldAttackChains = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Block", meta = (ClampMin = "0.0", ClampMax = "360.0"))
	float BlockAngle = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Counter", meta = (ClampMin = "0.0"))
	float CounterRadius = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Counter", meta = (ClampMin = "0.0", ClampMax = "360.0"))
	float CounterAngle = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Counter", meta = (ClampMin = "0.0"))
	float CounterLockDuration = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (ClampMin = "0.0"))
	float HurtDuration = 0.25f;

	UFUNCTION(BlueprintPure, Category = "Combat")
	FVector GetFacingDirection() const { return LastFacingDirection; }

	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool ReceiveEnemyAttack(AActor* DamageCauser, float DamageAmount);

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsAlive() const;

protected:
	virtual void BeginPlay() override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void HandleAttackPressed();
	void HandleAttackReleased();
	void HandleBlockPressed();
	void HandleBlockReleased();

	bool CanMove() const;
	bool CanStartCombatAction() const;
	bool CanBlock() const;
	void RefreshBlockState();
	void StartBasicAttack();
	void FinishBasicAttack();
	void ApplyBasicAttackDamage();
	bool TryStartCounterAttack();
	void FinishCounterAttack();
	AZDBasicEnemy* FindCounterTarget() const;
	bool IsDamageBlockedFrom(const FVector& DamageSourceLocation) const;
	void EnterHurtState();
	void FinishHurtState();

	UFUNCTION()
	void HandleDeath(UZDHealthComponent* DeadHealthComponent);

private:
	FVector PendingMovementInput = FVector::ZeroVector;
	FVector LastFacingDirection = FVector::ForwardVector;
	bool bBlockInputHeld = false;
	bool bIsBlocking = false;
	bool bWantsAttackChain = false;
	FTimerHandle AttackTimerHandle;
	FTimerHandle CounterTimerHandle;
	FTimerHandle HurtTimerHandle;
};

