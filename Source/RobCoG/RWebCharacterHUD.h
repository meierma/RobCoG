// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/HUD.h"
#include "RWebCharacterHUD.generated.h"

/**
 * 
 */
UCLASS()
class ROBCOG_API ARWebCharacterHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	// Constructor
	ARWebCharacterHUD();

	// Primary draw call for the HUD
	virtual void DrawHUD() override;

private:
	// Crosshair asset pointer
	class UTexture2D* CrosshairT;
	
};
