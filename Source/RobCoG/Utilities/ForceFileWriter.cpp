// Copyright 2017, Institute for Artificial Intelligence - University of Bremen

#include "ForceFileWriter.h"
#include "UObjectGlobals.h"

ForceFileWriter::ForceFileWriter()
{
}

ForceFileWriter::~ForceFileWriter()
{

}


bool ForceFileWriter::AppendFloatToFile(
	const float Value,
	const FString & Filename,
	FFileHelper::EEncodingOptions::Type EncodingOptions,
	IFileManager* FileManager)
{
	return FFileHelper::SaveStringToFile(";" + FString::SanitizeFloat(Value), *Filename, EncodingOptions, FileManager, FILEWRITE_Append);
}

bool ForceFileWriter::AppendStringToFile(
	const FString & Value,
	const FString & Filename,
	FFileHelper::EEncodingOptions::Type EncodingOptions,
	IFileManager* FileManager)
{
	return FFileHelper::SaveStringToFile(Value, *Filename, EncodingOptions, FileManager, FILEWRITE_Append);
}

bool ForceFileWriter::WriteGraspInfoMapToFile(const TMap<FString, FLogInfo> & ItemToGraspInfoMap,
	const FString & Filename,
	bool GraspSucceeded,
	FFileHelper::EEncodingOptions::Type EncodingOptions,
	IFileManager* FileManager)
{
	bool Success = true;
	for (auto Entry : ItemToGraspInfoMap)
	{
		FString ItemName = Entry.Key;
		FLogInfo LogInfo = Entry.Value;

		FString GraspType;
		UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EGraspType"), true);

		if (EnumPtr)
			GraspType = EnumPtr->GetDisplayNameTextByIndex(static_cast<int64>(LogInfo.GraspType)).ToString();
		else
			GraspType = "Error";

		Success = Success && AppendStringToFile("\nItemName:;" + ItemName, Filename);
		Success = Success && AppendStringToFile("\nGraspType:;" + GraspType, Filename);
		if (GraspSucceeded)
		{
			Success = Success && AppendStringToFile("\nStatus:;True", Filename);
		}
		else
		{
			Success = Success && AppendStringToFile("\nStatus:;False", Filename);
		}

		if (GraspSucceeded)
		{
			// Index
			Success = Success && AppendStringToFile("\nIndex - Distal:", Filename);
			for (auto ForceValue : LogInfo.OrientationHandForces.IndexDistal)
			{
				Success = Success && AppendFloatToFile(ForceValue, Filename);
			}

			Success = Success && AppendStringToFile("\nIndex - Intermediate:", Filename);
			for (auto ForceValue : LogInfo.OrientationHandForces.IndexIntermediate)
			{
				Success = Success && AppendFloatToFile(ForceValue, Filename);
			}

			Success = Success && AppendStringToFile("\nIndex - Proximal:", Filename);
			for (auto ForceValue : LogInfo.OrientationHandForces.IndexProximal)
			{
				Success = Success && AppendFloatToFile(ForceValue, Filename);
			}

			// Middle
			Success = Success && AppendStringToFile("\nMiddle - Distal:", Filename);
			for (auto ForceValue : LogInfo.OrientationHandForces.MiddleDistal)
			{
				Success = Success && AppendFloatToFile(ForceValue, Filename);
			}

			Success = Success && AppendStringToFile("\nMiddle - Intermediate:", Filename);
			for (auto ForceValue : LogInfo.OrientationHandForces.MiddleIntermediate)
			{
				Success = Success && AppendFloatToFile(ForceValue, Filename);
			}

			Success = Success && AppendStringToFile("\nMiddle - Proximal:", Filename);
			for (auto ForceValue : LogInfo.OrientationHandForces.MiddleProximal)
			{
				Success = Success && AppendFloatToFile(ForceValue, Filename);
			}

			// Ring
			Success = Success && AppendStringToFile("\nRing - Distal:", Filename);
			for (auto ForceValue : LogInfo.OrientationHandForces.RingDistal)
			{
				Success = Success && AppendFloatToFile(ForceValue, Filename);
			}

			Success = Success && AppendStringToFile("\nRing - Intermediate:", Filename);
			for (auto ForceValue : LogInfo.OrientationHandForces.RingIntermediate)
			{
				Success = Success && AppendFloatToFile(ForceValue, Filename);
			}

			Success = Success && AppendStringToFile("\nRing - Proximal:", Filename);
			for (auto ForceValue : LogInfo.OrientationHandForces.RingProximal)
			{
				Success = Success && AppendFloatToFile(ForceValue, Filename);
			}

			// Pinky
			Success = Success && AppendStringToFile("\nPinky - Distal:", Filename);
			for (auto ForceValue : LogInfo.OrientationHandForces.PinkyDistal)
			{
				Success = Success && AppendFloatToFile(ForceValue, Filename);
			}

			Success = Success && AppendStringToFile("\nPinky - Intermediate:", Filename);
			for (auto ForceValue : LogInfo.OrientationHandForces.PinkyIntermediate)
			{
				Success = Success && AppendFloatToFile(ForceValue, Filename);
			}

			Success = Success && AppendStringToFile("\nPinky - Proximal:", Filename);
			for (auto ForceValue : LogInfo.OrientationHandForces.PinkyProximal)
			{
				Success = Success && AppendFloatToFile(ForceValue, Filename);
			}

			// Thumb
			Success = Success && AppendStringToFile("\nThumb - Distal:", Filename);
			for (auto ForceValue : LogInfo.OrientationHandForces.ThumbDistal)
			{
				Success = Success && AppendFloatToFile(ForceValue, Filename);
			}

			Success = Success && AppendStringToFile("\nThumb - Intermediate:", Filename);
			for (auto ForceValue : LogInfo.OrientationHandForces.ThumbIntermediate)
			{
				Success = Success && AppendFloatToFile(ForceValue, Filename);
			}

			Success = Success && AppendStringToFile("\nThumb - Proximal:", Filename);
			for (auto ForceValue : LogInfo.OrientationHandForces.ThumbProximal)
			{
				Success = Success && AppendFloatToFile(ForceValue, Filename);
			}
		}


		Success = Success && AppendStringToFile("\n\n", Filename);

	}
	return Success;
}