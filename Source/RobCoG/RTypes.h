#pragma once

/** Enum indicating the game type from the menu */
UENUM()
enum class ERMenuType : uint8
{
	MouseKeyboard = 0	UMETA(DisplayName = "MouseKeyboard"),
	VR					UMETA(DisplayName = "VR")
	
};


/** Enum indicating the log type */
UENUM()
enum class ERLogType : uint8
{
	Dynamic			UMETA(DisplayName = "Dynamic"),
	Static			UMETA(DisplayName = "Static"),
};

/** Enum indicating the hand parts */
UENUM()
enum class ERHandPart : uint8
{
	Thumb		UMETA(DisplayName = "Thumb"),
	Index		UMETA(DisplayName = "Index"),
	Middle		UMETA(DisplayName = "Middle"),
	Ring		UMETA(DisplayName = "Ring"),
	Pinky		UMETA(DisplayName = "Pinky"),
	Palm		UMETA(DisplayName = "Palm"),
	Wrist		UMETA(DisplayName = "Wrist"),
};

/** Enum indicating the grasp state */
UENUM()
enum class ERGraspState : uint8
{
	Opened		UMETA(DisplayName = "Opened"),
	Opening		UMETA(DisplayName = "Opening"),
	Closed		UMETA(DisplayName = "Closed"),
	Closing		UMETA(DisplayName = "Closing"),
	Free		UMETA(DisplayName = "Free"),
	Blocked		UMETA(DisplayName = "Blocked"),
	Attached	UMETA(DisplayName = "Attached"),
};
