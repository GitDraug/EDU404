#include "UtilityLibrary.h"


FRotator UtilityLibrary::GetRotationToTargetPos(const AActor* SourceActor, const FVector& TargetPos)
{ //  FLOW_LOG
	FVector LocalDirectionToTarget = TargetPos - SourceActor->GetActorLocation();
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

FRotator UtilityLibrary::GetRotationFromTargetActor(const AActor* SourceActor, const AActor* TargetActor)
{ //  FLOW_LOG
	FVector LocalDirectionToTarget = SourceActor->GetActorLocation() - TargetActor->GetActorLocation();
	return LocalDirectionToTarget.Rotation();
}