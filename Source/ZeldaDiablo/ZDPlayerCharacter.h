#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ZDCombatTypes.h"
#include "ZDPlayerCharacter.generated.h"

class AZDBasicEnemy;
class UAnimSequence;
class USkeletalMesh;
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	USkeletalMesh* CharacterMeshAsset = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals|Animation")
	UAnimSequence* IdleAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals|Animation")
	UAnimSequence* MovementAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals|Animation")
	UAnimSequence* BasicAttackAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals|Animation")
	UAnimSequence* BlockAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals|Animation")
	UAnimSequence* CounterAttackAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals|Animation")
	UAnimSequence* HurtAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals|Animation")
	UAnimSequence* DeathAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals|Animation", meta = (ClampMin = "0.0"))
	float IdlePoseTime = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals|Animation", meta = (ClampMin = "0.0"))
	float BlockPoseTime = 0.28f;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	EZDPlayerCombatState CombatState = EZDPlayerCombatState::Idle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0"))
	float MovementSpeed = 550.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BlockingMovementMultiplier = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bCanMoveWhileBlocking = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MovementInputDeadZone = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LookInputDeadZone = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim", meta = (ClampMin = "0.0"))
	float FacingRotationSpeed = 720.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim")
	bool bInvertLookForward = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim")
	bool bInvertLookRight = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (ClampMin = "0.0"))
	float BasicAttackDamage = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (ClampMin = "0.0"))
	float BasicAttackRadius = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (ClampMin = "0.0", ClampMax = "360.0"))
	float BasicAttackAngle = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (ClampMin = "0.0"))
	float BasicAttackDuration = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (ClampMin = "0.0"))
	float BasicAttackHitDelay = 0.16f;

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
	void LookForward(float Value);
	void LookRight(float Value);
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
	void ConfigureVisuals();
	bool HasDirectionalInput(const FVector& Input, float DeadZone) const;
	bool CanUpdateFacing() const;
	void UpdateDesiredFacingFromInput(const FVector& Input);
	void ApplyFacingRotation(float DeltaSeconds);
	void PlayLoopAnimation(UAnimSequence* Animation);
	void HoldPoseAnimation(UAnimSequence* Animation, float PoseTime);
	void PlayOneShotAnimation(UAnimSequence* Animation);
	void PlayCurrentLoopAnimation();

	UFUNCTION()
	void HandleDeath(UZDHealthComponent* DeadHealthComponent);

private:
	FVector PendingMovementInput = FVector::ZeroVector;
	FVector PendingLookInput = FVector::ZeroVector;
	FVector DesiredFacingDirection = FVector::ForwardVector;
	FVector LastFacingDirection = FVector::ForwardVector;
	bool bBlockInputHeld = false;
	bool bIsBlocking = false;
	bool bWantsAttackChain = false;
	UPROPERTY()
	UAnimSequence* ActiveAnimation = nullptr;
	FTimerHandle AttackTimerHandle;
	FTimerHandle AttackHitTimerHandle;
	FTimerHandle CounterTimerHandle;
	FTimerHandle HurtTimerHandle;
};
