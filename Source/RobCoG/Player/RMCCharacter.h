// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "RMCBaseCharacter.h"
#include "RMCCharacter.generated.h"

UCLASS()
class ROBCOG_API ARMCCharacter : public ARMCBaseCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ARMCCharacter();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

protected:
	// Handles moving forward/backward
	void MoveForward(const float Val);

	// Handles strafing Left/Right
	void MoveRight(const float Val);
};
