// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "RWebCharacter.generated.h"

// Enum of the possible selected hands
UENUM()
enum class ESelectedHand : uint8
{
	Right	UMETA(DisplayName = "Right"),
	Left	UMETA(DisplayName = "Left"),
	Both	UMETA(DisplayName = "Both")
};

// Enum of the possible actor interaction
UENUM()
enum class EItemInteraction : uint8
{
	Pickable				UMETA(DisplayName = "Pickable"),
	Openable				UMETA(DisplayName = "Openable"),
	PickableWithTwoHands	UMETA(DisplayName = "PickableWithTwoHands"),
	OpenableWithTwoHands	UMETA(DisplayName = "OpenableWithTwoHands")
};


UCLASS()
class ROBCOG_API ARWebCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ARWebCharacter();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	// Maximum arm length for grasping
	UPROPERTY(EditAnywhere, Category = "Robcog")
	float MaxGraspLength;

protected:
	// Initialize interactive items
	void InitInteractiveItems();

	// Handles moving forward/backward
	void MoveForward(const float Val);

	// Handles strafing Left/Right
	void MoveRight(const float Val);

	// Handles character crouch
	void ToggleCrouch();

	// Handles choosing hands
	void SwitchHands();

	// Handles mouse click
	void OnSelect();

	// Collect item
	bool IteractWithItem(ESelectedHand SelectedHand, AStaticMeshActor* Item);

	// Highlight objects
	FORCEINLINE void TraceHighlight();

	// Character camera
	UCameraComponent* CharacterCamera;

	// Speed factor, used for slowing/increasing the speed of the caracter
	float SpeedFactor;
	
	// Set of all interactive actors
	TMap<AStaticMeshActor*, EItemInteraction> InteractiveActors;
	
	// Currently highlighted actor
	AStaticMeshActor* HighlightedActor;

	// Selected hand
	ESelectedHand SelectedHand;

	// Selected hand to picked actor
	TMap<ESelectedHand, AStaticMeshActor*> HandToItem;




	// Currently selected actor
	AActor* SelectedActor;

	// Actor in the right hand
	AActor* RightHandSlot;

	// Actor in the left hand
	AActor* LeftHandSlot;

	// Actor in both hands
	AActor* TwoHandsSlot;


	// Parameters for the raytrace
	FCollisionQueryParams TraceParams;

	// Start vector for raytracing
	FVector Start;

	// End vector for raytracing
	FVector End;

	// Raytrace Hit Result 
	FHitResult HitObject;

	// Hand is selected of performing an action
	ESelectedHand ActionSelectedHand;
};

