#include "ZDCombatCameraActor.h"

#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"

AZDCombatCameraActor::AZDCombatCameraActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SceneRoot);
	Camera->SetFieldOfView(FieldOfView);
}

void AZDCombatCameraActor::BeginPlay()
{
	Super::BeginPlay();

	UpdateCameraTransform();

	if (bSetAsViewTargetOnBeginPlay)
	{
		if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
		{
			PlayerController->SetViewTarget(this);
		}
	}
}

void AZDCombatCameraActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	UpdateCameraTransform();
}

void AZDCombatCameraActor::UpdateCameraTransform()
{
	const FVector CameraLocation = TargetPoint + FVector(-DistanceToTarget, 0.0f, CameraHeight);
	SetActorLocation(CameraLocation);
	SetActorRotation((TargetPoint - CameraLocation).Rotation());

	if (Camera)
	{
		Camera->SetFieldOfView(FieldOfView);
	}
}

