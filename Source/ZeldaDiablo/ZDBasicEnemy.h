#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ZDCombatTypes.h"
#include "ZDBasicEnemy.generated.h"

class AZDPlayerCharacter;
class UAnimSequence;
class UMaterialInstanceDynamic;
class USkeletalMesh;
class UStaticMeshComponent;
class UZDHealthComponent;

UCLASS()
class ZELDADIABLO_API AZDBasicEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	AZDBasicEnemy();

	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UZDHealthComponent* HealthComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* PrototypeMesh = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* StateMarkerMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	USkeletalMesh* CharacterMeshAsset = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals|Animation")
	UAnimSequence* IdleAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals|Animation")
	UAnimSequence* ChaseAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals|Animation")
	UAnimSequence* WindupAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals|Animation")
	UAnimSequence* AttackAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals|Animation")
	UAnimSequence* HurtAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals|Animation")
	UAnimSequence* DeathAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals|Animation", meta = (ClampMin = "0.0"))
	float IdlePoseTime = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals|Animation", meta = (ClampMin = "0.0"))
	float WindupPoseTime = 0.34f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals|Readability")
	bool bShowStateMarker = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals|Readability", meta = (ClampMin = "0.1"))
	float StateMarkerScale = 1.35f;

	UPROPERTY(BlueprintReadOnly, Category = "AI")
	EZDEnemyState EnemyState = EZDEnemyState::Idle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (ClampMin = "0.0"))
	float MovementSpeed = 260.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (ClampMin = "0.0"))
	float DetectionRadius = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (ClampMin = "0.0"))
	float TurnSpeed = 720.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	bool bUseDirectChaseMovement = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (ClampMin = "0.0"))
	float DirectChaseStopDistance = 115.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (ClampMin = "0.0"))
	float AttackDamage = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (ClampMin = "0.0"))
	float AttackRadius = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (ClampMin = "0.0", ClampMax = "360.0"))
	float AttackAngle = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (ClampMin = "0.0"))
	float WindupDuration = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (ClampMin = "0.0"))
	float AttackRecovery = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (ClampMin = "0.0"))
	float AttackHitDelay = 0.12f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (ClampMin = "0.0"))
	float TimeBetweenAttacks = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (ClampMin = "0.0"))
	float HurtDuration = 0.18f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Counter")
	bool bCanBeCounterAttacked = true;

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsAlive() const;

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsCounterable() const;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ReceivePlayerHit(float DamageAmount, AActor* DamageCauser);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void KillByCounterAttack(AActor* CounterAttacker);

protected:
	virtual void BeginPlay() override;

	void AcquirePlayer();
	void UpdateIdle();
	void UpdateChase(float DeltaSeconds);
	void StartWindup();
	void FinishWindup();
	void ExecuteAttack();
	void FinishAttack();
	void ClearAttackCooldown();
	void EnterHurtState();
	void FinishHurtState();
	void FacePlayer(float DeltaSeconds);
	void MoveTowardPlayer(float DeltaSeconds);
	bool IsPlayerInRange(float Range) const;
	float GetDistanceToPlayer2D() const;
	bool IsPlayerInAttackCone() const;
	void InterruptCurrentAction();
	void ConfigureVisuals();
	void PlayLoopAnimation(UAnimSequence* Animation);
	void HoldPoseAnimation(UAnimSequence* Animation, float PoseTime);
	void PlayOneShotAnimation(UAnimSequence* Animation);
	void PlayCurrentLoopAnimation();
	void UpdateStateMarker();
	void SetStateMarkerColor(const FLinearColor& MarkerColor, bool bVisible);

	UFUNCTION()
	void HandleDeath(UZDHealthComponent* DeadHealthComponent);

private:
	UPROPERTY()
	AZDPlayerCharacter* TargetPlayer = nullptr;

	bool bHasDetectedPlayer = false;
	bool bAttackOnCooldown = false;
	UPROPERTY()
	UAnimSequence* ActiveAnimation = nullptr;
	UPROPERTY()
	UMaterialInstanceDynamic* StateMarkerMaterial = nullptr;
	FTimerHandle WindupTimerHandle;
	FTimerHandle AttackTimerHandle;
	FTimerHandle AttackHitTimerHandle;
	FTimerHandle AttackCooldownTimerHandle;
	FTimerHandle HurtTimerHandle;
};
