// Fill out your copyright notice in the Description page of Project Settings.


#include "Entities/EDU_CORE_MobileEntity.h"
#include "AI/WayPoints/EDU_CORE_Waypoint.h"
#include "AI/WayPoints/EDU_CORE_Waypoint_Move.h"
#include "Framework/Data/FLOWLOGS/FLOWLOG_ENTITIES.h"
#include "Framework/Managers/GameModes/EDU_CORE_GameMode.h"

//------------------------------------------------------------------------------
// Aggregated Server tick
//------------------------------------------------------------------------------

void AEDU_CORE_MobileEntity::ServerMobileCalc(float DeltaTime)
{

}

void AEDU_CORE_MobileEntity::ServerMobilesExec(float DeltaTime)
{	
	Align(DeltaTime);

	FrameCounter++;
	
	if(bAlign)
	{
		NavigationLocation = CachedWaypointLocation;
		
		if(FrameCounter > 100)
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
// Interface Commands
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

void AEDU_CORE_MobileEntity::Command_NavigateTo(AActor* Commander, const FVector& Position, const FRotator& Rotation, bool Que, float Delay)
{ // FLOW_LOG
	// Clear earlier waypoints unless Qued.
	if(!Que) { ClearAllWaypoints(); }

	// Delay the Spawn of the waypoint
	// FTimerHandle CommandDelay;

	// ;

		FTimerHandle CommandDelay;
	// Use a lambda function to capture parameters and call OnTimerTick
	GetWorld()->GetTimerManager().SetTimer(	CommandDelay,	// FTimerHandle
		[this, Commander, Position, Rotation] // Capture list for Lambda
		{ AEDU_CORE_Waypoint_Move* Waypoint = SpawnWaypoint(Commander, Position, Rotation); },// Function to fire
		Delay, // Delay between loops
		false  // Looping
		// Delay / 60.f // First Delay
	);
	
	// NavigationLocation = WaypointPosition;
	bAlign = true;
			
	// Send RPC to server that the unit should move
	APlayerController* LocalPlayerController = GetWorld()->GetFirstPlayerController();
}

//------------------------------------------------------------------------------
// Waypoint Utility
//------------------------------------------------------------------------------

void AEDU_CORE_MobileEntity::SaveWaypoint(AEDU_CORE_Waypoint* Waypoint)
{ FLOW_LOG
	if (!WaypointArray.Contains(Waypoint) && WaypointArray.Num() < MaxWaypointCapacity)
	{
		WaypointArray.AddUnique(Waypoint);
		FLOW_LOG_WARNING("Adding Waypoint")
	}
}

void AEDU_CORE_MobileEntity::DeleteWaypoint(AEDU_CORE_Waypoint* Waypoint)
{ FLOW_LOG
	if (WaypointArray.Contains(Waypoint))
	{
		WaypointArray.Remove(Waypoint);
		FLOW_LOG_WARNING("Removing Waypoint")
		Waypoint->Destroy();
	}
}

void AEDU_CORE_MobileEntity::ClearAllWaypoints()
{ FLOW_LOG
	for(AEDU_CORE_Waypoint* Waypoint : WaypointArray)
	{
		if(Waypoint)
		{
			FLOW_LOG_WARNING("Clearing all Waypoints")
			Waypoint->Destroy();
		}
	}
	WaypointArray.Reset();
}

AEDU_CORE_Waypoint_Move* AEDU_CORE_MobileEntity::SpawnWaypoint(AActor* Camera, const FVector& Position, const FRotator& Rotation)
{ FLOW_LOG
	// Define spawn parameters
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Camera;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	
	// Spawn the actor
	if(AEDU_CORE_Waypoint_Move* Waypoint = GetWorld()->SpawnActor<AEDU_CORE_Waypoint_Move>(WaypointClass, Position, Rotation, SpawnParams))
	{
		FVector WaypointPosition = Waypoint->GetActorLocation();
		
		// Add the Waypoint to this entity's Array
		SaveWaypoint(Waypoint);
		UpdateNavigation();
		
		return Waypoint;
	}
	return nullptr;
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
