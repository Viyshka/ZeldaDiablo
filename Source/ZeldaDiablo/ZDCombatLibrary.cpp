#include "ZDCombatLibrary.h"

bool UZDCombatLibrary::IsTargetInCone(const FVector& Origin, const FVector& Forward, const FVector& TargetLocation, float Radius, float AngleDegrees)
{
	FVector ToTarget = TargetLocation - Origin;
	ToTarget.Z = 0.0f;

	if (Radius > 0.0f && ToTarget.SizeSquared() > FMath::Square(Radius))
	{
		return false;
	}

	FVector FlatForward = FlattenDirection(Forward);
	if (FlatForward.IsNearlyZero())
	{
		FlatForward = FVector::ForwardVector;
	}

	if (ToTarget.IsNearlyZero())
	{
		return true;
	}

	const float HalfAngleRadians = FMath::DegreesToRadians(FMath::Clamp(AngleDegrees, 0.0f, 360.0f) * 0.5f);
	const float MinimumDot = FMath::Cos(HalfAngleRadians);
	return FVector::DotProduct(FlatForward, ToTarget.GetSafeNormal()) >= MinimumDot;
}

FVector UZDCombatLibrary::FlattenDirection(const FVector& Direction)
{
	FVector Result = Direction;
	Result.Z = 0.0f;
	return Result.GetSafeNormal();
}

