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
	Pickable			UMETA(DisplayName = "Pickable"),
	Openable			UMETA(DisplayName = "Openable"),
	TwoHandsPickable	UMETA(DisplayName = "TwoHandsPickable"),
	TwoHandsOpenable	UMETA(DisplayName = "TwoHandsOpenable")
};

// Enum of the possible actor interaction
UENUM()
enum class EItemStackable : uint8
{
	SameType		UMETA(DisplayName = "SameType"),
	Mixed			UMETA(DisplayName = "Mixed"),
	Tray			UMETA(DisplayName = "Tray")
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


	// Handles choosing hands
	void SwitchHands();

	// Switch rotation axis of the selected actor
	void SwitchRotAxis();

	// Rotate selected actor into positive direction
	void RotatePos();

	// Rotate selected actor into negative direction
	void RotateNeg();

	// Handles item selection
	void OnSelect();

	// Collect the highlighted item
	void CollectActor();

	// Release the collected items
	void ReleaseActor();

	// Open/close the highlighted item
	void ManipulateActor();

	// Highlight interaction
	FORCEINLINE void HighlightInteraction();
	
	// Check for a possible release area of the objects in hand
	FORCEINLINE void CheckReleaseArea();
	
	// Check release collisions
	FORCEINLINE bool RootCloneIsColliding();

	// Set highlighted selection
	FORCEINLINE void SetHighlights(AStaticMeshActor* RootActor);

	// Check multiple highlights (stack, tray)
	FORCEINLINE void CreateHighlightStack(AStaticMeshActor* RootActor);

	// Clear highlighted selection
	FORCEINLINE void RemoveHighlights();

	// Set cloned actor, used for highlighting release positions
	void CreateClones(AStaticMeshActor* RootActorToClone);

	// Hide/view the cloned objects
	FORCEINLINE void ShowClonedObjects(bool bShow);

	// Color the cloned objects
	FORCEINLINE void ColorClonedObjects(UMaterialInstanceConstant* Material);

	// Set cloned actor, used for highlighting release positions
	void RemoveClones();

	// Character camera
	UCameraComponent* CharacterCamera;

	// Collision parameters for the trace call, clone root is added to ignore list
	FCollisionQueryParams TraceParams;

	// Speed factor, used for slowing/increasing the speed of the caracter
	float SpeedFactor;

	// Smooth crouch timer handle
	FTimerHandle SmoothCrouchTimerHandle;

	// Standing height
	float StandingHeight;

	// In crouch position
	bool bIsCrouched;

	// Set of all interactive actors
	TMap<AActor*, EItemInteraction> InteractiveActors;

	// Set of all interactive actors
	TMap<AActor*, EItemStackable> StackableActors;

	// Openable actors and their opened state
	TMap<AStaticMeshActor*, bool> InteractiveActorsToOpenedState;

	// Selected hand
	ESelectedHand SelectedHand;

	// Selected hand to collected root item
	TMap<ESelectedHand, AStaticMeshActor*> HandToRootItem;

	// Currently attached stack
	TArray<AStaticMeshActor*> AttachedStack;

	// Trace hit result
	FHitResult HitResult;

	// Currently highlighted root actor
	AStaticMeshActor* HighlightedRoot;

	// Currently highlighted stack
	TArray<AStaticMeshActor*> HighlightedStack;

	// Currently highlighted tray stack
	TArray<AStaticMeshActor*> HighlightedTrayStack;

	// Clone of the attached root object (green/red) for release visualization
	AStaticMeshActor* CloneRoot;

	// Cloned stack (green/red) for release visualization
	TArray<AStaticMeshActor*> CloneStack;

	// Highlight clone material green
	UMaterialInstanceConstant* GreenMat;

	// Highlight clone material red
	UMaterialInstanceConstant* RedMat;

	// Flag showing current clone material color
	bool bIsGreen;

	// Flag showing that the clones are visible
	bool bIsCloneVisible;

	// Rotator axis used for rotating the selected object
	uint8 RotAxisIndex;
};

