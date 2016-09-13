// Fill out your copyright notice in the Description page of Project Settings.

#include "RobCoG.h"
#include "REntity.h"


// Sets default values
AREntity::AREntity()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Default class name
	ItemClassName = GetName() + "_Default";

	// Default log type (static)
	ItemLogType = ERLogType::Dynamic;

	// Physics default values
	GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
	GetStaticMeshComponent()->SetSimulatePhysics(true);

}

// Called when the game starts or when spawned
void AREntity::BeginPlay()
{
	Super::BeginPlay();

	if (ItemClassName.Contains("Default"))
	{
		UE_LOG(RobCoG, Error, TEXT(" ** RItem: %s has no semantic class name set!"),
			*GetActorLabel());
	}
	
}

