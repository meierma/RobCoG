// Fill out your copyright notice in the Description page of Project Settings.

#include "RobCoG.h"
#include "MotionControllerComponent.h"
#include "RMCCharacter.h"


// Sets default values
ARMCCharacter::ARMCCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void ARMCCharacter::BeginPlay()
{
	Super::BeginPlay();
}

// Called to bind functionality to input
void ARMCCharacter::SetupPlayerInputComponent(class UInputComponent* InputComp)
{
	Super::SetupPlayerInputComponent(InputComp);

	// Set up gameplay key bindings
	InputComp->BindAxis("MoveForward", this, &ARMCCharacter::MoveForward);
	InputComp->BindAxis("MoveRight", this, &ARMCCharacter::MoveRight);
	// Default Camera view bindings
	InputComp->BindAxis("CameraPitch", this, &ARMCCharacter::AddControllerPitchInput);
	InputComp->BindAxis("CameraYaw", this, &ARMCCharacter::AddControllerYawInput);
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

