// Fill out your copyright notice in the Description page of Project Settings.


#include "Entities/EDU_CORE_MobileEntity.h"
#include "AI/WayPoints/EDU_CORE_Waypoint.h"
#include "Framework/Data/FLOWLOGS/FLOWLOG_ENTITIES.h"
#include "Framework/Managers/GameModes/EDU_CORE_GameMode.h"

//------------------------------------------------------------------------------
// Aggregated Server tick
//------------------------------------------------------------------------------

void AEDU_CORE_MobileEntity::ServerMobileCalc(float DeltaTime)
{
	if(bAlign)
	{
		NavigationLocation = CachedWaypointLocation;
		
		if(FrameCounter > 100) // We count to 100 before doing anything, then we start over.
		{
			UpdateNavigation();
			AlignEndRotation = GetRotationToTargetPos(NavigationLocation);
			FrameCounter = 0;
		}
		AlignStartRotation = GetActorRotation();
		AlignEndRotation.Pitch = GetActorRotation().Pitch;
		AlignEndRotation.Roll = GetActorRotation().Roll;
		
		AlignInterpRotation = FMath::RInterpTo(AlignStartRotation, AlignEndRotation, DeltaTime, MaxTurnRate);
		float Difference = AlignInterpRotation.Yaw - AlignEndRotation.Yaw;
		if(Difference <= 0.1f && Difference >= -0.1f)
		{
			bAlign = false;
		}
	}
}

void AEDU_CORE_MobileEntity::ServerMobilesExec(float DeltaTime)
{	
	Align(DeltaTime);

	FrameCounter++;
	
	/*
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Cyan, 
	FString::Printf(TEXT("MyRotation: %f"),GetActorRotation().Yaw));
	
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Cyan, 
	FString::Printf(TEXT("AlignStartRotation: %f"),AlignStartRotation.Yaw));

	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Cyan, 
	FString::Printf(TEXT("AlignEndRotation: %f"),AlignEndRotation.Yaw));

	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Cyan, 
	FString::Printf(TEXT("bAlign: %d"),bAlign));
	*/
}

//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------

void AEDU_CORE_MobileEntity::BeginPlay()
{
	Super::BeginPlay();

	if(GetNetMode() != NM_Client)
	{
		if (AEDU_CORE_GameMode* GameMode = Cast<AEDU_CORE_GameMode>(GetWorld()->GetAuthGameMode()))
		{
			GameMode->AddToMobileEntityArray(this);
		}
	}
}

//------------------------------------------------------------------------------
// Commands
//------------------------------------------------------------------------------
void AEDU_CORE_MobileEntity::Command_NavigateTo(int32 FormationIndex, AEDU_CORE_Waypoint* WayPoint, bool QueueWaypoint)
{ FLOW_LOG
	if(!QueueWaypoint)
	{
		ClearAllWaypoints();
	}
	
	AddWaypoint(WayPoint, QueueWaypoint);
}

void AEDU_CORE_MobileEntity::Command_LookAt(int32 FormationIndex, AEDU_CORE_Waypoint* WayPoint, bool QueueWaypoint)
{ FLOW_LOG
	if(!QueueWaypoint)
	{
		ClearAllWaypoints();
		UpdateNavigation();
		bAlign = true;
	}
	
	AddWaypoint(WayPoint, QueueWaypoint);
	//FVector WaypointPosition = Waypoint->GetActorLocation();

}

//------------------------------------------------------------------------------
// Waypoint Utility
//------------------------------------------------------------------------------

void AEDU_CORE_MobileEntity::AddWaypoint(AEDU_CORE_Waypoint* Waypoint, int32 FormationIndex, bool Queue)
{ FLOW_LOG
	if(!Queue)
	{
		ClearAllWaypoints();
	}
	
	if (!WaypointArray.Contains(Waypoint) && WaypointArray.Num() < MaxWaypointCapacity)
	{
		// Add the waypoint to this entity's array, to retrieve info if needed.
		WaypointArray.AddUnique(Waypoint);
	}
}

void AEDU_CORE_MobileEntity::RemoveWaypoint(AEDU_CORE_Waypoint* Waypoint)
{ FLOW_LOG
	if (WaypointArray.Contains(Waypoint))
	{
		// Unsubscribe from updates, the waypoint will destroy itself when no one longer listens.
		Waypoint->RemoveActorFromWaypoint(this);
		WaypointArray.Remove(Waypoint);
		FLOW_LOG_WARNING("Removing Waypoint")
	}
}

void AEDU_CORE_MobileEntity::ClearAllWaypoints()
{ FLOW_LOG
	for(AEDU_CORE_Waypoint* Waypoint : WaypointArray)
	{
		if(Waypoint)
		{
			// Unsubscribe from updates, the waypoint will destroy itself when no one longer listens.
			Waypoint->RemoveActorFromWaypoint(this); 
			FLOW_LOG_WARNING("Clearing all Waypoints")
		}
	}
	WaypointArray.Reset();
}

//------------------------------------------------------------------------------
// Server Commands
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Simple Physics Movement
//------------------------------------------------------------------------------

void AEDU_CORE_MobileEntity::Accelerate()
{ FLOW_LOG
	if(CurrentSpeed < MaxSpeed)
	{
		CurrentSpeed += Acceleration;
	}
}

void AEDU_CORE_MobileEntity::Align(float DeltaTime)
{ // FLOW_LOG
	SetActorRotation(AlignInterpRotation);
}

//------------------------------------------------------------------------------
// Pure Calculations for Multithreading
//------------------------------------------------------------------------------

FRotator AEDU_CORE_MobileEntity::GetRotationToTargetPos(const FVector& TargetPos) const
{ //  FLOW_LOG
	// This is crazy fast, but will not return the shortest route.
	FVector LocalDirectionToTarget = TargetPos - this->GetActorLocation();
	return LocalDirectionToTarget.Rotation();
}

FRotator AEDU_CORE_MobileEntity::GetRotationToTargetActor(const AActor* TargetActor) const
{ FLOW_LOG
	// This is crazy fast, but will not return the shortest route.
	FVector LocalDirectionToTarget = TargetActor->GetActorLocation() - this->GetActorLocation();
	return LocalDirectionToTarget.Rotation();
}

void AEDU_CORE_MobileEntity::UpdateNavigation()
{ // FLOW_LOG
	if(WaypointArray.Num() > 0)
	{
		if(WaypointArray[0])
		{
			// Save Rotation To Target Position
			CachedWaypointLocation = WaypointArray[0]->GetActorLocation();

			// Set Navigation immediately
			NavigationLocation = CachedWaypointLocation;
		}
	}

}
