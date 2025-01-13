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
	// (Overloaded) Get the 3D rotation from Component TO a target position in 180 Degrees, 
	static FRotator GetRotationToTargetPos(const USceneComponent* SourceComponent, const FVector& TargetPos);

	
	// Get the 3D rotation TO a target Actor in 180 Degrees.
	static FRotator GetRotationToTargetActor(const AActor* SourceActor, const AActor* TargetActor);

	// (Overloaded) Get the 3D rotation from Component TO a target Actor in 180 Degrees.
	static FRotator GetRotationToTargetActor(const USceneComponent* SourceComponent, const AActor* TargetActor);
	
	// Get the 3D rotation FROM a target position in 180 Degrees.
	static FRotator GetRotationFromTargetPos(const AActor* SourceActor, const FVector& Target);

	// Get the 3D rotation FROM a target Actor in 180 Degrees.
	static FRotator GetRotationFromTargetActor(const AActor* SourceActor, const AActor* TargetActor);
	
	// Calculates the 1D (Yaw, Pitch or Roll) Rotational Distance between 2 rotations, with a tolerence for error. 
	static float CalculateRotationDistance (const float StartRotation, const float EndRotation, const float Tolerance);

	// Calculates the fastest 1D turn based on a Max Rotation Speed, Distance and a SlowRotationDistance.
	static float CalculateFastestTurnRate (const float RotationSpeed, const float RotationDistance, const float SlowRotationDistance = 0.f);
	
};

