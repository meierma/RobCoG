// Fill out your copyright notice in the Description page of Project Settings.

#include "RobCoG.h"
#include "RWebCharacter.h"

#define RIGHT_HAND FVector(20, 20, 30)
#define LEFT_HAND FVector(20, -20, 30)
#define BOTH_HANDS FVector(20, 0, 30)
#define IMPULSE FVector(1000)

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
	// Default rotation index
	RotAxisIndex = 0;

	// Create a CameraComponent
	CharacterCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("CharacterCamera"));
	// Set capsule component as parent for the camera
	CharacterCamera->SetupAttachment(GetRootComponent());
	// Position the camera
	CharacterCamera->RelativeLocation = FVector(10.0f, 0.0f, BaseEyeHeight);
	// Allow the pawn to control the camera rotation
	CharacterCamera->bUsePawnControlRotation = true;
	
	// Highlight materials
	HCGreenMat = ConstructorHelpers::FObjectFinderOptional<UMaterialInstanceConstant>(
		TEXT("MaterialInstanceConstant'/Game/Highlights/M_HighlightClone_Green_Inst.M_HighlightClone_Green_Inst'")).Get();
	HCRedMat = ConstructorHelpers::FObjectFinderOptional<UMaterialInstanceConstant>(
		TEXT("MaterialInstanceConstant'/Game/Highlights/M_HighlightClone_Red_Inst.M_HighlightClone_Red_Inst'")).Get();
}

// Called when the game starts or when spawned
void ARWebCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Standing height
	StandingHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	// Check if materials are present
	if (!HCGreenMat || !HCRedMat)
	{
		UE_LOG(RobCoG, Error, TEXT(" !! ARWebCharacter: No highlight materials found!"));
	}
	
	// Init items that can interact with the character
	ARWebCharacter::InitInteractiveItems();
}

// Called every frame
void ARWebCharacter::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	// Highlight interactive objects from the trace
	ARWebCharacter::TraceAndHighlight();
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

// Handles mouse click
void ARWebCharacter::OnSelect()
{
	if (HandToItem.Contains(SelectedHand))
	{
		// If selected hand is in use
		ARWebCharacter::ReleaseActor();
	}
	else if (HighlightedActor)
	{
		// If selected hand is free, and an actor is highlighted
		ARWebCharacter::InteractWithActor();
	}
	else
	{
		UE_LOG(RobCoG, Warning, TEXT(" ** ARWebCharacter: Invalid action!"));
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
		if (!HandToItem.Contains(ESelectedHand::Left) && !HandToItem.Contains(ESelectedHand::Right))
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
		if (!HandToItem.Contains(ESelectedHand::Both))
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

	// Remove old clone
	ARWebCharacter::RemoveHighlightClone();

	// Create new highlight clone if the currently selected hand is occupied 
	if (HandToItem.Contains(SelectedHand))
	{
		// Set new clone
		ARWebCharacter::SetHighlightClone(HandToItem[SelectedHand]);	
		
		// Remove interaction highlight is there is one currently
		if (HighlightedActor)
		{
			HighlightedActor->GetStaticMeshComponent()->SetRenderCustomDepth(false);
			HighlightedActor = nullptr;
		}
	}
}

// Switch rotation axis of the selected actor
void ARWebCharacter::SwitchRotAxis()
{
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
	if (HighlightClone)
	{
		switch (RotAxisIndex)
		{
		case 0: 		
			//HighlightClone->AddActorLocalRotation(FRotator(0.f, 10.f, 0.f)); // Yaw
			HighlightClone->AddActorWorldRotation(FRotator(0.f, 10.f, 0.f)); // Yaw
			break;
		case 1: 
			//HighlightClone->AddActorLocalRotation(FRotator(10.f, 0.f, 0.f)); // Pitch
			HighlightClone->AddActorWorldRotation(FRotator(10.f, 0.f, 0.f)); // Pitch
			break;
		case 2: 
			//HighlightClone->AddActorLocalRotation(FRotator(0.f, 0.f, 10.f)); // Roll
			HighlightClone->AddActorWorldRotation(FRotator(0.f, 0.f, 10.f)); // Roll
			break;
		default:
			return;
		}
		// Calculate new release plane offset
		ARWebCharacter::CalculatePlaneOffset();
	}
}

// Rotate selected actor into negative direction
void ARWebCharacter::RotateNeg()
{
	if (HighlightClone)
	{
		switch (RotAxisIndex)
		{
		case 0:
			//HighlightClone->AddActorLocalRotation(FRotator(0.f, -10.f, 0.f)); // Yaw
			HighlightClone->AddActorWorldRotation(FRotator(0.f, -10.f, 0.f)); // Yaw
			break;
		case 1:
			//HighlightClone->AddActorLocalRotation(FRotator(-10.f, 0.f, 0.f)); // Pitch
			HighlightClone->AddActorWorldRotation(FRotator(-10.f, 0.f, 0.f)); // Pitch
			break;
		case 2:
			//HighlightClone->AddActorLocalRotation(FRotator(0.f, 0.f, -10.f)); // Roll
			HighlightClone->AddActorWorldRotation(FRotator(0.f, 0.f, -10.f)); // Roll
			break;
		default:
			return;
		}
		// Calculate new release plane offset
		ARWebCharacter::CalculatePlaneOffset();
	}
}

// Interact with the highlighted item
bool ARWebCharacter::InteractWithActor()
{
	if (InteractiveActors.Contains(HighlightedActor))
	{
		// Get item interaction type
		EItemInteraction InteractionType = InteractiveActors[HighlightedActor];
		
		if (InteractionType == EItemInteraction::Pickable)
		{
			ARWebCharacter::CollectActor();
		}
		else if (InteractionType == EItemInteraction::Openable)
		{
			ARWebCharacter::ManipulateActor();			
		}
	}
	return true;
}

// Collect the highlighted item
void ARWebCharacter::CollectActor()
{
	// Add item to map (hand occupied)
	HandToItem.Add(SelectedHand, HighlightedActor);
	
	// Disable physics, collisions and attach item to the character
	HighlightedActor->GetStaticMeshComponent()->SetSimulatePhysics(false);
	HighlightedActor->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HighlightedActor->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
	
	// Set the relative  position of the attached item
	if (SelectedHand == ESelectedHand::Right)
	{
		HighlightedActor->SetActorRelativeLocation(RIGHT_HAND);
	}
	else if (SelectedHand == ESelectedHand::Left)
	{
		HighlightedActor->SetActorRelativeLocation(LEFT_HAND);
	}
	else if (SelectedHand == ESelectedHand::Both)
	{
		HighlightedActor->SetActorRelativeLocation(BOTH_HANDS);
	}

	// Set the cloned actor
	ARWebCharacter::SetHighlightClone(HighlightedActor);

	// Remove highlight
	HighlightedActor->GetStaticMeshComponent()->SetRenderCustomDepth(false);		
	HighlightedActor = nullptr;
}

// Release the highlighted item
void ARWebCharacter::ReleaseActor()
{
	// Release if the highlighted clone is green
	if (HitResult.IsValidBlockingHit() && bIsGreen)
	{
		// Currently selected actor
		AStaticMeshActor* CurrSelectedActor = HandToItem[SelectedHand];

		// Remove object from hand
		HandToItem.Remove(SelectedHand);

		// Detach component, fix collision and physics
		CurrSelectedActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		CurrSelectedActor->GetStaticMeshComponent()->SetSimulatePhysics(true);
		CurrSelectedActor->GetStaticMeshComponent()->bGenerateOverlapEvents = true;
		//CurrSelectedActor->GetStaticMeshComponent()->SetNotifyRigidBodyCollision(true);
		CurrSelectedActor->GetStaticMeshComponent()->SetCollisionProfileName("PhysicsActor");

		// Set actor location
		CurrSelectedActor->SetActorLocationAndRotation(
			HighlightClone->GetActorLocation(), HighlightClone->GetActorQuat());

		// Remove highlight clone
		ARWebCharacter::RemoveHighlightClone();
	}
	else
	{
		UE_LOG(RobCoG, Warning, TEXT(" ** ARWebCharacter: Cannot put %s down."), *HandToItem[SelectedHand]->GetName());
	}
}

// Open/close the highlighted item
void ARWebCharacter::ManipulateActor()
{
	if (InteractiveActorsToOpenedState.Contains(HighlightedActor))
	{
		if (InteractiveActorsToOpenedState[HighlightedActor])
		{
			// Close furniture
			HighlightedActor->GetStaticMeshComponent()->AddImpulse(-IMPULSE * HighlightedActor->GetActorForwardVector());
			// Set state to closed
			InteractiveActorsToOpenedState[HighlightedActor] = false;
		}
		else
		{
			// Open furniture
			HighlightedActor->GetStaticMeshComponent()->AddImpulse(IMPULSE * HighlightedActor->GetActorForwardVector());
			// Set state to closed
			InteractiveActorsToOpenedState[HighlightedActor] = true;
		}
	}
}

// Set the cloned actor
void ARWebCharacter::SetHighlightClone(AStaticMeshActor* ActorToClone)
{
	// Remove old clone
	ARWebCharacter::RemoveHighlightClone();
	
	// Spawn parameters for the cloned actor
	FActorSpawnParameters SpawnParam;
	SpawnParam.Name = FName("HighlightClone");
	SpawnParam.Template = ActorToClone;
	SpawnParam.Owner = ActorToClone->GetOwner();
	SpawnParam.Instigator = ActorToClone->GetInstigator();
	SpawnParam.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Create the clone from the actor
	HighlightClone = GetWorld()->SpawnActorAbsolute<AStaticMeshActor>(
		ActorToClone->GetClass(), ActorToClone->GetTransform(), SpawnParam);
	
	// Set default highlight material to red
	bIsGreen = false;
	HighlightClone->GetStaticMeshComponent()->SetMaterial(0, HCRedMat);

	// Plane collision offset
	ARWebCharacter::CalculatePlaneOffset();

	// Set physics, collisions properties
	HighlightClone->GetStaticMeshComponent()->SetSimulatePhysics(false);
	HighlightClone->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Set cloned actor to invisible
	HighlightClone->SetActorHiddenInGame(true);
}

// Remove clone
FORCEINLINE void ARWebCharacter::RemoveHighlightClone()
{
	if (HighlightClone)
	{
		HighlightClone->SetActorHiddenInGame(true);
		HighlightClone->SetActorEnableCollision(false);
		//HighlightClone->Destroy();
		RotAxisIndex = 0;
	}
}

// Trace and highlight manipulation
FORCEINLINE void ARWebCharacter::TraceAndHighlight()
{
	// Vectors to trace between 
	const FVector Start = CharacterCamera->GetComponentLocation();
	const FVector End = Start + CharacterCamera->GetForwardVector() * MaxGraspLength;
	// Line trace
	GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Pawn);

	if (HandToItem.Contains(SelectedHand))
	{
		// If hand is occupied highlight release area
		ARWebCharacter::HighlightRelease();
	}	
	else if (HitResult.IsValidBlockingHit() && HitResult.GetActor()->IsA(AStaticMeshActor::StaticClass()))
	{
		// If hand is free, and the hit actor is of type static mesh actor	
		ARWebCharacter::HighlightInteraction();
	}
	else if (HighlightedActor)
	{
		// If actor is highlighted, but the current one is not a static mesh actor
		HighlightedActor->GetStaticMeshComponent()->SetRenderCustomDepth(false);
		HighlightedActor = nullptr;
	}
}

// Highlight release area
FORCEINLINE void ARWebCharacter::HighlightRelease()
{
	if (HitResult.IsValidBlockingHit())
	{
		// Set HC actor location
		HighlightClone->SetActorLocation(HitResult.ImpactPoint + HCReleaseOffset);
		// Set to visible
		HighlightClone->SetActorHiddenInGame(false);

		if (HitResult.ImpactNormal.Z > 0.9 && !IsCollidingAtRelease())
		{	
			// Draw HC with green, if plane is horizontal and no collisions are present
			if (!bIsGreen)
			{
				HighlightClone->GetStaticMeshComponent()->SetMaterial(0, HCGreenMat);
				bIsGreen = true;
			}
		}
		else
		{
			// Draw cloned actor with red
			if (bIsGreen)
			{
				HighlightClone->GetStaticMeshComponent()->SetMaterial(0, HCRedMat);
				bIsGreen = false;
			}
		}
	}
	else
	{		
		HighlightClone->SetActorHiddenInGame(true);
	}
	return;
}

// Calculate plane offset
FORCEINLINE void ARWebCharacter::CalculatePlaneOffset()
{
	HCReleaseOffset = FVector(0.f, 0.f, HighlightClone->GetComponentsBoundingBox(true).GetExtent().Z + 0.5f);
}

// Check if object is colliding at release
FORCEINLINE bool ARWebCharacter::IsCollidingAtRelease()
{
	FHitResult OutHit;
	const FVector Loc = HighlightClone->GetActorLocation();
	// Sweep current location plus a small offset (apparently Start and End loc need to differ)
	const bool Hit = GetWorld()->SweepSingleByChannel(OutHit, Loc, Loc + FVector(0.f, 0.f, 0.1f),
		HighlightClone->GetActorQuat(),	ECollisionChannel::ECC_Pawn,
		HighlightClone->GetStaticMeshComponent()->GetCollisionShape(-2.5f));

	//if (Hit)
	//{
	//	DrawDebugPoint(GetWorld(), OutHit.ImpactPoint, 10.f, FColor::Blue, false, 0.5);

	//	if (HighlightClone->GetStaticMeshComponent()->GetCollisionShape().IsBox())
	//	{
	//		UE_LOG(RobCoG, Warning, TEXT("BOX Collision: %s"), *OutHit.GetActor()->GetName());

	//		DrawDebugBox(GetWorld(), OutHit.ImpactPoint, 
	//			HighlightClone->GetStaticMeshComponent()->GetCollisionShape(-2.5f).GetExtent(),
	//			FColor::Blue, false, 0.5f);

	//	}
	//	else if (HighlightClone->GetStaticMeshComponent()->GetCollisionShape().IsCapsule())
	//	{
	//		UE_LOG(RobCoG, Warning, TEXT("CAPSULE Collision: %s"), *OutHit.GetActor()->GetName());
	//		
	//		DrawDebugCapsule(GetWorld(), OutHit.ImpactPoint,
	//			HighlightClone->GetStaticMeshComponent()->GetCollisionShape().GetCapsuleHalfHeight(),
	//			HighlightClone->GetStaticMeshComponent()->GetCollisionShape().GetCapsuleRadius(),
	//			HighlightClone->GetStaticMeshComponent()->GetComponentQuat(),
	//			FColor::Blue, false, 0.5f);

	//	}
	//	else if (HighlightClone->GetStaticMeshComponent()->GetCollisionShape().IsSphere())
	//	{
	//		UE_LOG(RobCoG, Warning, TEXT("SPHERE Collision: %s"), *OutHit.GetActor()->GetName());
	//	}
	//	else
	//	{
	//		UE_LOG(RobCoG, Warning, TEXT("OTHER Collision: %s"), *OutHit.GetActor()->GetName());
	//	}
	//}
	//else
	//{
	//	UE_LOG(RobCoG, Warning, TEXT("NO Collision!"));
	//}
	
	return Hit;
}

// Highlight interaction
FORCEINLINE void ARWebCharacter::HighlightInteraction()
{
	// Cast actor to a static mesh
	AStaticMeshActor* HitActor = Cast<AStaticMeshActor>(HitResult.GetActor());
	// Check if it is an interactive actor
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

