// Fill out your copyright notice in the Description page of Project Settings.

#include "RobCoG.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "RCloseAllDrawers.h"


// Sets default values
ARCloseAllDrawers::ARCloseAllDrawers()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void ARCloseAllDrawers::BeginPlay()
{
	Super::BeginPlay();
	
	// Apply force to close the drawers (after a delay until the objects fall on the surfaces)
	GetWorldTimerManager().SetTimer(
		CloseFurnitureTimerHandle, this, &ARCloseAllDrawers::CloseDrawers, 1.f, true, 2);
}

// Close drawers
void ARCloseAllDrawers::CloseDrawers()
{
	// Iterate all the constraint actors
	for (TActorIterator<APhysicsConstraintActor> ConstrActItr(GetWorld()); ConstrActItr; ++ConstrActItr)
	{
		// Cast to static mesh actor
		AStaticMeshActor* SMAct = Cast<AStaticMeshActor>(ConstrActItr->GetConstraintComp()->ConstraintActor2);

		if (SMAct)
		{
			// Add impule to static mesh in order to close the drawer/door
			SMAct->GetStaticMeshComponent()->AddImpulse(FVector(-900) * SMAct->GetActorForwardVector());
		}
	}

	// Clear timer
	GetWorldTimerManager().ClearTimer(CloseFurnitureTimerHandle);

	// Check for the SLManager
}