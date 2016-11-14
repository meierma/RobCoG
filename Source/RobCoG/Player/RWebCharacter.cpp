// Fill out your copyright notice in the Description page of Project Settings.

#include "RobCoG.h"
#include "RWebCharacter.h"

#define RIGHT_HAND FVector(10.f, 20.f, 25.f)
#define LEFT_HAND FVector(10.f, -20.f, 25.f)
#define BOTH_HANDS FVector(5.f, 0.f, 25.f)
#define IMPULSE FVector(1500.f)
#define MAX_STACK_HEIGHT 20.0f

// Sets default values
ARWebCharacter::ARWebCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// Set this pawn to be controlled by the lowest-numbered player
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(5.f, 80.0f);
	// Smooth crouch flag
	bIsCrouched = false;
	// Set the maximum grasping length (Length of the 'hands' of the character)
	MaxGraspLength = 100.f;
	// Speed factor default value
	SpeedFactor = 1.0f;
	// Default selected hand (right)
	SelectedHand = ESelectedHand::Right;
	// Flag of currently selected materisl (avoids re-setting the same material)
	bIsGreen = false;
	// Flag showing that the clone is visible
	bIsCloneVisible = true;
	// Default rotation index
	RotAxisIndex = 0;

	// Create a CameraComponent
	CharacterCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("CharacterCamera"));
	// Set capsule component as parent for the camera
	CharacterCamera->SetupAttachment(GetRootComponent());
	// Position the camera
	CharacterCamera->RelativeLocation = FVector(0.0f, 0.0f, BaseEyeHeight);
	// Allow the pawn to control the camera rotation
	CharacterCamera->bUsePawnControlRotation = true;
	
	// Highlight materials
	GreenMat = ConstructorHelpers::FObjectFinderOptional<UMaterialInstanceConstant>(
		TEXT("MaterialInstanceConstant'/Game/Highlights/M_HighlightClone_Green_Inst.M_HighlightClone_Green_Inst'")).Get();
	RedMat = ConstructorHelpers::FObjectFinderOptional<UMaterialInstanceConstant>(
		TEXT("MaterialInstanceConstant'/Game/Highlights/M_HighlightClone_Red_Inst.M_HighlightClone_Red_Inst'")).Get();
	
	// Initialize the trace parameters, used for ignoring the cloned objets
	//TraceParams = FCollisionQueryParams();
	TraceParams.TraceTag = FName("UserTrace");
	// Self ignore
	TraceParams.AddIgnoredActor(this);
}

// Called when the game starts or when spawned
void ARWebCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Standing height
	StandingHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	// Check if materials are present
	if (!GreenMat || !RedMat)
	{
		UE_LOG(RobCoG, Error, TEXT(" !! ARWebCharacter: No highlight materials found!"));
	}
	
	// Init items that can interact with the character
	ARWebCharacter::InitInteractiveItems();

	// TODO add as UPROPERTY
	//GetWorld()->DebugDrawTraceTag = FName("UserTrace");
}

// Called every frame
void ARWebCharacter::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	// Vectors to trace between 
	const FVector Start = CharacterCamera->GetComponentLocation();
	const FVector End = Start + CharacterCamera->GetForwardVector() * MaxGraspLength;

	// Calculate pointing line trace, ignore cloned object (TraceParam)
	GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_PhysicsBody, TraceParams);

	// Check if selected hand is occupied
	if (HandToRootItem.Contains(SelectedHand))
	{
		if (HitResult.IsValidBlockingHit())
		{
			// Check release possilibity of the objects in hand
			ARWebCharacter::CheckReleaseArea();
		}
		else if(bIsCloneVisible)
		{
			// Hit is not valid, hide the cloned objects
			ARWebCharacter::ShowClonedObjects(false);
		}
	}	
	else if (InteractiveActors.Contains(HitResult.GetActor()))
	{
		// Hand is free, hit actor is interactive -> highlight actor(s)
		ARWebCharacter::HighlightInteraction();
	}
	else if (HighlightedRoot)
	{
		ARWebCharacter::RemoveHighlights();
	}
}

// Called to bind functionality to input
void ARWebCharacter::SetupPlayerInputComponent(class UInputComponent* InputComp)
{
	Super::SetupPlayerInputComponent(InputComp);

	// Set up gameplay key bindings
	InputComp->BindAxis("MoveForward", this, &ARWebCharacter::MoveForward);
	InputComp->BindAxis("MoveRight", this, &ARWebCharacter::MoveRight);
	// Default Camera view bindings
	InputComp->BindAxis("CameraPitch", this, &ARWebCharacter::AddControllerPitchInput);
	InputComp->BindAxis("CameraYaw", this, &ARWebCharacter::AddControllerYawInput);
	// Bind actions
	InputComp->BindAction("Crouch", EInputEvent::IE_Pressed, this, &ARWebCharacter::ToggleCrouch);
	InputComp->BindAction("Select", IE_Pressed, this, &ARWebCharacter::OnSelect);
	InputComp->BindAction("SwitchHands", IE_Pressed, this, &ARWebCharacter::SwitchHands);
	// Bind object rotation
	InputComp->BindAction("SwitchRotationAxis", EInputEvent::IE_Pressed, this, &ARWebCharacter::SwitchRotAxis);
	InputComp->BindAction("RotatePos", IE_Pressed, this, &ARWebCharacter::RotatePos);
	InputComp->BindAction("RotateNeg", IE_Pressed, this, &ARWebCharacter::RotateNeg);
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
							// Set all furniture to closed state as default
							InteractiveActorsToOpenedState.Add(*ActItr, false);
						}
						else if (Val.Equals("Pickable"))
						{
							InteractiveActors.Add(*ActItr, EItemInteraction::Pickable);
						}
						else if (Val.Equals("TwoHandsPickable"))
						{
							InteractiveActors.Add(*ActItr, EItemInteraction::TwoHandsPickable);
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
							StackableActors.Add(*ActItr, EItemStackable::SameType);
						}
						else if (Val.Equals("Mixed"))
						{
							StackableActors.Add(*ActItr, EItemStackable::Mixed);
						}
						else if (Val.Equals("Tray"))
						{
							StackableActors.Add(*ActItr, EItemStackable::Tray);
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
	if (!bIsCrouched)
	{
		// Callback to crouch character
		GetWorldTimerManager().SetTimer(
			SmoothCrouchTimerHandle, this, &ARWebCharacter::SmoothCrouch, 0.001f, true);
		// Set crouch flag
		bIsCrouched = true;
		// Slow down movement
		SpeedFactor = 0.2f;
	}
	else
	{
		// Callback to bring up character
		GetWorldTimerManager().SetTimer(
			SmoothCrouchTimerHandle, this, &ARWebCharacter::SmoothStandUp, 0.001f, true);
		// Set crouch flag
		bIsCrouched = false;
	}
}

// Smooth crouch
void ARWebCharacter::SmoothCrouch()
{
	// Current height of the capsule
	const float CurrHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	GetCapsuleComponent()->SetCapsuleHalfHeight(CurrHeight - 0.1f);
	
	if (CurrHeight <= (StandingHeight * 0.4))
	{
		// Clear crouch timer
		GetWorldTimerManager().ClearTimer(SmoothCrouchTimerHandle);
	}
}

// Smooth stand up
void ARWebCharacter::SmoothStandUp()
{
	// Current height of the capsule
	const float CurrHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	GetCapsuleComponent()->SetCapsuleHalfHeight(CurrHeight + 0.1f);

	if (CurrHeight >= StandingHeight)
	{
		// Increase movement speed only when completely standing
		SpeedFactor = 1.0f;
		// Clear stand up timer
		GetWorldTimerManager().ClearTimer(SmoothCrouchTimerHandle);
	}
}

// Switch rotation axis of the selected actor
void ARWebCharacter::SwitchRotAxis()
{
	// Stack can only be rotated on first axis
	if (CloneStack.Num() > 0)
	{
		RotAxisIndex = 0;
		return;
	}
	//Increment the index which coresponds to rotation axis
	RotAxisIndex++;
	if (RotAxisIndex > 2)
	{
		RotAxisIndex = 0;
	}
	UE_LOG(RobCoG, Warning, TEXT(" ** Rot Index: %i"), RotAxisIndex);
}

// Rotate selected actor into positive direction
void ARWebCharacter::RotatePos()
{
	if (CloneRoot)
	{
		switch (RotAxisIndex)
		{
		case 0: 		
			//RootClone->AddActorLocalRotation(FRotator(0.f, 10.f, 0.f)); // Yaw
			CloneRoot->AddActorWorldRotation(FRotator(0.f, 10.f, 0.f)); // Yaw
			break;
		case 1: 
			//RootClone->AddActorLocalRotation(FRotator(10.f, 0.f, 0.f)); // Pitch
			CloneRoot->AddActorWorldRotation(FRotator(10.f, 0.f, 0.f)); // Pitch
			break;
		case 2: 
			//RootClone->AddActorLocalRotation(FRotator(0.f, 0.f, 10.f)); // Roll
			CloneRoot->AddActorWorldRotation(FRotator(0.f, 0.f, 10.f)); // Roll
			break;
		default:
			return;
		}
	}
}

// Rotate selected actor into negative direction
void ARWebCharacter::RotateNeg()
{
	if (CloneRoot)
	{
		switch (RotAxisIndex)
		{
		case 0:
			//RootClone->AddActorLocalRotation(FRotator(0.f, -10.f, 0.f)); // Yaw
			CloneRoot->AddActorWorldRotation(FRotator(0.f, -10.f, 0.f)); // Yaw
			break;
		case 1:
			//RootClone->AddActorLocalRotation(FRotator(-10.f, 0.f, 0.f)); // Pitch
			CloneRoot->AddActorWorldRotation(FRotator(-10.f, 0.f, 0.f)); // Pitch
			break;
		case 2:
			//RootClone->AddActorLocalRotation(FRotator(0.f, 0.f, -10.f)); // Roll
			CloneRoot->AddActorWorldRotation(FRotator(0.f, 0.f, -10.f)); // Roll
			break;
		default:
			return;
		}
	}
}

// Handle switching hands
void ARWebCharacter::SwitchHands()
{
	// Select new hand
	if (SelectedHand == ESelectedHand::Right)
	{
		SelectedHand = ESelectedHand::Left;
		UE_LOG(RobCoG, Warning, TEXT(" ** Selected hand: LEFT"));
	}
	else if (SelectedHand == ESelectedHand::Left)
	{
		// If both hands are free, switch to both
		if (!HandToRootItem.Contains(ESelectedHand::Left) 
			&& !HandToRootItem.Contains(ESelectedHand::Right))
		{
			SelectedHand = ESelectedHand::Both;
			UE_LOG(RobCoG, Warning, TEXT(" ** Selected hand: BOTH"));
		}
		else
		{
			SelectedHand = ESelectedHand::Right;
			UE_LOG(RobCoG, Warning, TEXT(" ** Selected hand: RIGHT"));
		}
	}
	else if (SelectedHand == ESelectedHand::Both)
	{
		if (!HandToRootItem.Contains(ESelectedHand::Both))
		{
			// If both hands are empty, switch to right
			SelectedHand = ESelectedHand::Right;
			UE_LOG(RobCoG, Warning, TEXT(" ** Selected hand: RIGHT"));
		}
		else
		{
			return;
		}
	}

	// Hand switched, clear previous highlights and clones
	ARWebCharacter::RemoveHighlights();
	ARWebCharacter::RemoveClones();
	
	// If the hand switched, and the current hand is occupied -> create clone
	if (HandToRootItem.Contains(SelectedHand))
	{
		// Switch to the other hand clone
		ARWebCharacter::CreateClones(HandToRootItem[SelectedHand]);
	}
}

// Handles mouse click
void ARWebCharacter::OnSelect()
{
	if (HandToRootItem.Contains(SelectedHand))
	{
		// If selected hand is in use
		ARWebCharacter::ReleaseActor();
	}
	else if (HighlightedRoot)
	{
		// If selected hand is free, and an actor is highlighted
		if (InteractiveActors.Contains(HighlightedRoot))
		{
			// Get item interaction type
			EItemInteraction InteractionType = InteractiveActors[HighlightedRoot];

			if (InteractionType == EItemInteraction::Pickable)
			{
				ARWebCharacter::CollectActor();
			}
			else if (InteractionType == EItemInteraction::Openable)
			{
				ARWebCharacter::ManipulateActor();
			}
		}
	}
	else
	{
		UE_LOG(RobCoG, Warning, TEXT(" ** ARWebCharacter: Invalid action!"));
	}
}

// Release the collected items
void ARWebCharacter::ReleaseActor()
{
	// Release if the highlighted clone is green
	if (HitResult.IsValidBlockingHit() && bIsGreen)
	{
		// Currently selected actor
		AStaticMeshActor* CurrRootActor = HandToRootItem[SelectedHand];

		// Remove object from hand
		HandToRootItem.Remove(SelectedHand);

		// Detach component, fix collision and physics
		CurrRootActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		CurrRootActor->GetStaticMeshComponent()->SetSimulatePhysics(true);
		CurrRootActor->GetStaticMeshComponent()->bGenerateOverlapEvents = true;
		CurrRootActor->GetStaticMeshComponent()->SetCollisionProfileName("PhysicsActor");

		// Set actor location
		CurrRootActor->SetActorLocationAndRotation(
			CloneRoot->GetActorLocation(), CloneRoot->GetActorQuat());

		// Detach stack if available
		if (AttachedStack.Num() > 0)
		{
			for (const auto AttachStackItr : AttachedStack)
			{
				AttachStackItr->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
				AttachStackItr->GetStaticMeshComponent()->SetSimulatePhysics(true);
				AttachStackItr->GetStaticMeshComponent()->bGenerateOverlapEvents = true;
				AttachStackItr->GetStaticMeshComponent()->SetCollisionProfileName("PhysicsActor");
			}
			// Clear stack
			AttachedStack.Empty();
		}
		// Remove clone(s)
		ARWebCharacter::RemoveClones();
	}
	else
	{
		UE_LOG(RobCoG, Warning, TEXT(" ** ARWebCharacter: Cannot put %s down."), *HandToRootItem[SelectedHand]->GetName());
	}
}

// Collect the highlighted item
void ARWebCharacter::CollectActor()
{
	// Set the relative  position of the attached item
	if (SelectedHand == ESelectedHand::Right)
	{
		// Create clones of the collected objects
		ARWebCharacter::CreateClones(HighlightedRoot);
		// Disable physics, collisions and attach root item to the character
		HighlightedRoot->GetStaticMeshComponent()->SetSimulatePhysics(false);
		HighlightedRoot->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HighlightedRoot->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
		HighlightedRoot->SetActorRelativeLocation(RIGHT_HAND);
	}
	else if (SelectedHand == ESelectedHand::Left)
	{
		// Create clones of the collected objects
		ARWebCharacter::CreateClones(HighlightedRoot);
		// Disable physics, collisions and attach root item to the character
		HighlightedRoot->GetStaticMeshComponent()->SetSimulatePhysics(false);
		HighlightedRoot->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HighlightedRoot->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
		HighlightedRoot->SetActorRelativeLocation(LEFT_HAND);

	}
	else if (SelectedHand == ESelectedHand::Both)
	{
		// Attache stacked items first to the root item
		if (HighlightedStack.Num() > 0)
		{
			float StackHeight = 0.0f;
			for (const auto HighlightStackItr : HighlightedStack)
			{	
				// Get bounding box before disabling simulation
				StackHeight += HighlightStackItr->GetComponentsBoundingBox().GetExtent().Z;
				// Disable physics, collisions and attach item to the root actor
				HighlightStackItr->GetStaticMeshComponent()->SetSimulatePhysics(false);
				HighlightStackItr->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				HighlightStackItr->AttachToActor(HighlightedRoot, FAttachmentTransformRules::KeepWorldTransform);
				// Set stack height
				HighlightStackItr->SetActorRelativeLocation(FVector(0.f, 0.f, StackHeight));
			}
			// Save stack, since the highlight will be removed
			AttachedStack = HighlightedStack;
		}
		else if (HighlightedTrayStack.Num() > 0)
		{
			for (const auto HighlightTrayStackItr : HighlightedTrayStack)
			{
				// Disable physics, collisions and attach item to the root actor
				HighlightTrayStackItr->GetStaticMeshComponent()->SetSimulatePhysics(false);
				HighlightTrayStackItr->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				HighlightTrayStackItr->AttachToActor(HighlightedRoot, FAttachmentTransformRules::KeepWorldTransform);
			}
			// Save stack, since the highlight will be removed
			AttachedStack = HighlightedTrayStack;
		}

		// Create clones of the collected objects
		ARWebCharacter::CreateClones(HighlightedRoot);
		// Attach root to character after the objects have been attached to the root
		HighlightedRoot->GetStaticMeshComponent()->SetSimulatePhysics(false);
		HighlightedRoot->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HighlightedRoot->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
		HighlightedRoot->SetActorRelativeLocation(BOTH_HANDS);
	}
	
	// Add item to map (hand occupied)
	HandToRootItem.Add(SelectedHand, HighlightedRoot);
	// Remove highlights
	ARWebCharacter::RemoveHighlights();
}

// Open/close the highlighted item
void ARWebCharacter::ManipulateActor()
{
	if (InteractiveActorsToOpenedState.Contains(HighlightedRoot))
	{
		if (InteractiveActorsToOpenedState[HighlightedRoot])
		{
			// Close furniture
			HighlightedRoot->GetStaticMeshComponent()->AddImpulse(-IMPULSE * HighlightedRoot->GetActorForwardVector());
			// Set state to closed
			InteractiveActorsToOpenedState[HighlightedRoot] = false;
		}
		else
		{
			// Open furniture
			HighlightedRoot->GetStaticMeshComponent()->AddImpulse(IMPULSE * HighlightedRoot->GetActorForwardVector());
			// Set state to closed
			InteractiveActorsToOpenedState[HighlightedRoot] = true;
		}
	}
}

// Highlight interaction
FORCEINLINE void ARWebCharacter::HighlightInteraction()
{
	// Cast hit actor to a static mesh
	AStaticMeshActor* NewHitActor = Cast<AStaticMeshActor>(HitResult.GetActor());

	// Return if the highlighted actor was set and did not change
	if (HighlightedRoot && HighlightedRoot == NewHitActor)
	{
		return;
	}

	if (!HighlightedRoot)
	{
		// If no prev highlighted actor exist, highlight this one
		ARWebCharacter::SetHighlights(NewHitActor);
	}
	else
	{
		// Replace old highlights
		ARWebCharacter::RemoveHighlights();
		ARWebCharacter::SetHighlights(NewHitActor);
	}
}

// Highlight release area
FORCEINLINE void ARWebCharacter::CheckReleaseArea()
{
	// Set root clone location
	CloneRoot->SetActorLocation(HitResult.ImpactPoint + 
		(HitResult.Normal * CloneRoot->GetComponentsBoundingBox(true).GetExtent() + 1.1f));
	
	// Check if root clone is in collision
	if (!ARWebCharacter::RootCloneIsColliding())
	{
		// Draw the cloned objects with green
		if (!bIsGreen)
		{
			ARWebCharacter::ColorClonedObjects(GreenMat);
			bIsGreen = true;
		}
	}
	else
	{
		// Draw the cloned objects with red
		if (bIsGreen)
		{
			ARWebCharacter::ColorClonedObjects(RedMat);
			bIsGreen = false;
		}
	}

	// If clone(s) is not visible, set it to visible
	if (!bIsCloneVisible)
	{
		ARWebCharacter::ShowClonedObjects(true);
	}
}

// Check if object is colliding at release
FORCEINLINE bool ARWebCharacter::RootCloneIsColliding()
{
	FComponentQueryParams CompCollParams(TEXT("CollOverl"), CloneRoot);
	CompCollParams.TraceTag = FName("CollOverl");
	FCollisionResponseParams ResponseParam; // TODO It was in the example code, see why was it needed
	UPrimitiveComponent* RootPrimitiveComp = Cast<class UPrimitiveComponent>(CloneRoot->GetRootComponent());
	RootPrimitiveComp->InitSweepCollisionParams(CompCollParams, ResponseParam);

	if (!RootPrimitiveComp)
	{
		UE_LOG(RobCoG, Error,
			TEXT(" !! ARWebCharacter::RootCloneIsColliding: Could not cast root component into UPrimitiveComponent!"));
		return false;
	}

	TArray<FOverlapResult> Overlaps;
	GetWorld()->ComponentOverlapMulti(
		Overlaps, RootPrimitiveComp, CloneRoot->GetActorLocation(), CloneRoot->GetActorQuat(), CompCollParams);


	//UE_LOG(RobCoG, Warning, TEXT(" ** Coll overlaps: "));
	//for (const auto OverlapItr : Overlaps)
	//{
	//	UE_LOG(RobCoG, Warning, TEXT(" \t %s "), *OverlapItr.GetActor()->GetName());
	//}

	if (Overlaps.Num() > 0)
	{
		return true;
	}
	return false;
}

// Set highlighted selection
FORCEINLINE void ARWebCharacter::SetHighlights(AStaticMeshActor* RootActor)
{
	RootActor->GetStaticMeshComponent()->SetRenderCustomDepth(true);
	// If both hands are selected and the root actor is stackable
	if (SelectedHand == ESelectedHand::Both
		&& StackableActors.Contains(RootActor))
	{
		// Highlighted actor is stackable
		ARWebCharacter::CreateHighlightStack(RootActor);
	}
	HighlightedRoot = RootActor;
}

// Check for stacking higlight
FORCEINLINE void ARWebCharacter::CreateHighlightStack(AStaticMeshActor* RootActor)
{
	// TODO cleanup extra code, eg debug draw trace..
	// Get root actor location
	const FVector ActorLoc = RootActor->GetActorLocation();	

	// Check stackable type
	if (StackableActors[RootActor] == EItemStackable::Tray)
	{
		TArray<FHitResult> TrayHitResults;
		
		FComponentQueryParams CompCollParams(TEXT("TrayTrace"), RootActor);
		CompCollParams.TraceTag = FName("TrayTrace");
		//FCollisionResponseParams ResponseParam; // It was in the example code, see why was it needed
		//RootActor->GetRootPrimitiveComponent()->InitSweepCollisionParams(CompCollParams, ResponseParam);
		GetWorld()->DebugDrawTraceTag = FName("TrayTrace");
		UE_LOG(RobCoG, Warning, TEXT(" ** Tray swipe:"));
		UPrimitiveComponent* RootPrimitiveComp = Cast<UPrimitiveComponent>(RootActor->GetRootComponent());

		if (!RootPrimitiveComp)
		{
			UE_LOG(RobCoG, Error, 
				TEXT(" !! ARWebCharacter::CreateHighlightStack: Could not cast root component into UPrimitiveComponent!"));
			return;
		}

		// Check object on tray
		if (GetWorld()->ComponentSweepMulti(
			TrayHitResults,
			RootPrimitiveComp,
			ActorLoc + FVector(0.f, 0.f, 2.f),
			ActorLoc + FVector(0.f, 0.f, MAX_STACK_HEIGHT),
			RootActor->GetActorQuat(),
			CompCollParams))
		{
			for (const auto TrayHitItr : TrayHitResults)
			{
				UE_LOG(RobCoG, Warning, TEXT("\t %s"), *TrayHitItr.GetActor()->GetName());
				if (InteractiveActors.Contains(TrayHitItr.GetActor()))
				{
					// If actor is interactive cast to static mesh actor and add it to the stack
					AStaticMeshActor* CurrStackActor = Cast<AStaticMeshActor>(TrayHitItr.GetActor());
					if (CurrStackActor)
					{
						// Set highlight to true;
						CurrStackActor->GetStaticMeshComponent()->SetRenderCustomDepth(true);
						HighlightedTrayStack.AddUnique(CurrStackActor);

						
					}
				}
			}
		}
	}
	else
	{
		FCollisionQueryParams CollisionParams;
		CollisionParams.TraceTag = FName("StackTrace");

		TArray<FHitResult> StackHitResults;
		// Check objects above
		GetWorld()->LineTraceMultiByObjectType(StackHitResults,
			ActorLoc, ActorLoc + FVector(0.f, 0.f, MAX_STACK_HEIGHT), FCollisionObjectQueryParams::AllDynamicObjects,
			CollisionParams);

		GetWorld()->DebugDrawTraceTag = FName("StackTrace");

		UE_LOG(RobCoG, Warning, TEXT("Stack:"));
		// Iterate hit results
		for (const auto StackItr : StackHitResults)
		{
			if (StackableActors.Contains(StackItr.GetActor()))
			{
				// If actor is stackable cast to static mesh actor and add it to the stack
				AStaticMeshActor* CurrStackActor = Cast<AStaticMeshActor>(StackItr.GetActor());
				if (CurrStackActor)
				{
					CurrStackActor->GetStaticMeshComponent()->SetRenderCustomDepth(true);
					HighlightedStack.AddUnique(CurrStackActor);
				}
			}
		}
	}
}

// Check for stacking higlight
FORCEINLINE void ARWebCharacter::RemoveHighlights()
{	
	if (HighlightedRoot)
	{
		HighlightedRoot->GetStaticMeshComponent()->SetRenderCustomDepth(false);
		HighlightedRoot = nullptr;
	}

	if (HighlightedStack.Num() > 0)
	{
		for (const auto HighlightStackItr : HighlightedStack)
		{
			HighlightStackItr->GetStaticMeshComponent()->SetRenderCustomDepth(false);
		}
		HighlightedStack.Empty();
	}

	if (HighlightedTrayStack.Num() > 0)
	{
		for (const auto HighlightTrayStackItr : HighlightedTrayStack)
		{
			HighlightTrayStackItr->GetStaticMeshComponent()->SetRenderCustomDepth(false);
		}
		HighlightedTrayStack.Empty();
	}
}

// Create the cloned actor
void ARWebCharacter::CreateClones(AStaticMeshActor* RootActorToClone)
{
	// Remove old clone(s)
	ARWebCharacter::RemoveClones();

	// Spawn parameters for the cloned actor
	FActorSpawnParameters SpawnParam;
	SpawnParam.Template = RootActorToClone;
	SpawnParam.Owner = RootActorToClone->GetOwner();
	SpawnParam.Instigator = RootActorToClone->GetInstigator();
	SpawnParam.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Create the clone root from the actor
	CloneRoot = GetWorld()->SpawnActor<AStaticMeshActor>(
		RootActorToClone->GetClass(), RootActorToClone->GetActorTransform(), SpawnParam);
	// Set physics, collisions properties
	CloneRoot->GetStaticMeshComponent()->SetSimulatePhysics(false);
	CloneRoot->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CloneRoot->SetActorTransform(RootActorToClone->GetActorTransform());

	for (const auto AttachedStackItr : AttachedStack)
	{
		// Spawn parameters for the cloned stacked actor
		SpawnParam.Template = AttachedStackItr;
		SpawnParam.Owner = AttachedStackItr->GetOwner();
		SpawnParam.Instigator = AttachedStackItr->GetInstigator();
		// Create the current stack clone from the actor
		AStaticMeshActor* CurrStackClone = GetWorld()->SpawnActorAbsolute<AStaticMeshActor>(
			AttachedStackItr->GetClass(), AttachedStackItr->GetActorTransform(), SpawnParam);
		// Set physics, collisions properties
		CurrStackClone->GetStaticMeshComponent()->SetSimulatePhysics(false);
		CurrStackClone->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		// Attach clone to the root clone
		CurrStackClone->AttachToActor(CloneRoot, FAttachmentTransformRules::KeepWorldTransform);
		CurrStackClone->SetActorRelativeTransform(AttachedStackItr->GetRootComponent()->GetRelativeTransform());
		// Add clone to stack
		CloneStack.Add(CurrStackClone);
	}

	// Set cloned actor to invisible
	ShowClonedObjects(false);
	// Set default color to red
	ARWebCharacter::ColorClonedObjects(RedMat);
	bIsGreen = false;
	// Add clone to the ignored components of the trace
	TraceParams.ClearIgnoredComponents();
	TraceParams.AddIgnoredActor(CloneRoot);
}

// Hide/view cloned objects
FORCEINLINE void ARWebCharacter::ShowClonedObjects(bool bShow)
{
	CloneRoot->SetActorHiddenInGame(!bShow);
	for (const auto StackItr : CloneStack)
	{
		StackItr->SetActorHiddenInGame(!bShow);
	}
	bIsCloneVisible = bShow;
}

// Color the cloned objects
FORCEINLINE void ARWebCharacter::ColorClonedObjects(UMaterialInstanceConstant* Material)
{
	for (uint8 i = 0; i < CloneRoot->GetStaticMeshComponent()->GetMaterials().Num(); ++i)
	{
		CloneRoot->GetStaticMeshComponent()->SetMaterial(i, Material);
	}
	for (const auto StackItr : CloneStack)
	{
		for (uint8 i = 0; i < StackItr->GetStaticMeshComponent()->GetMaterials().Num(); ++i)
		{
			StackItr->GetStaticMeshComponent()->SetMaterial(i, Material);
		}
	}
}

// Remove clone
void ARWebCharacter::RemoveClones()
{
	if (CloneRoot)
	{
		CloneRoot->Destroy();
		for (const auto CloneStackItr : CloneStack)
		{
			CloneStackItr->Destroy();
		}
		CloneStack.Empty();
	}
	RotAxisIndex = 0;
}
