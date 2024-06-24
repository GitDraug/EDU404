// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Player/EDU_USER_PlayerController.h"
#include "Framework/Data/FlowLog.h"
#include "Framework/Data/EDU_USER_DataTypes.h"

AEDU_USER_PlayerController::AEDU_USER_PlayerController(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{ FLOW_LOG
}

void AEDU_USER_PlayerController::GetTerrainPosition(FVector& TargetPos, FVector& LastValidPos) const
{ FLOW_LOG
	// New trace
	FHitResult GroundTrace;
	FCollisionQueryParams CollisionParameters;
	
	FVector TraceStart = TargetPos;
	TraceStart.Z += 10'000.f;
	
	FVector TraceEnd = TargetPos;
	TraceEnd.Z -= 10'000.f;
	
	// Draw the debug sphere
	// DrawDebugSphere(GetWorld(), TracePos,  50.0f, 12, FColor::Green, false, 1.0f, 0, 2.0f);

	// Draw the debug line
	// DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Green, false, 1.0f, 0, 2.0f);
	
	if(GetWorld())
	{ // RTS_TRACE_CHANNEL_TERRAIN is defined in EDU_USER_StaticGameData 
		if(GetWorld()->LineTraceSingleByChannel(GroundTrace, TraceStart, TraceEnd, ECC_Visibility, CollisionParameters))
		{
			TargetPos = GroundTrace.ImpactPoint;
			// Trace was valid, save it.
			LastValidPos = GroundTrace.ImpactPoint;
		}
		else
		{ // If the tracing fails, because we're outside the map boundaries, we'll use the last cached position.
			TargetPos = LastValidPos;
		}
	}
}

void AEDU_USER_PlayerController::GetMouseCursorOnTerrain(FVector& TerrainPosition) const
{ FLOW_LOG
	FVector WorldLocation, WorldDirection;
	DeprojectMousePositionToWorld(WorldLocation, WorldDirection);

	FHitResult CursorWorldTrace;
	
	if(GetWorld()->LineTraceSingleByChannel(CursorWorldTrace, WorldLocation, WorldLocation+(WorldDirection*100'000.f), TRACE_CHANNEL_TERRAIN))
	{
		if(CursorWorldTrace.bBlockingHit)
		{
			TerrainPosition = CursorWorldTrace.Location;
		}
	}
}
