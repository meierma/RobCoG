// Fill out your copyright notice in the Description page of Project Settings.

#include "RobCoG.h"
#include "HeadMountedDisplay.h"
#include "MotionControllerComponent.h"
#include "RMCCharacter.h"


// Sets default values
ARMCCharacter::ARMCCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Visualize motion controller debug arrows
	bVisTargetArrows = true;
	// Positional head tracking
	bPositionalHeadTracking = false;
	// Make the capsule thin
	GetCapsuleComponent()->SetCapsuleRadius(1);
	// Set the default height of the capsule
	GetCapsuleComponent()->SetCapsuleHalfHeight(80);
	// Set this pawn to be controlled by the lowest-numbered player
	AutoPossessPlayer = EAutoReceiveInput::Player0;
	// Get root component
	RootComponent = GetRootComponent();

	// Create the MC origin component
	MCOriginComponent = CreateDefaultSubobject<USceneComponent>(TEXT("MCOriginComponent"));
	// Attach Offset to root
	MCOriginComponent->SetupAttachment(RootComponent);
	// Position of the MC origin
	MCOriginComponent->RelativeLocation = FVector(
		0.0f, 0.0f, -GetCapsuleComponent()->GetScaledCapsuleHalfHeight());

	// Create a CameraComponent, attach to the VR origin component
	CharacterCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("CharacterCamera"));
	// TODO might needed, check with MC
	//CharacterCamera->SetupAttachment(GetCapsuleComponent()); // no VR
	CharacterCamera->SetupAttachment(MCOriginComponent); //VR

	// Position the camera
	CharacterCamera->RelativeLocation = FVector(0.0f, 0.0f, BaseEyeHeight);
	// Allow the pawn to control the camera rotation
	CharacterCamera->bUsePawnControlRotation = true;
	
	// Create left/right motion controller
	LeftMC = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftMotionController"));
	RightMC = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightMotionController"));
	// Set the mapped hand (from the Motion Controller)
	LeftMC->Hand = EControllerHand::Left;
	RightMC->Hand = EControllerHand::Right;
	// Attach controllers to root component
	LeftMC->SetupAttachment(MCOriginComponent);
	RightMC->SetupAttachment(MCOriginComponent);

	// TODO create the arrows on begin play if needed
	// Create left/right target vis arrows
	LeftTargetArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("LeftVisArrow"));
	RightTargetArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("RightVisArrow"));
	// Set arrow sizes
	LeftTargetArrow->ArrowSize = 0.1;
	RightTargetArrow->ArrowSize = 0.1;
	// Attach vis arrow to motion controllers
	LeftTargetArrow->SetupAttachment(LeftMC);
	RightTargetArrow->SetupAttachment(RightMC);
}

// Called when the game starts or when spawned
void ARMCCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Create visualisation arrows
	if (bVisTargetArrows)
	{
		// Set arrows to visible
		LeftTargetArrow->SetHiddenInGame(false);
		RightTargetArrow->SetHiddenInGame(false);
	}

	// Check if game is started in VR mode
	IHeadMountedDisplay* HMD = (IHeadMountedDisplay*)(GEngine->HMDDevice.Get());
	if (HMD && HMD->IsStereoEnabled())
	{
		// TODO might needed, check with MC
		// Attach camera to the MC origin
		//CharacterCamera->AttachToComponent(MCOriginComponent,
		//	FAttachmentTransformRules::KeepWorldTransform);

		// Disable/Enable positional movement to pin camera translation
		HMD->EnablePositionalTracking(bPositionalHeadTracking);

		// Remove any translation when disabling positional head tracking
		if (!bPositionalHeadTracking)
		{
			CharacterCamera->SetRelativeLocation(FVector(0.0f));
		}
	}
	else
	{
		// TODO might needed, check with MC
		// Attach camera to the MC origin
		//CharacterCamera->AttachToComponent(GetCapsuleComponent(),
		//	FAttachmentTransformRules::KeepWorldTransform);

		//// Position the camera
		//CharacterCamera->RelativeLocation = FVector(0.0f, 0.0f, BaseEyeHeight);
		//// Allow the pawn to control the camera rotation
		//CharacterCamera->bUsePawnControlRotation = true;
	}	
}

// Called to bind functionality to input
void ARMCCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

	// Enable inputs if VR not enabled, or enabled but no head tracking
	IHeadMountedDisplay* HMD = (IHeadMountedDisplay*)(GEngine->HMDDevice.Get());
	if (!HMD || !HMD->IsStereoEnabled() || !bPositionalHeadTracking)
	{
		// Set up gameplay key bindings
		InputComponent->BindAxis("MoveForward", this, &ARMCCharacter::MoveForward);
		InputComponent->BindAxis("MoveRight", this, &ARMCCharacter::MoveRight);
		// Default Camera view bindings
		InputComponent->BindAxis("CameraPitch", this, &ARMCCharacter::AddControllerPitchInput);
		InputComponent->BindAxis("CameraYaw", this, &ARMCCharacter::AddControllerYawInput);
	}

}


// Handles moving forward/backward
void ARMCCharacter::MoveForward(const float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// Find out which way is forward
		FRotator Rotation = Controller->GetControlRotation();
		// Limit pitch when walking or falling
		if (GetCharacterMovement()->IsMovingOnGround() || GetCharacterMovement()->IsFalling())
		{
			Rotation.Pitch = 0.0f;
		}
		// add movement in that direction
		const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

// Handles moving right/left
void ARMCCharacter::MoveRight(const float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// Find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

// Get the Motion Controller Component
UMotionControllerComponent* ARMCCharacter::GetMotionControllerComponent(EControllerHand HandType)
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
