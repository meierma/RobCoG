// Fill out your copyright notice in the Description page of Project Settings.

#include "RobCoG.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "SLManager.h"
#include "SLContactTriggerBox.h"
#include "RSetupEnvironment.h"
#include "XmlParser.h"

// Sets default values
ARSetupEnvironment::ARSetupEnvironment()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void ARSetupEnvironment::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(RobCoG, Log, TEXT(" ** ARSetupEnvironment: waiting for objects to settle .."));

	// Apply force to close the drawers (after a delay until the objects fall on the surfaces)
	GetWorldTimerManager().SetTimer(
		CloseFurnitureTimerHandle, this, &ARSetupEnvironment::CloseFurnitures, 10.0f, true, 1.5f);
}

// Close drawers
void ARSetupEnvironment::CloseFurnitures()
{
	UE_LOG(RobCoG, Log, TEXT(" ** ARSetupEnvironment: closing furniture .."));
	// Clear timer
	GetWorldTimerManager().ClearTimer(CloseFurnitureTimerHandle);

	// Iterate all the constraint actors
	for (TActorIterator<APhysicsConstraintActor> ConstrActItr(GetWorld()); ConstrActItr; ++ConstrActItr)
	{
		// Cast to static mesh actor
		AStaticMeshActor* SMAct = Cast<AStaticMeshActor>(
			ConstrActItr->GetConstraintComp()->ConstraintActor2);
		if (SMAct)
		{
			// Add impule to static mesh in order to close the drawer/door
			SMAct->GetStaticMeshComponent()->AddImpulse(FVector(-900) * SMAct->GetActorForwardVector());
		}
	}

	// Check if there is a SLManager
	for (TActorIterator<ASLManager> SLManagerItr(GetWorld()); SLManagerItr; ++SLManagerItr)
	{
		//SLManagerItr
		SemLogManager = *SLManagerItr;		
		break;
	}
	if (!SemLogManager)
	{
		UE_LOG(RobCoG, Error, TEXT(" !! ARSetupEnvironment: no Semlog Manager found .."));
		return;
	}

	// Init semlog manager after a delay for closing the drawers
	GetWorldTimerManager().SetTimer(
		InitLoggingTimerHandle, this, &ARSetupEnvironment::InitLogging, 10.0f, true, 3);
}

// Init logging
void ARSetupEnvironment::InitLogging()
{
	UE_LOG(RobCoG, Log, TEXT(" ** ARSetupEnvironment: init logging current state .."));
	// Clear timer
	GetWorldTimerManager().ClearTimer(InitLoggingTimerHandle);

	// Init the semantic logger
	if (SemLogManager)
	{
		SemLogManager->Init();
	}

	// Locate all triger boxes and refresh them
	for (TActorIterator<ASLContactTriggerBox> SLTriggerBoxItr(GetWorld()); SLTriggerBoxItr; ++SLTriggerBoxItr)
	{
		SLTriggerBoxItr->SetActorEnableCollision(false);
		SLTriggerBoxItr->SetActorEnableCollision(true);
	}

	// TODO check start logging here (remove start logging timer handle if it works)
	// Check drawer states with the given update rate (add delay until the drawers are closed)
	GetWorldTimerManager().SetTimer(
		StartLoggingTimerHandle, this, &ARSetupEnvironment::StartLogging, 10.0f, true, 0.1f);
}

// Start logging
void ARSetupEnvironment::StartLogging()
{
	UE_LOG(RobCoG, Log, TEXT(" ** ARSetupEnvironment: starting semantic logging .."));
	// Clear timer
	GetWorldTimerManager().ClearTimer(StartLoggingTimerHandle);

	if (SemLogManager)
	{
		SemLogManager->Start();
	}

	//// Check drawer states with the given update rate (add delay until the drawers are closed)
	//GetWorldTimerManager().SetTimer(
	//	StopLoggingTimerHandle, this, &ARSetupEnvironment::StopLogging, 1.0f, true, 3);
}

// Stop logging
void ARSetupEnvironment::StopLogging()
{
	if (SemLogManager)
	{
		SemLogManager->Pause();
	}

	//// Clear timer
	//GetWorldTimerManager().ClearTimer(StopLoggingTimerHandle);

	//// Check drawer states with the given update rate (add delay until the drawers are closed)
	//GetWorldTimerManager().SetTimer(
	//	RestartLoggingTimerHandle, this, &ARSetupEnvironment::RestartLogging, 1.0f, true, 3);
}

// ReStart logging
void ARSetupEnvironment::RestartLogging()
{
	if (SemLogManager)
	{
		SemLogManager->Start();
	}

	//// Clear timer
	//GetWorldTimerManager().ClearTimer(RestartLoggingTimerHandle);
}