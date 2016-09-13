// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "RMCCharacter.generated.h"

UCLASS()
class ROBCOG_API ARMCCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ARMCCharacter();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	// Get the Motion Controller Component
	class UMotionControllerComponent* GetMotionControllerComponent(EControllerHand HandType);

protected:
	// Handles moving forward/backward
	void MoveForward(const float Val);

	// Handles strafing Left/Right
	void MoveRight(const float Val);

	// Create visualisation target arrows
	UPROPERTY(EditAnywhere, Category = "Robcog")
	bool bVisTargetArrows;

	// Enable HMD tracking
	UPROPERTY(EditAnywhere, Category = "Robcog")
	bool bPositionalHeadTracking;

private:
	// Character camera
	UCameraComponent* CharacterCamera;

	// Motion controller origin
	USceneComponent* MCOriginComponent;

	// Left hand motion controller
	class UMotionControllerComponent* LeftMC;

	// Right hand motion controller
	class UMotionControllerComponent* RightMC;

	// Left target arrow visual
	UArrowComponent* LeftTargetArrow;

	// Right target arrow visual
	UArrowComponent* RightTargetArrow;
};
