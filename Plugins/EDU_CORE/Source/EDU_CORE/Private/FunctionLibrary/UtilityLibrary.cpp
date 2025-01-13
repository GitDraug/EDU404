#include "FunctionLibrary/UtilityLibrary.h"

//--------------------------------------------------------------------------------
//  Rotation Utility
//  These are all crazy fast, but will not return the shortest route.
//--------------------------------------------------------------------------------

FRotator UtilityLibrary::GetRotationToTargetPos(const AActor* SourceActor, const FVector& TargetPos)
{ //  FLOW_LOG
	FVector LocalDirectionToTarget = TargetPos - SourceActor->GetActorLocation();
	return LocalDirectionToTarget.Rotation();
}

FRotator UtilityLibrary::GetRotationToTargetPos(const USceneComponent* SourceComponent, const FVector& TargetPos)
{ //  FLOW_LOG
	FVector LocalDirectionToTarget = TargetPos - SourceComponent->GetComponentLocation();
	return LocalDirectionToTarget.Rotation();
}

FRotator UtilityLibrary::GetRotationFromTargetPos(const AActor* SourceActor, const FVector& TargetPos)
{ //  FLOW_LOG
	FVector LocalDirectionToTarget = SourceActor->GetActorLocation() - TargetPos;
	return LocalDirectionToTarget.Rotation();
}

FRotator UtilityLibrary::GetRotationToTargetActor(const AActor* SourceActor, const AActor* TargetActor)
{ //  FLOW_LOG
	FVector LocalDirectionToTarget = TargetActor->GetActorLocation() - SourceActor->GetActorLocation();
	return LocalDirectionToTarget.Rotation();
}

FRotator UtilityLibrary::GetRotationToTargetActor(const USceneComponent* SourceComponent, const AActor* TargetActor)
{ //  FLOW_LOG
	FVector LocalDirectionToTarget = TargetActor->GetActorLocation() - SourceComponent->GetComponentLocation();
	return LocalDirectionToTarget.Rotation();
}

FRotator UtilityLibrary::GetRotationFromTargetActor(const AActor* SourceActor, const AActor* TargetActor)
{ //  FLOW_LOG
	FVector LocalDirectionToTarget = SourceActor->GetActorLocation() - TargetActor->GetActorLocation();
	return LocalDirectionToTarget.Rotation();
}

float UtilityLibrary::CalculateRotationDistance(float StartRotation, float EndRotation, float Tolerance)
{ //  FLOW_LOG
	
	// Calculate rotational distance
	float RotationDistance = EndRotation - StartRotation;

	/*-----------------------------------------------------------------------------------------------
	  Normalize the raw rotational distance to the range [-180, 180). This ensures that the
	  resulting distance is always the shortest possible rotation path.

	  The formula works by first shifting the range to [0, 360) (adding 180 degrees before modulo),
	  and then subtracting 180 to bring it back into [-180, 180).
	-----------------------------------------------------------------------------------------------*/
	RotationDistance = FMath::Fmod(RotationDistance + 180.0f, 360.0f);
	
	// Handle the case where Fmod can return negative results by adding 360.
	// This ensures that the range remains valid in [0, 360) after modulo.
	if(RotationDistance < 0.0f){ RotationDistance += 360.0f; }

	// Subtract 180 to normalize the range to [-180, 180).
	RotationDistance -= 180.0f;

	// Check if the calculated rotation distance is within the given tolerance.
	if (FMath::Abs(RotationDistance) < Tolerance)
	{
		return 0.f; // The rotation distance is too small to matter.
	}

	// Return the normalized rotational distance.
	return RotationDistance;
}

float UtilityLibrary::CalculateFastestTurnRate(const float RotationSpeed, const float RotationDifference, const float SlowRotationDistance)
{
	const float AsyncedRotationSpeed = RotationSpeed; // * FixedDeltaTick;

	// Branchless optimized version
	const float TurnRate = AsyncedRotationSpeed * FMath::Min(FMath::Abs(RotationDifference) / SlowRotationDistance, 1.0f);
	const float FastestTurnRate = FMath::Sign(RotationDifference) * TurnRate;
	
	return FastestTurnRate;
	
	//-----------------------------------------------
		/* Legacy version for readability
	//-----------------------------------------------
	float TurnRate = AsyncedRotationSpeed; // Default speed, unless below SlowRotationDistance
	
	// How fast should we turn?
	if(FMath::Abs(RotationDifference) < SlowRotationDistance)
	{
		// When within the slow rotation distance, scale down the Turn Rate based on the RotationDifference
		TurnRate = (RotationDifference / SlowRotationDistance) * AsyncedRotationSpeed;
	}
	
	// Is right or left turn fastest?
	if (RotationDifference < 0) 
	{
		FastestTurnRate = -FMath::Abs(TurnRate); // Go Left
	}
	else
	{
		FastestTurnRate = FMath::Abs(TurnRate); // Go Right
	}
	-----------------------------------------------*/
	//-----------------------------------------------

}


