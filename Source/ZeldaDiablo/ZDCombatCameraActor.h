#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZDCombatCameraActor.generated.h"

class UCameraComponent;
class USceneComponent;

UCLASS()
class ZELDADIABLO_API AZDCombatCameraActor : public AActor
{
	GENERATED_BODY()

public:
	AZDCombatCameraActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* SceneRoot = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* Camera = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	FVector TargetPoint = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (ClampMin = "100.0"))
	float CameraHeight = 1700.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (ClampMin = "0.0"))
	float DistanceToTarget = 650.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (ClampMin = "5.0", ClampMax = "170.0"))
	float FieldOfView = 55.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	bool bSetAsViewTargetOnBeginPlay = true;

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void UpdateCameraTransform();

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
};

