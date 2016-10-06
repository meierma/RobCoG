// Fill out your copyright notice in the Description page of Project Settings.

#include "RobCoG.h"
#include "RWebCharacter.h"


// Sets default values
ARWebCharacter::ARWebCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Set this pawn to be controlled by the lowest-numbered player
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(5.f, 80.0f);

	// The caracter can croush
	//TODO set as default

	// Create a CameraComponent
	CharacterCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("CharacterCamera"));
	// Set capsule component as parent for the camera
	CharacterCamera->SetupAttachment(GetRootComponent());
	// Position the camera
	CharacterCamera->RelativeLocation = FVector(10.0f, 0.0f, BaseEyeHeight);
	// Allow the pawn to control the camera rotation
	CharacterCamera->bUsePawnControlRotation = true;
	
	//Initialize TraceParams parameter
	TraceParams = FCollisionQueryParams(FName(TEXT("Trace")), true, this);
	TraceParams.bTraceComplex = true;
	//TraceParams.bTraceAsyncScene = true;
	TraceParams.bReturnPhysicalMaterial = false;

	// Set the maximum grasping length (Length of the 'hands' of the character)
	MaxGraspLength = 100.f;

	// Speed factor default value
	SpeedFactor = 1.0f;

	// Default selected hand (right)
	SelectedHand = ESelectedHand::Right;
}

// Called when the game starts or when spawned
void ARWebCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// Init items that can interact with the character
	ARWebCharacter::InitInteractiveItems();
}

// Called every frame
void ARWebCharacter::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	// Highlight interactive objects from the trace
	ARWebCharacter::TraceHighlight();
}

// Init interactive intems
void ARWebCharacter::InitInteractiveItems()
{
	UE_LOG(RobCoG, Log, TEXT(" ** Init interactive items: "));
	// Iterate through the static mesh actors and check tags to see which objects should be logged
	for (TActorIterator<AStaticMeshActor> ActItr(GetWorld()); ActItr; ++ActItr)
	{
		// Iterate throught the tags
		for (const auto TagItr : ActItr->Tags)
		{
			// Copy of the current tag
			FString CurrTag = TagItr.ToString();

			// Check if the tag describes the interactive properties of the actor
			if (CurrTag.RemoveFromStart("Interactive:"))
			{
				UE_LOG(RobCoG, Log, TEXT(" \t %s: "), *ActItr->GetName());

				// Parse tag string into array of strings reprsenting comma separated key-value pairs
				TArray<FString> TagKeyValueArr;
				CurrTag.ParseIntoArray(TagKeyValueArr, TEXT(";"));

				// Iterate the array of key-value strings and add them to the map
				for (const auto TagKeyValItr : TagKeyValueArr)
				{
					// Split string and add the key-value to the string pair
					FString Key;
					FString Val;
					TagKeyValItr.Split(TEXT(","), &Key, &Val);
					UE_LOG(RobCoG, Log, TEXT(" \t\t %s : %s"), *Key, *Val);

					// Check key type
					if (Key.Equals("Interaction"))
					{
						if (Val.Equals("Openable"))
						{
							InteractiveActors.Add(*ActItr, EItemInteraction::Openable);
						}
						else if (Val.Equals("Pickable"))
						{
							InteractiveActors.Add(*ActItr, EItemInteraction::Pickable);
						}
						else if (Val.Equals("PickableWithTwoHands"))
						{
							InteractiveActors.Add(*ActItr, EItemInteraction::PickableWithTwoHands);
						}
						else
						{
							UE_LOG(RobCoG, Error, TEXT(" !! %s is not a valid interaction."), *Val);
						}
					}
					else if (Key.Equals("Stackable"))
					{
						if (Val.Equals("SameType"))
						{

						}
						else if (Val.Equals("Mixed"))
						{

						}
						else
						{
							UE_LOG(RobCoG, Error, TEXT(" !! %s is not a valid stackable type."), *Val);
						}
					}
					else
					{
						UE_LOG(RobCoG, Error, TEXT(" !! %s is not a valid interaction key type."), *Key);
					}
				}
				// Interactive object info found, stop searching in other tags.
				break;
			}
		}
	}
}

// Called to bind functionality to input
void ARWebCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

	// Set up gameplay key bindings
	InputComponent->BindAxis("MoveForward", this, &ARWebCharacter::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &ARWebCharacter::MoveRight);
	// Default Camera view bindings
	InputComponent->BindAxis("CameraPitch", this, &ARWebCharacter::AddControllerPitchInput);
	InputComponent->BindAxis("CameraYaw", this, &ARWebCharacter::AddControllerYawInput);
	// Bind actions
	InputComponent->BindAction("Crouch", EInputEvent::IE_Pressed, this, &ARWebCharacter::ToggleCrouch);
	InputComponent->BindAction("LeftClick", IE_Pressed, this, &ARWebCharacter::OnSelect);
	InputComponent->BindAction("SwitchHands", IE_Pressed, this, &ARWebCharacter::SwitchHands);
}

// Handles moving forward/backward
void ARWebCharacter::MoveForward(const float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// Find out which way is forward
		FRotator Rotation = Controller->GetControlRotation();
		// Limit pitch when walking or falling
		if (GetCharacterMovement()->IsMovingOnGround() || GetCharacterMovement()->IsFalling())
		{
			Rotation.Pitch = 0.0f;
		}
		// add movement in that direction
		const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::X);
		AddMovementInput(Direction, Value * SpeedFactor);
	}
}

// Handles moving right/left
void ARWebCharacter::MoveRight(const float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// Find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value * SpeedFactor);
	}
}

// Handles character crouch
void ARWebCharacter::ToggleCrouch()
{
	// If we are crouching then CanCrouch will return false. If we cannot crouch then calling Crouch() wont do anything  
	if (CanCrouch() == true)
	{
		Crouch();
		SpeedFactor = 0.5f;
	}
	else
	{
		UnCrouch();
		SpeedFactor = 1.0f;
	}
}

// Handle switching hands
void ARWebCharacter::SwitchHands()
{
	if (SelectedHand == ESelectedHand::Right)
	{
		SelectedHand = ESelectedHand::Left;
	}
	else if (SelectedHand == ESelectedHand::Left)
	{
		SelectedHand = ESelectedHand::Both;
	}
	else
	{
		SelectedHand = ESelectedHand::Right;
	}

	UE_LOG(RobCoG, Warning, TEXT("Switched HAND to: %i"), (uint8)SelectedHand);
}

// Handles mouse click
void ARWebCharacter::OnSelect()
{
	if (HandToItem.Contains(SelectedHand))
	{
		UE_LOG(RobCoG, Warning, TEXT("Put item %s down"), *HandToItem[SelectedHand]->GetName());
		HandToItem.Remove(SelectedHand);
	}
	else if (HighlightedActor)
	{
		ARWebCharacter::IteractWithItem(SelectedHand, HighlightedActor);
	}
	else
	{
		UE_LOG(RobCoG, Warning, TEXT("Invalid action"));
	}
}

// Collect item
bool ARWebCharacter::IteractWithItem(ESelectedHand SelectedHand, AStaticMeshActor* Item)
{
	if (InteractiveActors.Contains(Item))
	{
		// Get item interaction type
		EItemInteraction InteractionType = InteractiveActors[Item];
		
		if (InteractionType == EItemInteraction::Pickable)
		{
			// Add item to map
			HandToItem.Add(SelectedHand, HighlightedActor);

			Item->GetStaticMeshComponent()->SetSimulatePhysics(false);

			Item->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);


			UE_LOG(RobCoG, Warning, TEXT("Selected item: %s is pickable"), *Item->GetName());
		}
		else if (InteractionType == EItemInteraction::Openable)
		{
			UE_LOG(RobCoG, Warning, TEXT("Selected item: %s is openable"), *Item->GetName());
		}

	}

	return true;
}

// Highlight interactive objects from trace
FORCEINLINE void ARWebCharacter::TraceHighlight()
{
	// Vectors to trace between 
	const FVector Start = CharacterCamera->GetComponentLocation();
	const FVector End = Start + CharacterCamera->GetForwardVector() * MaxGraspLength;
	// Trace data result
	FHitResult HitResult(ForceInit);
	// Line trace
	GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Pawn, TraceParams);

	// Check if the hit actor is of type static mesh actor
	if (HitResult.GetActor() && HitResult.GetActor()->IsA(AStaticMeshActor::StaticClass()))
	{
		// Cast to static mesh actor and check if it is an interactive actor
		AStaticMeshActor* HitActor = Cast<AStaticMeshActor>(HitResult.GetActor());
		if (InteractiveActors.Contains(HitActor))
		{
			if (!HighlightedActor)
			{
				// If no highlighted actor exist, highlight this one
				HighlightedActor = HitActor;
				HighlightedActor->GetStaticMeshComponent()->SetRenderCustomDepth(true);

			}
			else if (HighlightedActor == HitActor)
			{
				// Return if the highlighted actor is the same with the previous one
				return;
			}
			else
			{
				// Switch the highlights, turn off previous, highlight current one
				HighlightedActor->GetStaticMeshComponent()->SetRenderCustomDepth(false);
				HighlightedActor = HitActor;
				HighlightedActor->GetStaticMeshComponent()->SetRenderCustomDepth(true);
			}
		}
		else if (HighlightedActor)
		{
			// If actor is highlighted, but the current one is not an interactive one
			HighlightedActor->GetStaticMeshComponent()->SetRenderCustomDepth(false);
			HighlightedActor = nullptr;
		}
	}
	else if (HighlightedActor)
	{
		// If actor is highlighted, but the current one is not a static mesh actor
		HighlightedActor->GetStaticMeshComponent()->SetRenderCustomDepth(false);
		HighlightedActor = nullptr;
	}
}