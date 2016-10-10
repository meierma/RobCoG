// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "RSetupEnvironment.generated.h"

UCLASS()
class ROBCOG_API ARSetupEnvironment : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARSetupEnvironment();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	// Close the furnitures (drawers/doors)
	void CloseFurnitures();

	// Start logging
	void InitLogging();

	// Start logging
	void StartLogging();

	// Start logging
	void StopLogging();

	// Start logging
	void RestartLogging();

	// Close furniture timer handle (call after a delay, apply impulse to close drawers)
	FTimerHandle CloseFurnitureTimerHandle;

	// Init semantic logger
	FTimerHandle InitLoggingTimerHandle;

	// Start semantic logger
	FTimerHandle StartLoggingTimerHandle;

	//// Close furniture timer handle (call after a delay, apply impulse to close drawers)
	//FTimerHandle StopLoggingTimerHandle;

	//// Close furniture timer handle (call after a delay, apply impulse to close drawers)
	//FTimerHandle RestartLoggingTimerHandle;

	// Pointer to the semantic log manager
	class ASLManager* SemLogManager;
	
};
