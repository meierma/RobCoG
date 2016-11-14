// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "RMCBaseCharacter.generated.h"

UCLASS()
class ROBCOG_API ARMCBaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ARMCBaseCharacter();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	// Get the Motion Controller Component
	class UMotionControllerComponent* GetMotionControllerComponent(EControllerHand HandType);

	// Create visualisation target arrows
	UPROPERTY(EditAnywhere, Category = "Robcog")
	bool bVisualizeMCDebugArrows;

protected:
	// Character camera
	UCameraComponent* CharacterCamera;

	// Motion controller origin
	USceneComponent* MCOriginComponent;

	// Left hand motion controller
	class UMotionControllerComponent* LeftMC;

	// Right hand motion controller
	class UMotionControllerComponent* RightMC;

	// Left target arrow visual
	UArrowComponent* LeftMCDebugArrow;

	// Right target arrow visual
	UArrowComponent* RightMCDebugArrow;
};
