// Fill out your copyright notice in the Description page of Project Settings.

#include "RobCoG.h"
#include "HeadMountedDisplay.h"
#include "MotionControllerComponent.h"
#include "RMCCharacterVR.h"


// Sets default values
ARMCCharacterVR::ARMCCharacterVR()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Positional head tracking
	bPositionalHeadTracking = true;

	// Attach camera to the MC origin
	CharacterCamera->SetupAttachment(MCOriginComponent);
	// Disable the pawn to control the camera rotation
	CharacterCamera->bUsePawnControlRotation = false;
}

// Called when the game starts or when spawned
void ARMCCharacterVR::BeginPlay()
{
	Super::BeginPlay();

	IHeadMountedDisplay* HMD = (IHeadMountedDisplay*)(GEngine->HMDDevice.Get());
	if (HMD && HMD->IsStereoEnabled())
	{
		// Disable/Enable positional movement to pin camera translation
		HMD->EnablePositionalTracking(bPositionalHeadTracking);

		// Remove any translation when disabling positional head tracking
		if (!bPositionalHeadTracking)
		{
			CharacterCamera->SetRelativeLocation(FVector(0.0f));
		}
	}	
}

// Called to bind functionality to input
void ARMCCharacterVR::SetupPlayerInputComponent(class UInputComponent* InputComp)
{
	Super::SetupPlayerInputComponent(InputComp);
}