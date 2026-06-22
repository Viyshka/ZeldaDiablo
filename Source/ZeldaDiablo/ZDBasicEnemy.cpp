#include "ZDBasicEnemy.h"

#include "Engine/StaticMesh.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
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

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->MaxWalkSpeed = MovementSpeed;

	HealthComponent = CreateDefaultSubobject<UZDHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->MaxHealth = 1.0f;

	PrototypeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PrototypeMesh"));
	PrototypeMesh->SetupAttachment(RootComponent);
	PrototypeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PrototypeMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -8.0f));
	PrototypeMesh->SetRelativeScale3D(FVector(0.65f, 0.65f, 1.45f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		PrototypeMesh->SetStaticMesh(CubeMesh.Object);
	}
}

void AZDBasicEnemy::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->MaxWalkSpeed = MovementSpeed;
	AcquirePlayer();

	if (HealthComponent)
	{
		HealthComponent->OnDeath.AddDynamic(this, &AZDBasicEnemy::HandleDeath);
	}
}

void AZDBasicEnemy::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

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
	}
}

void AZDBasicEnemy::UpdateChase(float DeltaSeconds)
{
	if (!TargetPlayer)
	{
		return;
	}

	FacePlayer(DeltaSeconds);

	if (!bAttackOnCooldown && IsPlayerInRange(AttackRadius))
	{
		StartWindup();
		return;
	}

	FVector Direction = TargetPlayer->GetActorLocation() - GetActorLocation();
	Direction.Z = 0.0f;

	if (!Direction.IsNearlyZero())
	{
		AddMovementInput(Direction.GetSafeNormal());
	}
}

void AZDBasicEnemy::StartWindup()
{
	EnemyState = EZDEnemyState::Windup;
	GetWorldTimerManager().SetTimer(WindupTimerHandle, this, &AZDBasicEnemy::FinishWindup, WindupDuration, false);
}

void AZDBasicEnemy::FinishWindup()
{
	if (!IsAlive())
	{
		return;
	}

	EnemyState = EZDEnemyState::Attacking;
	ExecuteAttack();
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
	GetWorldTimerManager().SetTimer(HurtTimerHandle, this, &AZDBasicEnemy::FinishHurtState, HurtDuration, false);
}

void AZDBasicEnemy::FinishHurtState()
{
	if (!IsAlive())
	{
		return;
	}

	EnemyState = bHasDetectedPlayer ? EZDEnemyState::Chasing : EZDEnemyState::Idle;
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

bool AZDBasicEnemy::IsPlayerInRange(float Range) const
{
	return TargetPlayer && FVector::DistSquared2D(GetActorLocation(), TargetPlayer->GetActorLocation()) <= FMath::Square(Range);
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
	GetWorldTimerManager().ClearTimer(HurtTimerHandle);
}

void AZDBasicEnemy::HandleDeath(UZDHealthComponent* DeadHealthComponent)
{
	EnemyState = EZDEnemyState::Dead;
	GetWorldTimerManager().ClearAllTimersForObject(this);
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetLifeSpan(6.0f);
}

