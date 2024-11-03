#pragma once



class UtilityLibrary
{
public:
//--------------------------------------------------------------------------------
//  Rotation Utility
//  These are all crazy fast, but will not return the shortest route.
//--------------------------------------------------------------------------------
	
	// Get the 3D rotation TO a target position in 180 Degrees.
	static FRotator GetRotationToTargetPos(const AActor* SourceActor, const FVector& Target);

	// Get the 3D rotation TO a target Actor in 180 Degrees.
	static FRotator GetRotationToTargetActor(const AActor* SourceActor, const AActor* TargetActor);

	// Get the 3D rotation FROM a target position in 180 Degrees.
	static FRotator GetRotationFromTargetPos(const AActor* SourceActor, const FVector& Target);

	// Get the 3D rotation FROM a target Actor in 180 Degrees.
	static FRotator GetRotationFromTargetActor(const AActor* SourceActor, const AActor* TargetActor);
	
};
