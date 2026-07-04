#include "ZDPlayerCharacter.h"

#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimSequence.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"
#include "ZDCombatLibrary.h"
#include "ZDHealthComponent.h"
#include "ZDBasicEnemy.h"

AZDPlayerCharacter::AZDPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(35.0f, 88.0f);
	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = MovementSpeed;

	HealthComponent = CreateDefaultSubobject<UZDHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->MaxHealth = 3.0f;

	PrototypeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PrototypeMesh"));
	PrototypeMesh->SetupAttachment(RootComponent);
	PrototypeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PrototypeMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -5.0f));
	PrototypeMesh->SetRelativeScale3D(FVector(0.7f, 0.7f, 1.6f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		PrototypeMesh->SetStaticMesh(CubeMesh.Object);
	}

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MannyMesh(TEXT("/Game/FreeAnimsMixPack/Demo/Mannequins/Meshes/SKM_Manny.SKM_Manny"));
	if (MannyMesh.Succeeded())
	{
		CharacterMeshAsset = MannyMesh.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequence> PracticeFencingAnim(TEXT("/Game/FreeAnimsMixPack/Animation/AS_PracticeFencing.AS_PracticeFencing"));
	if (PracticeFencingAnim.Succeeded())
	{
		IdleAnimation = PracticeFencingAnim.Object;
		BlockAnimation = PracticeFencingAnim.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequence> SwingSwordAnim(TEXT("/Game/FreeAnimsMixPack/Animation/AS_SwingSword.AS_SwingSword"));
	if (SwingSwordAnim.Succeeded())
	{
		BasicAttackAnimation = SwingSwordAnim.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequence> ComboAnim(TEXT("/Game/FreeAnimsMixPack/Animation/AS_Combo.AS_Combo"));
	if (ComboAnim.Succeeded())
	{
		CounterAttackAnimation = ComboAnim.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequence> DyingAnim(TEXT("/Game/FreeAnimsMixPack/Animation/AS_DyingFromWounds.AS_DyingFromWounds"));
	if (DyingAnim.Succeeded())
	{
		DeathAnimation = DyingAnim.Object;
	}

	ConfigureVisuals();
}

void AZDPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HealthComponent)
	{
		HealthComponent->OnDeath.AddDynamic(this, &AZDPlayerCharacter::HandleDeath);
	}

	ConfigureVisuals();
	PlayCurrentLoopAnimation();
}

void AZDPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const float DesiredSpeed = bIsBlocking ? MovementSpeed * BlockingMovementMultiplier : MovementSpeed;
	GetCharacterMovement()->MaxWalkSpeed = DesiredSpeed;

	const bool bHasMovementInput = HasDirectionalInput(PendingMovementInput, MovementInputDeadZone);

	if (CanUpdateFacing())
	{
		UpdateDesiredFacingFromInput(PendingLookInput);
		ApplyFacingRotation(DeltaSeconds);
	}

	if (CanMove() && bHasMovementInput)
	{
		FVector MoveDirection = PendingMovementInput;
		MoveDirection.Z = 0.0f;
		const float MoveScale = FMath::Clamp(MoveDirection.Size2D(), 0.0f, 1.0f);
		MoveDirection.Normalize();

		AddMovementInput(MoveDirection, MoveScale);
	}

	if (CombatState != EZDPlayerCombatState::Attacking
		&& CombatState != EZDPlayerCombatState::CounterAttacking
		&& CombatState != EZDPlayerCombatState::Hurt
		&& CombatState != EZDPlayerCombatState::Dead)
	{
		if (bIsBlocking)
		{
			CombatState = EZDPlayerCombatState::Blocking;
		}
		else
		{
			CombatState = bHasMovementInput ? EZDPlayerCombatState::Moving : EZDPlayerCombatState::Idle;
		}

		PlayCurrentLoopAnimation();
	}
}

void AZDPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AZDPlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AZDPlayerCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("LookForward"), this, &AZDPlayerCharacter::LookForward);
	PlayerInputComponent->BindAxis(TEXT("LookRight"), this, &AZDPlayerCharacter::LookRight);
	PlayerInputComponent->BindAction(TEXT("BasicAttack"), IE_Pressed, this, &AZDPlayerCharacter::HandleAttackPressed);
	PlayerInputComponent->BindAction(TEXT("BasicAttack"), IE_Released, this, &AZDPlayerCharacter::HandleAttackReleased);
	PlayerInputComponent->BindAction(TEXT("Block"), IE_Pressed, this, &AZDPlayerCharacter::HandleBlockPressed);
	PlayerInputComponent->BindAction(TEXT("Block"), IE_Released, this, &AZDPlayerCharacter::HandleBlockReleased);
}

void AZDPlayerCharacter::MoveForward(float Value)
{
	PendingMovementInput.X = Value;
}

void AZDPlayerCharacter::MoveRight(float Value)
{
	PendingMovementInput.Y = Value;
}

void AZDPlayerCharacter::LookForward(float Value)
{
	PendingLookInput.X = Value;
}

void AZDPlayerCharacter::LookRight(float Value)
{
	PendingLookInput.Y = Value;
}

void AZDPlayerCharacter::HandleAttackPressed()
{
	bWantsAttackChain = true;

	if (!CanStartCombatAction())
	{
		return;
	}

	if (bBlockInputHeld && TryStartCounterAttack())
	{
		return;
	}

	StartBasicAttack();
}

void AZDPlayerCharacter::HandleAttackReleased()
{
	bWantsAttackChain = false;
}

void AZDPlayerCharacter::HandleBlockPressed()
{
	bBlockInputHeld = true;
	RefreshBlockState();
}

void AZDPlayerCharacter::HandleBlockReleased()
{
	bBlockInputHeld = false;
	bIsBlocking = false;

	if (CombatState == EZDPlayerCombatState::Blocking)
	{
		CombatState = EZDPlayerCombatState::Idle;
		PlayCurrentLoopAnimation();
	}
}

bool AZDPlayerCharacter::CanMove() const
{
	if (!IsAlive()
		|| CombatState == EZDPlayerCombatState::CounterAttacking
		|| CombatState == EZDPlayerCombatState::Hurt)
	{
		return false;
	}

	return !bIsBlocking || bCanMoveWhileBlocking;
}

bool AZDPlayerCharacter::CanStartCombatAction() const
{
	return IsAlive()
		&& CombatState != EZDPlayerCombatState::Attacking
		&& CombatState != EZDPlayerCombatState::CounterAttacking
		&& CombatState != EZDPlayerCombatState::Hurt;
}

bool AZDPlayerCharacter::CanBlock() const
{
	return IsAlive()
		&& CombatState != EZDPlayerCombatState::Attacking
		&& CombatState != EZDPlayerCombatState::CounterAttacking
		&& CombatState != EZDPlayerCombatState::Hurt;
}

void AZDPlayerCharacter::RefreshBlockState()
{
	bIsBlocking = bBlockInputHeld && CanBlock();

	if (bIsBlocking)
	{
		CombatState = EZDPlayerCombatState::Blocking;
		PlayCurrentLoopAnimation();
	}
}

void AZDPlayerCharacter::StartBasicAttack()
{
	bIsBlocking = false;
	CombatState = EZDPlayerCombatState::Attacking;
	PlayOneShotAnimation(BasicAttackAnimation);

	GetWorldTimerManager().ClearTimer(AttackHitTimerHandle);
	const float HitDelay = FMath::Min(BasicAttackHitDelay, BasicAttackDuration);
	if (HitDelay <= 0.0f)
	{
		ApplyBasicAttackDamage();
	}
	else
	{
		GetWorldTimerManager().SetTimer(AttackHitTimerHandle, this, &AZDPlayerCharacter::ApplyBasicAttackDamage, HitDelay, false);
	}

	const float TotalAttackTime = BasicAttackDuration + BasicAttackRecovery;
	GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &AZDPlayerCharacter::FinishBasicAttack, TotalAttackTime, false);
}

void AZDPlayerCharacter::FinishBasicAttack()
{
	if (!IsAlive())
	{
		return;
	}

	CombatState = EZDPlayerCombatState::Idle;
	RefreshBlockState();
	PlayCurrentLoopAnimation();

	if (bHoldAttackChains && bWantsAttackChain && CanStartCombatAction())
	{
		if (bBlockInputHeld && TryStartCounterAttack())
		{
			return;
		}

		StartBasicAttack();
	}
}

void AZDPlayerCharacter::ApplyBasicAttackDamage()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<AZDBasicEnemy> EnemyIterator(World); EnemyIterator; ++EnemyIterator)
	{
		AZDBasicEnemy* Enemy = *EnemyIterator;
		if (!Enemy || !Enemy->IsAlive())
		{
			continue;
		}

		if (UZDCombatLibrary::IsTargetInCone(GetActorLocation(), LastFacingDirection, Enemy->GetActorLocation(), BasicAttackRadius, BasicAttackAngle))
		{
			Enemy->ReceivePlayerHit(BasicAttackDamage, this);
		}
	}
}

bool AZDPlayerCharacter::TryStartCounterAttack()
{
	AZDBasicEnemy* TargetEnemy = FindCounterTarget();
	if (!TargetEnemy)
	{
		return false;
	}

	bIsBlocking = false;
	CombatState = EZDPlayerCombatState::CounterAttacking;
	PlayOneShotAnimation(CounterAttackAnimation);

	TargetEnemy->KillByCounterAttack(this);
	GetWorldTimerManager().SetTimer(CounterTimerHandle, this, &AZDPlayerCharacter::FinishCounterAttack, CounterLockDuration, false);
	return true;
}

void AZDPlayerCharacter::FinishCounterAttack()
{
	if (!IsAlive())
	{
		return;
	}

	CombatState = EZDPlayerCombatState::Idle;
	RefreshBlockState();
	PlayCurrentLoopAnimation();
}

AZDBasicEnemy* AZDPlayerCharacter::FindCounterTarget() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	AZDBasicEnemy* BestTarget = nullptr;
	float BestDistanceSquared = TNumericLimits<float>::Max();

	for (TActorIterator<AZDBasicEnemy> EnemyIterator(World); EnemyIterator; ++EnemyIterator)
	{
		AZDBasicEnemy* Enemy = *EnemyIterator;
		if (!Enemy || !Enemy->IsCounterable())
		{
			continue;
		}

		if (!UZDCombatLibrary::IsTargetInCone(GetActorLocation(), LastFacingDirection, Enemy->GetActorLocation(), CounterRadius, CounterAngle))
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared2D(GetActorLocation(), Enemy->GetActorLocation());
		if (DistanceSquared < BestDistanceSquared)
		{
			BestDistanceSquared = DistanceSquared;
			BestTarget = Enemy;
		}
	}

	return BestTarget;
}

bool AZDPlayerCharacter::ReceiveEnemyAttack(AActor* DamageCauser, float DamageAmount)
{
	if (!IsAlive())
	{
		return false;
	}

	if (CombatState == EZDPlayerCombatState::CounterAttacking)
	{
		return true;
	}

	if (DamageCauser && IsDamageBlockedFrom(DamageCauser->GetActorLocation()))
	{
		return true;
	}

	if (HealthComponent && HealthComponent->ApplyDamage(DamageAmount) && IsAlive())
	{
		EnterHurtState();
	}

	return false;
}

bool AZDPlayerCharacter::IsAlive() const
{
	return HealthComponent && !HealthComponent->IsDead();
}

bool AZDPlayerCharacter::IsDamageBlockedFrom(const FVector& DamageSourceLocation) const
{
	return bIsBlocking
		&& UZDCombatLibrary::IsTargetInCone(GetActorLocation(), LastFacingDirection, DamageSourceLocation, 0.0f, BlockAngle);
}

void AZDPlayerCharacter::EnterHurtState()
{
	bIsBlocking = false;
	CombatState = EZDPlayerCombatState::Hurt;
	PlayOneShotAnimation(HurtAnimation);
	GetWorldTimerManager().ClearTimer(AttackTimerHandle);
	GetWorldTimerManager().ClearTimer(AttackHitTimerHandle);
	GetWorldTimerManager().ClearTimer(CounterTimerHandle);
	GetWorldTimerManager().SetTimer(HurtTimerHandle, this, &AZDPlayerCharacter::FinishHurtState, HurtDuration, false);
}

void AZDPlayerCharacter::FinishHurtState()
{
	if (!IsAlive())
	{
		return;
	}

	CombatState = EZDPlayerCombatState::Idle;
	RefreshBlockState();
	PlayCurrentLoopAnimation();
}

void AZDPlayerCharacter::HandleDeath(UZDHealthComponent* DeadHealthComponent)
{
	bIsBlocking = false;
	bBlockInputHeld = false;
	CombatState = EZDPlayerCombatState::Dead;
	PlayOneShotAnimation(DeathAnimation);
	GetWorldTimerManager().ClearAllTimersForObject(this);
	GetCharacterMovement()->DisableMovement();
}

void AZDPlayerCharacter::ConfigureVisuals()
{
	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (!MeshComponent)
	{
		return;
	}

	if (CharacterMeshAsset)
	{
		MeshComponent->SetSkeletalMesh(CharacterMeshAsset);
		MeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
		MeshComponent->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		MeshComponent->SetAnimationMode(EAnimationMode::AnimationSingleNode);

		if (PrototypeMesh)
		{
			PrototypeMesh->SetVisibility(false);
			PrototypeMesh->SetHiddenInGame(true);
		}
	}
}

bool AZDPlayerCharacter::HasDirectionalInput(const FVector& Input, float DeadZone) const
{
	return Input.SizeSquared2D() > FMath::Square(DeadZone);
}

bool AZDPlayerCharacter::CanUpdateFacing() const
{
	return IsAlive()
		&& CombatState != EZDPlayerCombatState::CounterAttacking
		&& CombatState != EZDPlayerCombatState::Hurt;
}

void AZDPlayerCharacter::UpdateDesiredFacingFromInput(const FVector& Input)
{
	if (!HasDirectionalInput(Input, LookInputDeadZone))
	{
		return;
	}

	FVector FacingDirection = Input;
	FacingDirection.Z = 0.0f;
	FacingDirection.Normalize();

	DesiredFacingDirection = FacingDirection;
}

void AZDPlayerCharacter::ApplyFacingRotation(float DeltaSeconds)
{
	if (DesiredFacingDirection.IsNearlyZero())
	{
		return;
	}

	const FRotator CurrentRotation = GetActorRotation();
	const FRotator DesiredRotation = DesiredFacingDirection.Rotation();
	const FRotator NewRotation = FacingRotationSpeed <= 0.0f
		? DesiredRotation
		: FMath::RInterpConstantTo(CurrentRotation, DesiredRotation, DeltaSeconds, FacingRotationSpeed);

	SetActorRotation(NewRotation);
	LastFacingDirection = NewRotation.Vector().GetSafeNormal2D();
}

void AZDPlayerCharacter::PlayLoopAnimation(UAnimSequence* Animation)
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

void AZDPlayerCharacter::HoldPoseAnimation(UAnimSequence* Animation, float PoseTime)
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

void AZDPlayerCharacter::PlayOneShotAnimation(UAnimSequence* Animation)
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

void AZDPlayerCharacter::PlayCurrentLoopAnimation()
{
	if (CombatState == EZDPlayerCombatState::Blocking && BlockAnimation)
	{
		HoldPoseAnimation(BlockAnimation, BlockPoseTime);
		return;
	}

	if (CombatState == EZDPlayerCombatState::Moving && MovementAnimation)
	{
		PlayLoopAnimation(MovementAnimation);
		return;
	}

	HoldPoseAnimation(IdleAnimation, IdlePoseTime);
}
