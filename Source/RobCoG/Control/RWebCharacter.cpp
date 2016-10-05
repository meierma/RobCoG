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
	TraceParams.bTraceAsyncScene = true;
	TraceParams.bReturnPhysicalMaterial = false;

	// Set the maximum grasping length (Length of the 'hands' of the character)
	MaxGraspLength = 150.f;

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

}

// Init interactive intems
void ARWebCharacter::InitInteractiveItems()
{
	UE_LOG(RobCoG, Log, TEXT(" ** Init interactive items: "));
	// Iterate through the static mesh actors and check tags to see which objects should be logged
	for (TActorIterator<AActor> ActItr(GetWorld()); ActItr; ++ActItr)
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
		AddMovementInput(Direction, Value);
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
		AddMovementInput(Direction, Value);
	}
}

