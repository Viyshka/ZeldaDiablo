#include "ZDBasicEnemy.h"

#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimSequence.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"
#include "ZDCombatLibrary.h"
#include "ZDHealthComponent.h"
#include "ZDPlayerCharacter.h"

AZDBasicEnemy::AZDBasicEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(34.0f, 82.0f);
	bUseControllerRotationYaw = false;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->MaxWalkSpeed = MovementSpeed;
	GetCharacterMovement()->BrakingDecelerationWalking = 2048.0f;

	HealthComponent = CreateDefaultSubobject<UZDHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->MaxHealth = 1.0f;

	PrototypeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PrototypeMesh"));
	PrototypeMesh->SetupAttachment(RootComponent);
	PrototypeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PrototypeMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -8.0f));
	PrototypeMesh->SetRelativeScale3D(FVector(0.65f, 0.65f, 1.45f));

	StateMarkerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StateMarker"));
	StateMarkerMesh->SetupAttachment(RootComponent);
	StateMarkerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StateMarkerMesh->SetCanEverAffectNavigation(false);
	StateMarkerMesh->SetCastShadow(false);
	StateMarkerMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -79.0f));
	StateMarkerMesh->SetRelativeScale3D(FVector(StateMarkerScale, StateMarkerScale, 0.025f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		PrototypeMesh->SetStaticMesh(CubeMesh.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderMesh.Succeeded())
	{
		StateMarkerMesh->SetStaticMesh(CylinderMesh.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MarkerMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (MarkerMaterial.Succeeded())
	{
		StateMarkerMesh->SetMaterial(0, MarkerMaterial.Object);
	}

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> QuinnMesh(TEXT("/Game/FreeAnimsMixPack/Demo/Mannequins/Meshes/SKM_Quinn.SKM_Quinn"));
	if (QuinnMesh.Succeeded())
	{
		CharacterMeshAsset = QuinnMesh.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequence> PracticeFencingAnim(TEXT("/Game/FreeAnimsMixPack/Animation/AS_PracticeFencing.AS_PracticeFencing"));
	if (PracticeFencingAnim.Succeeded())
	{
		IdleAnimation = PracticeFencingAnim.Object;
		WindupAnimation = PracticeFencingAnim.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequence> SwingSwordAnim(TEXT("/Game/FreeAnimsMixPack/Animation/AS_SwingSword.AS_SwingSword"));
	if (SwingSwordAnim.Succeeded())
	{
		AttackAnimation = SwingSwordAnim.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequence> DyingAnim(TEXT("/Game/FreeAnimsMixPack/Animation/AS_DyingFromWounds.AS_DyingFromWounds"));
	if (DyingAnim.Succeeded())
	{
		DeathAnimation = DyingAnim.Object;
	}

	ConfigureVisuals();
}

void AZDBasicEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (!GetController())
	{
		SpawnDefaultController();
	}

	GetCharacterMovement()->MaxWalkSpeed = MovementSpeed;
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	AcquirePlayer();

	if (HealthComponent)
	{
		HealthComponent->OnDeath.AddDynamic(this, &AZDBasicEnemy::HandleDeath);
	}

	ConfigureVisuals();
	PlayCurrentLoopAnimation();
	UpdateStateMarker();
}

void AZDBasicEnemy::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	GetCharacterMovement()->MaxWalkSpeed = MovementSpeed;

	if (!IsAlive())
	{
		return;
	}

	if (!TargetPlayer || !TargetPlayer->IsAlive())
	{
		AcquirePlayer();
	}

	if (!TargetPlayer || !TargetPlayer->IsAlive())
	{
		EnemyState = EZDEnemyState::Idle;
		PlayCurrentLoopAnimation();
		UpdateStateMarker();
		return;
	}

	switch (EnemyState)
	{
	case EZDEnemyState::Idle:
		UpdateIdle();
		break;
	case EZDEnemyState::Chasing:
		UpdateChase(DeltaSeconds);
		break;
	case EZDEnemyState::Windup:
	case EZDEnemyState::Attacking:
		FacePlayer(DeltaSeconds);
		break;
	default:
		break;
	}
}

void AZDBasicEnemy::AcquirePlayer()
{
	TargetPlayer = Cast<AZDPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
}

void AZDBasicEnemy::UpdateIdle()
{
	if (!TargetPlayer)
	{
		return;
	}

	if (IsPlayerInRange(DetectionRadius))
	{
		bHasDetectedPlayer = true;
		EnemyState = EZDEnemyState::Chasing;
		PlayCurrentLoopAnimation();
		UpdateStateMarker();
	}
}

void AZDBasicEnemy::UpdateChase(float DeltaSeconds)
{
	if (!TargetPlayer)
	{
		return;
	}

	FacePlayer(DeltaSeconds);

	const float DistanceToPlayer = GetDistanceToPlayer2D();
	if (!bAttackOnCooldown && DistanceToPlayer <= AttackRadius)
	{
		StartWindup();
		return;
	}

	MoveTowardPlayer(DeltaSeconds);
}

void AZDBasicEnemy::StartWindup()
{
	EnemyState = EZDEnemyState::Windup;
	PlayCurrentLoopAnimation();
	UpdateStateMarker();
	GetWorldTimerManager().SetTimer(WindupTimerHandle, this, &AZDBasicEnemy::FinishWindup, WindupDuration, false);
}

void AZDBasicEnemy::FinishWindup()
{
	if (!IsAlive())
	{
		return;
	}

	EnemyState = EZDEnemyState::Attacking;
	PlayOneShotAnimation(AttackAnimation);
	UpdateStateMarker();

	GetWorldTimerManager().ClearTimer(AttackHitTimerHandle);
	const float HitDelay = FMath::Min(AttackHitDelay, AttackRecovery);
	if (HitDelay <= 0.0f)
	{
		ExecuteAttack();
	}
	else
	{
		GetWorldTimerManager().SetTimer(AttackHitTimerHandle, this, &AZDBasicEnemy::ExecuteAttack, HitDelay, false);
	}

	GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &AZDBasicEnemy::FinishAttack, AttackRecovery, false);
}

void AZDBasicEnemy::ExecuteAttack()
{
	if (TargetPlayer && TargetPlayer->IsAlive() && IsPlayerInAttackCone())
	{
		TargetPlayer->ReceiveEnemyAttack(this, AttackDamage);
	}
}

void AZDBasicEnemy::FinishAttack()
{
	if (!IsAlive())
	{
		return;
	}

	bAttackOnCooldown = true;
	EnemyState = EZDEnemyState::Chasing;
	PlayCurrentLoopAnimation();
	UpdateStateMarker();
	GetWorldTimerManager().SetTimer(AttackCooldownTimerHandle, this, &AZDBasicEnemy::ClearAttackCooldown, TimeBetweenAttacks, false);
}

void AZDBasicEnemy::ClearAttackCooldown()
{
	bAttackOnCooldown = false;
}

void AZDBasicEnemy::ReceivePlayerHit(float DamageAmount, AActor* DamageCauser)
{
	if (!IsAlive() || !HealthComponent)
	{
		return;
	}

	InterruptCurrentAction();

	if (HealthComponent->ApplyDamage(DamageAmount) && IsAlive())
	{
		EnterHurtState();
	}
}

void AZDBasicEnemy::KillByCounterAttack(AActor* CounterAttacker)
{
	if (!IsAlive() || !HealthComponent)
	{
		return;
	}

	InterruptCurrentAction();
	HealthComponent->ApplyDamage(HealthComponent->GetHealth());
}

void AZDBasicEnemy::EnterHurtState()
{
	EnemyState = EZDEnemyState::Hurt;
	PlayOneShotAnimation(HurtAnimation);
	UpdateStateMarker();
	GetWorldTimerManager().SetTimer(HurtTimerHandle, this, &AZDBasicEnemy::FinishHurtState, HurtDuration, false);
}

void AZDBasicEnemy::FinishHurtState()
{
	if (!IsAlive())
	{
		return;
	}

	EnemyState = bHasDetectedPlayer ? EZDEnemyState::Chasing : EZDEnemyState::Idle;
	PlayCurrentLoopAnimation();
	UpdateStateMarker();
}

bool AZDBasicEnemy::IsAlive() const
{
	return HealthComponent && !HealthComponent->IsDead();
}

bool AZDBasicEnemy::IsCounterable() const
{
	return bCanBeCounterAttacked && IsAlive() && EnemyState == EZDEnemyState::Windup;
}

void AZDBasicEnemy::FacePlayer(float DeltaSeconds)
{
	if (!TargetPlayer)
	{
		return;
	}

	FVector Direction = TargetPlayer->GetActorLocation() - GetActorLocation();
	Direction.Z = 0.0f;

	if (Direction.IsNearlyZero())
	{
		return;
	}

	const FRotator DesiredRotation = Direction.Rotation();
	const FRotator NewRotation = FMath::RInterpConstantTo(GetActorRotation(), DesiredRotation, DeltaSeconds, TurnSpeed);
	SetActorRotation(NewRotation);
}

void AZDBasicEnemy::MoveTowardPlayer(float DeltaSeconds)
{
	if (!TargetPlayer || DeltaSeconds <= 0.0f)
	{
		return;
	}

	FVector Direction = TargetPlayer->GetActorLocation() - GetActorLocation();
	Direction.Z = 0.0f;

	const float Distance = Direction.Size();
	if (Distance <= FMath::Max(DirectChaseStopDistance, AttackRadius * 0.75f) || Direction.IsNearlyZero())
	{
		return;
	}

	Direction /= Distance;

	if (bUseDirectChaseMovement)
	{
		AddActorWorldOffset(Direction * MovementSpeed * DeltaSeconds, true);
		return;
	}

	AddMovementInput(Direction, 1.0f, true);
}

bool AZDBasicEnemy::IsPlayerInRange(float Range) const
{
	return TargetPlayer && FVector::DistSquared2D(GetActorLocation(), TargetPlayer->GetActorLocation()) <= FMath::Square(Range);
}

float AZDBasicEnemy::GetDistanceToPlayer2D() const
{
	return TargetPlayer ? FVector::Dist2D(GetActorLocation(), TargetPlayer->GetActorLocation()) : TNumericLimits<float>::Max();
}

bool AZDBasicEnemy::IsPlayerInAttackCone() const
{
	return TargetPlayer
		&& UZDCombatLibrary::IsTargetInCone(GetActorLocation(), GetActorForwardVector(), TargetPlayer->GetActorLocation(), AttackRadius, AttackAngle);
}

void AZDBasicEnemy::InterruptCurrentAction()
{
	GetWorldTimerManager().ClearTimer(WindupTimerHandle);
	GetWorldTimerManager().ClearTimer(AttackTimerHandle);
	GetWorldTimerManager().ClearTimer(AttackHitTimerHandle);
	GetWorldTimerManager().ClearTimer(HurtTimerHandle);
}

void AZDBasicEnemy::HandleDeath(UZDHealthComponent* DeadHealthComponent)
{
	EnemyState = EZDEnemyState::Dead;
	PlayOneShotAnimation(DeathAnimation);
	UpdateStateMarker();
	GetWorldTimerManager().ClearAllTimersForObject(this);
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetLifeSpan(6.0f);
}

void AZDBasicEnemy::ConfigureVisuals()
{
	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (!MeshComponent)
	{
		return;
	}

	if (CharacterMeshAsset)
	{
		MeshComponent->SetSkeletalMesh(CharacterMeshAsset);
		MeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, -82.0f));
		MeshComponent->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		MeshComponent->SetAnimationMode(EAnimationMode::AnimationSingleNode);

		if (PrototypeMesh)
		{
			PrototypeMesh->SetVisibility(false);
			PrototypeMesh->SetHiddenInGame(true);
		}
	}

	if (StateMarkerMesh)
	{
		StateMarkerMesh->SetRelativeScale3D(FVector(StateMarkerScale, StateMarkerScale, 0.025f));
		StateMarkerMesh->SetVisibility(bShowStateMarker);
		StateMarkerMesh->SetHiddenInGame(!bShowStateMarker);

		if (!StateMarkerMaterial)
		{
			StateMarkerMaterial = StateMarkerMesh->CreateAndSetMaterialInstanceDynamic(0);
		}
	}
}

void AZDBasicEnemy::PlayLoopAnimation(UAnimSequence* Animation)
{
	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (!Animation || !MeshComponent || ActiveAnimation == Animation)
	{
		return;
	}

	MeshComponent->PlayAnimation(Animation, true);
	MeshComponent->SetPlayRate(1.0f);
	ActiveAnimation = Animation;
}

void AZDBasicEnemy::HoldPoseAnimation(UAnimSequence* Animation, float PoseTime)
{
	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (!Animation || !MeshComponent)
	{
		return;
	}

	if (ActiveAnimation != Animation)
	{
		MeshComponent->PlayAnimation(Animation, false);
		ActiveAnimation = Animation;
	}

	MeshComponent->SetPlayRate(0.0f);
	MeshComponent->SetPosition(FMath::Clamp(PoseTime, 0.0f, Animation->GetPlayLength()), false);
}

void AZDBasicEnemy::PlayOneShotAnimation(UAnimSequence* Animation)
{
	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (!Animation || !MeshComponent)
	{
		return;
	}

	MeshComponent->PlayAnimation(Animation, false);
	MeshComponent->SetPlayRate(1.0f);
	ActiveAnimation = Animation;
}

void AZDBasicEnemy::PlayCurrentLoopAnimation()
{
	if (EnemyState == EZDEnemyState::Windup && WindupAnimation)
	{
		HoldPoseAnimation(WindupAnimation, WindupPoseTime);
		return;
	}

	if (EnemyState == EZDEnemyState::Chasing && ChaseAnimation)
	{
		PlayLoopAnimation(ChaseAnimation);
		return;
	}

	HoldPoseAnimation(IdleAnimation, IdlePoseTime);
}

void AZDBasicEnemy::UpdateStateMarker()
{
	if (!StateMarkerMesh)
	{
		return;
	}

	switch (EnemyState)
	{
	case EZDEnemyState::Idle:
		SetStateMarkerColor(FLinearColor(0.18f, 0.20f, 0.22f, 1.0f), bShowStateMarker);
		break;
	case EZDEnemyState::Chasing:
		SetStateMarkerColor(FLinearColor(0.1f, 0.35f, 1.0f, 1.0f), bShowStateMarker);
		break;
	case EZDEnemyState::Windup:
		SetStateMarkerColor(FLinearColor(1.0f, 0.82f, 0.08f, 1.0f), bShowStateMarker);
		break;
	case EZDEnemyState::Attacking:
		SetStateMarkerColor(FLinearColor(1.0f, 0.08f, 0.02f, 1.0f), bShowStateMarker);
		break;
	case EZDEnemyState::Hurt:
		SetStateMarkerColor(FLinearColor(1.0f, 0.18f, 0.12f, 1.0f), bShowStateMarker);
		break;
	case EZDEnemyState::Dead:
		SetStateMarkerColor(FLinearColor(0.25f, 0.0f, 0.0f, 1.0f), bShowStateMarker);
		break;
	default:
		SetStateMarkerColor(FLinearColor::White, bShowStateMarker);
		break;
	}
}

void AZDBasicEnemy::SetStateMarkerColor(const FLinearColor& MarkerColor, bool bVisible)
{
	if (!StateMarkerMesh)
	{
		return;
	}

	StateMarkerMesh->SetVisibility(bVisible);
	StateMarkerMesh->SetHiddenInGame(!bVisible);

	if (!StateMarkerMaterial)
	{
		StateMarkerMaterial = StateMarkerMesh->CreateAndSetMaterialInstanceDynamic(0);
	}

	if (StateMarkerMaterial)
	{
		StateMarkerMaterial->SetVectorParameterValue(TEXT("Color"), MarkerColor);
		StateMarkerMaterial->SetVectorParameterValue(TEXT("BaseColor"), MarkerColor);
	}
}
