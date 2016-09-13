// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Animation/SkeletalMeshActor.h"
#include "PhysicsEngine/ConstraintInstance.h"
#include "RPid3d.h"
#include "RGrasp.h"
#include "REnums.h"
#include "RMCHand.generated.h"

/**
 *  Skeletal mesh actor representing a hand that follows the left or right
 *  motion controller from the RMCCharacter class
 *  the following motion is done force based using a PID controller
 */
UCLASS()
class ROBCOG_API ARMCHand : public ASkeletalMeshActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this character's properties
	ARMCHand();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

protected:
	// Callback on finger tip collision
	UFUNCTION()
	void OnFingerHit(UPrimitiveComponent* SelfComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
			FVector NormalImpulse, const FHitResult& Hit);

	// Hand type
	UPROPERTY(EditAnywhere, Category = "Robcog|Motion Controller")
	EControllerHand HandType;

	// Body name to apply forces on
	UPROPERTY(EditAnywhere, Category = "Robcog|Motion Controller")
	FName ControlBoneName;

	// PID controller proportional argument
	UPROPERTY(EditAnywhere, Category = "Robcog|Motion Controller")
	float PGain;

	// PID controller integral argument
	UPROPERTY(EditAnywhere, Category = "Robcog|Motion Controller")
	float IGain;

	// PID controller derivative argument
	UPROPERTY(EditAnywhere, Category = "Robcog|Motion Controller")
	float DGain;

	// PID controller maximum output
	UPROPERTY(EditAnywhere, Category = "Robcog|Motion Controller")
	float PIDMaxOutput;

	// PID controller minimum output
	UPROPERTY(EditAnywhere, Category = "Robcog|Motion Controller")
	float PIDMinOutput;

	// Hand rotation controller output strength (multiply output velocity)
	UPROPERTY(EditAnywhere, Category = "Robcog|Motion Controller")
	float RotOutStrength;
	
	// Finger types of the hand (make sure the skeletal bone name is part of the finger name)
	UPROPERTY(EditAnywhere, Category = "Robcog|Joint Controller")
	TArray<ERHandPart> FingerTypes;

	// Finger collision bone name (used for collision detection)
	UPROPERTY(EditAnywhere, Category = "Robcog|Joint Controller")
	TArray<FName> CollisionBoneNames;

	// Spring value to apply to the angular drive (Position strength)
	UPROPERTY(EditAnywhere, Category = "Robcog|Joint Controller")
	float Spring;

	// Damping value to apply to the angular drive (Velocity strength) 
	UPROPERTY(EditAnywhere, Category = "Robcog|Joint Controller")
	float Damping;

	// Limit of the force that the angular drive can apply
	UPROPERTY(EditAnywhere, Category = "Robcog|Joint Controller")
	float ForceLimit;

	// Initial velocity
	UPROPERTY(EditAnywhere, Category = "Robcog|Joint Controller")
	float Velocity;

	// Fixating grasp constraint instance
	UPROPERTY(EditAnywhere, Category = "Robcog|Joint Controller")
	FConstraintInstance FixatingGraspConstraintInstance;
	
	// Set the tracking offset of the motion controller
	void SetMCTrackingOffset();

	// Handles closing the hand
	void CloseHand(const float Val);

	// Attach grasped object to hand
	void AttachToHand();

	// Handles opening the left hand
	void OpenHand(const float Val);

private:
	// Motion controller component for the hand to follow
	class UMotionControllerComponent* MCComponent;

	// 3D PID controller for all axis
	FRPid3d HandPID3D;

	// Current location of the control body
	FVector CurrLoc;

	// Current rotation of the control body
	FQuat CurrQuat;

	// Control body
	FBodyInstance* ControlBody;

	// Fingers type to constraints Map
	TMultiMap<ERHandPart, FConstraintInstance*> FingerTypeToConstrs;

	// Fingers type name to finger body (collision)
	TMap<FName, FBodyInstance*> FingerBoneNameToBody;

	// Grasp base class
	FRGrasp* Grasp;

	// Bone name to finger type
	TMap<FName, ERHandPart> BoneNameToFingerTypeMap;

	// Finger hit events enable flag
	bool bFingerHitEvents;

	// Store a map of components in contact with fingers,
	// used for reasoning on what objects to attach to the hand
	// (e.g. Cup : Index, Middle, Pinky, Thumb; Table : Palm; -> Attach Cup )
	TMultiMap<AActor*, ERHandPart> HitActorToFingerMMap;

	// Grasp physics constraint component (fixating grasp case)
	UPhysicsConstraintComponent* GraspFixatingConstraint;

	// Dummy static mesh actor (workaround for the contraint attachment directly to the skeleton issue)
	AStaticMeshActor* DummyStaticMeshActor;

	// Dummy static mesh (workaround for the contraint attachment directly to the skeleton issue)
	UStaticMeshComponent* DummyStaticMesh;

	// Currently grasped component (to enable/disable gravity when grasped)
	UStaticMeshComponent* GraspedComponent;

	// Player controller to apply force feedback and enable input
	APlayerController* PC;
};
