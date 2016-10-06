// Fill out your copyright notice in the Description page of Project Settings.

#include "RobCoG.h"
#include "RWebCharacterHUD.h"
#include "RWebGameMode.h"

ARWebGameMode::ARWebGameMode()
	: Super()
{
	// Use custom HUD class
	HUDClass = ARWebCharacterHUD::StaticClass();
}


