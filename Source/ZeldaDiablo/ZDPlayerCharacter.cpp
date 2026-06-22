#include "ZDPlayerCharacter.h"

#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
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
}

void AZDPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HealthComponent)
	{
		HealthComponent->OnDeath.AddDynamic(this, &AZDPlayerCharacter::HandleDeath);
	}
}

void AZDPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const float DesiredSpeed = bIsBlocking ? MovementSpeed * BlockingMovementMultiplier : MovementSpeed;
	GetCharacterMovement()->MaxWalkSpeed = DesiredSpeed;

	if (CanMove() && !PendingMovementInput.IsNearlyZero())
	{
		FVector MoveDirection = PendingMovementInput;
		MoveDirection.Z = 0.0f;
		MoveDirection.Normalize();

		AddMovementInput(MoveDirection);
		LastFacingDirection = MoveDirection;
		SetActorRotation(MoveDirection.Rotation());
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
			CombatState = PendingMovementInput.IsNearlyZero() ? EZDPlayerCombatState::Idle : EZDPlayerCombatState::Moving;
		}
	}
}

void AZDPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AZDPlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AZDPlayerCharacter::MoveRight);
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
	}
}

bool AZDPlayerCharacter::CanMove() const
{
	if (!IsAlive()
		|| CombatState == EZDPlayerCombatState::Attacking
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
	}
}

void AZDPlayerCharacter::StartBasicAttack()
{
	bIsBlocking = false;
	CombatState = EZDPlayerCombatState::Attacking;

	ApplyBasicAttackDamage();

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
	GetWorldTimerManager().ClearTimer(AttackTimerHandle);
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
}

void AZDPlayerCharacter::HandleDeath(UZDHealthComponent* DeadHealthComponent)
{
	bIsBlocking = false;
	bBlockInputHeld = false;
	CombatState = EZDPlayerCombatState::Dead;
	GetWorldTimerManager().ClearAllTimersForObject(this);
	GetCharacterMovement()->DisableMovement();
}

