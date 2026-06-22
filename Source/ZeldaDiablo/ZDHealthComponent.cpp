#include "ZDHealthComponent.h"

UZDHealthComponent::UZDHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UZDHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	ResetHealth();
}

void UZDHealthComponent::ResetHealth()
{
	CurrentHealth = MaxHealth;
	OnHealthChanged.Broadcast(this, CurrentHealth);
}

bool UZDHealthComponent::ApplyDamage(float DamageAmount)
{
	if (DamageAmount <= 0.0f || IsDead())
	{
		return false;
	}

	CurrentHealth = FMath::Max(0.0f, CurrentHealth - DamageAmount);
	OnHealthChanged.Broadcast(this, CurrentHealth);

	if (IsDead())
	{
		OnDeath.Broadcast(this);
	}

	return true;
}

bool UZDHealthComponent::IsDead() const
{
	return CurrentHealth <= 0.0f;
}

