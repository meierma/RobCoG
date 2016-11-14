// Fill out your copyright notice in the Description page of Project Settings.

#include "RobCoG.h"
#include "MotionControllerComponent.h"
#include "RMCBaseCharacter.h"


// Sets default values
ARMCBaseCharacter::ARMCBaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Visualize motion controller debug arrows
	bVisualizeMCDebugArrows = true;

	// Set this pawn to be controlled by the lowest-numbered player
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	// Make the capsule thin
	GetCapsuleComponent()->SetCapsuleRadius(1);
	// Set the default height of the capsule
	GetCapsuleComponent()->SetCapsuleHalfHeight(80);

	// Create a CameraComponent
	CharacterCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("CharacterCamera"));
	// Set capsule component as parent for the camera
	CharacterCamera->SetupAttachment(GetRootComponent());
	// Position the camera
	CharacterCamera->RelativeLocation = FVector(10.0f, 0.0f, BaseEyeHeight);
	// Allow the pawn to control the camera rotation
	CharacterCamera->bUsePawnControlRotation = true;

	// Create the MC origin component
	MCOriginComponent = CreateDefaultSubobject<USceneComponent>(TEXT("MCOriginComponent"));
	// Attach Offset to root
	MCOriginComponent->SetupAttachment(GetRootComponent());
	// Position of the MC origin
	MCOriginComponent->RelativeLocation = FVector(
		0.0f, 0.0f, -GetCapsuleComponent()->GetScaledCapsuleHalfHeight());

	// Create left/right motion controller
	LeftMC = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftMC"));
	RightMC = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightMC"));
	// Set the mapped hand (from the Motion Controller)
	LeftMC->Hand = EControllerHand::Left;
	RightMC->Hand = EControllerHand::Right;
	// Attach controllers to root component
	LeftMC->SetupAttachment(MCOriginComponent);
	RightMC->SetupAttachment(MCOriginComponent);

	// Create left/right target vis arrows
	LeftMCDebugArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("LeftMCArrow"));
	RightMCDebugArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("RightMCArrow"));
	// Set arrow sizes
	LeftMCDebugArrow->ArrowSize = 0.1;
	RightMCDebugArrow->ArrowSize = 0.1;
	// Attach vis arrow to motion controllers
	LeftMCDebugArrow->SetupAttachment(LeftMC);
	RightMCDebugArrow->SetupAttachment(RightMC);
}

// Called when the game starts or when spawned
void ARMCBaseCharacter::BeginPlay()
{
	Super::BeginPlay();	
	
	// Visualize MC debug arrows
	if (bVisualizeMCDebugArrows)
	{
		LeftMCDebugArrow->SetHiddenInGame(false);
		RightMCDebugArrow->SetHiddenInGame(false);
	}
}

// Called to bind functionality to input
void ARMCBaseCharacter::SetupPlayerInputComponent(class UInputComponent* InputComp)
{
	Super::SetupPlayerInputComponent(InputComp);

}

// Get the Motion Controller Component
UMotionControllerComponent* ARMCBaseCharacter::GetMotionControllerComponent(EControllerHand HandType)
{
	if (HandType == EControllerHand::Left)
	{
		return LeftMC;
	}
	else if (HandType == EControllerHand::Right)
	{
		return RightMC;
	}
	else
	{
		return nullptr;
	}
}

