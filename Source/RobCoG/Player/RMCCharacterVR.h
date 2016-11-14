// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "RMCBaseCharacter.h"
#include "RMCCharacterVR.generated.h"

UCLASS()
class ROBCOG_API ARMCCharacterVR : public ARMCBaseCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ARMCCharacterVR();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	// Enable HMD tracking
	UPROPERTY(EditAnywhere, Category = "Robcog")
	bool bPositionalHeadTracking;
};
