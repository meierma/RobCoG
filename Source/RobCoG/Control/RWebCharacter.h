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

	// Smooth crouch
	void SmoothCrouch();

	// Smooth stand up
	void SmoothStandUp();

	// Handles item selection
	void OnSelect();

	// Handles choosing hands
	void SwitchHands();
	
	// Switch rotation axis of the selected actor
	void SwitchRotAxis();

	// Rotate selected actor into positive direction
	void RotatePos();

	// Rotate selected actor into negative direction
	void RotateNeg();

	// Interact with the highlighted item
	bool InteractWithActor();

	// Collect the highlighted item
	void CollectActor();

	// Collect the highlighted item
	void ReleaseActor();

	// Open/close the highlighted item
	void ManipulateActor();

	// Set cloned actor, used for highlighting release positions
	void SetHighlightClone(AStaticMeshActor* ActorToClone);

	// Set cloned actor, used for highlighting release positions
	FORCEINLINE void RemoveHighlightClone();

	// Trace and highlight manipulation
	FORCEINLINE void TraceAndHighlight();

	// Highlight release area
	FORCEINLINE void HighlightRelease();

	// Calculate the offset from the plane
	FORCEINLINE void CalculatePlaneOffset();

	// Check release collisions
	FORCEINLINE bool IsCollidingAtRelease();

	// Highlight interaction
	FORCEINLINE void HighlightInteraction();

	// Character camera
	UCameraComponent* CharacterCamera;

	// Speed factor, used for slowing/increasing the speed of the caracter
	float SpeedFactor;

	// Smooth crouch timer handle
	FTimerHandle SmoothCrouchTimerHandle;

	// Standing height
	float StandingHeight;

	// In crouch position
	bool bIsCrouched;
	
	// Set of all interactive actors
	TMap<AStaticMeshActor*, EItemInteraction> InteractiveActors;

	// Openable actors and their opened state
	TMap<AStaticMeshActor*, bool> InteractiveActorsToOpenedState;

	// Selected hand
	ESelectedHand SelectedHand;

	// Selected hand to picked actor
	TMap<ESelectedHand, AStaticMeshActor*> HandToItem;

	// Trace hit result
	FHitResult HitResult;

	// Currently highlighted actor
	AStaticMeshActor* HighlightedActor;

	// Highlight clone actor, used for highlighting release positions
	AStaticMeshActor* HighlightClone;

	// Highlight clone release offset
	FVector HCReleaseOffset;

	// Highlight clone material green
	UMaterialInstanceConstant* HCGreenMat;

	// Highlight clone material red
	UMaterialInstanceConstant* HCRedMat;

	// Flag showing current material (avoids re-setting the same material)
	bool bIsGreen;

	// Rotator axis used for rotating the selected object
	uint8 RotAxisIndex;
};

