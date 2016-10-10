// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "RCloseAllDrawers.generated.h"

UCLASS()
class ROBCOG_API ARCloseAllDrawers : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARCloseAllDrawers();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	// Close drawers
	void CloseDrawers();

	// Close furniture timer handle (call after a delay, apply impulse to close drawers)
	FTimerHandle CloseFurnitureTimerHandle;
};
