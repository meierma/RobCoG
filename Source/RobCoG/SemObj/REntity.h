// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "REnums.h"
#include "REntity.generated.h"

UCLASS()
class ROBCOG_API AREntity : public AStaticMeshActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AREntity();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Ontology class name
	UPROPERTY(EditAnywhere, Category = "Robcog|Semantic Map")
	FString ItemClassName;

	// Ontology class name
	UPROPERTY(EditAnywhere, Category = "Robcog|Semantic Logger")
	ERLogType ItemLogType;
	
};
