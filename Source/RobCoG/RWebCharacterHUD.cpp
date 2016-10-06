// Fill out your copyright notice in the Description page of Project Settings.

#include "RobCoG.h"
#include "RWebCharacterHUD.h"

// Default values
ARWebCharacterHUD::ARWebCharacterHUD()
{
	// Set the crosshair texture
	static ConstructorHelpers::FObjectFinder<UTexture2D> CrosshairTObj(TEXT("/Game/Player/Textures/FirstPersonCrosshair"));
	CrosshairT = CrosshairTObj.Object;
}

// Frist draw call
void ARWebCharacterHUD::DrawHUD()
{
	Super::DrawHUD();

	// Find the center of the Canvas
	const FVector2D Center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);

	// Offset by half the texture's dimensions so that the center of the texture aligns with the center of the Canvas
	const FVector2D CrosshairDrawPosition((Center.X),(Center.Y));
	
	// Draw the crosshair
	FCanvasTileItem TileItem(CrosshairDrawPosition, CrosshairT->Resource, FLinearColor::White);
	TileItem.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(TileItem);
}


