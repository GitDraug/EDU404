// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Managers/GameModes/EDU_CORE_GameMode.h"

#include "AI/WayPoints/EDU_CORE_Waypoint.h"
#include "Entities/EDU_CORE_AbstractEntity.h"
#include "Entities/EDU_CORE_PhysicsEntity.h"
#include "Entities/EDU_CORE_MobileEntity.h"
#include "Framework/Data/FLOWLOGS/FLOWLOG_MANAGERS.h"

//------------------------------------------------------------------------------
// Construction & Object Lifetime Management
//------------------------------------------------------------------------------
AEDU_CORE_GameMode::AEDU_CORE_GameMode(const FObjectInitializer& ObjectInitializer)
{ FLOW_LOG
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.SetTickFunctionEnable(true);

	AvailableWaypointPool.Reserve(100);
	BusyWaypointPool.Reserve(100);
}

void AEDU_CORE_GameMode::Tick(float DeltaTime)
{ //FLOW_LOG
	Super::Tick(DeltaTime);
	//------------------------------------------------------------------------------
	// Debug
	//------------------------------------------------------------------------------
	
	if (DeltaTime < 0.016f)			DeltaTimeDisplayColor = FColor::Green;
	else if (DeltaTime < 0.033f)	DeltaTimeDisplayColor = FColor::Yellow;
	else							DeltaTimeDisplayColor = FColor::Red;
	// Add an on-screen debug message with the specified DisplayColor and DeltaTime information
	GEngine->AddOnScreenDebugMessage(12, GetWorld()->DeltaTimeSeconds, DeltaTimeDisplayColor, 
	FString::Printf(TEXT("FPS: %f"), 1.f/DeltaTime));
	
	/*------------------------------------------------------------------------------
	// Server-Side Aggregated Tick: AbstractEntityArray // Not in use.
	//------------------------------------------------------------------------------
	ParallelFor(AbstractEntityArray.Num(), [this, &DeltaTime ](int32 Index)
	{
		if(AbstractEntityArray[Index])
		{
			AbstractEntityArray[Index]->ServerAbstractCalc(DeltaTime);
		}
	});

	for (AEDU_CORE_AbstractEntity* AbstractEntity : AbstractEntityArray)
	{
		if (AbstractEntity)
		{
			AbstractEntity->ServerAbstractExec(DeltaTime);
		}
	}*/
	
	//------------------------------------------------------------------------------
	// Server-Side Aggregated Tick: PhysicsEntityArray
	//------------------------------------------------------------------------------	
	ParallelFor(PhysicsEntityArray.Num(), [this, &DeltaTime ](int32 Index)
	{
		if(PhysicsEntityArray[Index])
		{
			PhysicsEntityArray[Index]->ServerPhysicsCalc(DeltaTime);
		}
	});

	for (TObjectPtr<AEDU_CORE_PhysicsEntity> PhysicsEntity : PhysicsEntityArray)
	{
		if (PhysicsEntity)
		{
			PhysicsEntity->ServerPhysicsExec(DeltaTime);
		}
	}

	//------------------------------------------------------------------------------
	// Server-Side Aggregated Tick: MobileEntityArray
	//------------------------------------------------------------------------------
	
	ParallelFor(MobileEntityArray.Num(), [this, &DeltaTime ](int32 Index)
	{
		if(MobileEntityArray[Index])
		{
			MobileEntityArray[Index]->ServerMobileCalc(DeltaTime);
		}
	});
		
	for (AEDU_CORE_MobileEntity* MobileEntity : MobileEntityArray)
	{
		if (MobileEntity)
		{
			MobileEntity->ServerMobilesExec(DeltaTime);
		}
	}
}

void AEDU_CORE_GameMode::BeginPlay()
{
	Super::BeginPlay();
}

//------------------------------------------------------------------------------
// Public API: Aggregated Tick Arrays
//------------------------------------------------------------------------------
void AEDU_CORE_GameMode::AddToAbstractEntityArray(AEDU_CORE_AbstractEntity* AbstractEntity)
{ FLOW_LOG
	if (AbstractEntity)  // Check if the Entity is valid
	{
		// Add the entity to the TArray
		AbstractEntityArray.Add(AbstractEntity); // Adds the pointer to the array
		//UE_LOG(FLOWLOG_CATEGORY, Log, TEXT("Entity added: %s"), *AbstractEntity->GetName());
	}
}

void AEDU_CORE_GameMode::AddToPhysicsEntityArray(AEDU_CORE_PhysicsEntity* PhysicsEntity)
{ FLOW_LOG
	   
	if (PhysicsEntity)  // Check if the Entity is valid
	{
		// Add the entity to the TArray
		PhysicsEntityArray.Add(PhysicsEntity); // Adds the pointer to the array
		//UE_LOG(FLOWLOG_CATEGORY, Log, TEXT("Entity added: %s"), *PhysicsEntity->GetName());
	}
}

void AEDU_CORE_GameMode::AddToMobileEntityArray(AEDU_CORE_MobileEntity* MobileEntity)
{ FLOW_LOG
	if (MobileEntity)  // Check if the Entity is valid
	{
		// Add the entity to the TArray
		MobileEntityArray.Add(MobileEntity); // Adds the pointer to the array
		//UE_LOG(FLOWLOG_CATEGORY, Log, TEXT("Entity added: %s"), *AbstractEntity->GetName());
	}
}

//------------------------------------------------------------------------------
// Public API: Teams
//------------------------------------------------------------------------------

void AEDU_CORE_GameMode::AddActorToTeamArray(AActor* Actor, EEDU_CORE_Team TeamArray)
{ FLOW_LOG
	if (Actor)
	{
		switch (TeamArray) {
		case EEDU_CORE_Team::None:	Team_0_Array.AddUnique(Actor); // Default
			break;
		case EEDU_CORE_Team::Team_1:	Team_1_Array.AddUnique(Actor);
			break;
		case EEDU_CORE_Team::Team_2:	Team_2_Array.AddUnique(Actor);
			break;
		case EEDU_CORE_Team::Team_3:	Team_3_Array.AddUnique(Actor);
			break;
		case EEDU_CORE_Team::Team_4:	Team_4_Array.AddUnique(Actor);
			break;
		case EEDU_CORE_Team::Team_5:	Team_5_Array.AddUnique(Actor);
			break;
		case EEDU_CORE_Team::Team_6:	Team_6_Array.AddUnique(Actor);
			break;
		case EEDU_CORE_Team::Team_7:	Team_7_Array.AddUnique(Actor);
			break;
		case EEDU_CORE_Team::Team_8:	Team_8_Array.AddUnique(Actor);
			break;
		case EEDU_CORE_Team::Team_9:	Team_9_Array.AddUnique(Actor);
			break;
		case EEDU_CORE_Team::Team_10:	Team_10_Array.AddUnique(Actor);
			break;
		}
	}
}

void AEDU_CORE_GameMode::RemoveActorFromTeamArray(AActor* Actor, EEDU_CORE_Team TeamArray)
{ FLOW_LOG
	if (Actor)
	{
		switch (TeamArray) {
		case EEDU_CORE_Team::None:	if(Team_0_Array.Contains(Actor)) Team_0_Array.Remove(Actor);
			break;
		case EEDU_CORE_Team::Team_1:	if(Team_1_Array.Contains(Actor)) Team_1_Array.Remove(Actor);
			break;
		case EEDU_CORE_Team::Team_2:	if(Team_2_Array.Contains(Actor)) Team_2_Array.Remove(Actor);
			break;
		case EEDU_CORE_Team::Team_3:	if(Team_3_Array.Contains(Actor)) Team_3_Array.Remove(Actor);
			break;
		case EEDU_CORE_Team::Team_4:	if(Team_4_Array.Contains(Actor)) Team_4_Array.Remove(Actor);
			break;
		case EEDU_CORE_Team::Team_5:	if(Team_5_Array.Contains(Actor)) Team_5_Array.Remove(Actor);
			break;
		case EEDU_CORE_Team::Team_6:	if(Team_6_Array.Contains(Actor)) Team_6_Array.Remove(Actor);
			break;
		case EEDU_CORE_Team::Team_7:	if(Team_7_Array.Contains(Actor)) Team_7_Array.Remove(Actor);
			break;
		case EEDU_CORE_Team::Team_8:	if(Team_8_Array.Contains(Actor)) Team_8_Array.Remove(Actor);
			break;
		case EEDU_CORE_Team::Team_9:	if(Team_9_Array.Contains(Actor)) Team_9_Array.Remove(Actor);
			break;
		case EEDU_CORE_Team::Team_10:	if(Team_10_Array.Contains(Actor)) Team_10_Array.Remove(Actor);
			break;
		}
	}
}

TArray<TObjectPtr<AActor>> AEDU_CORE_GameMode::GetTeamArray(EEDU_CORE_Team TeamArray) const
{
	switch (TeamArray)
	{
		case EEDU_CORE_Team::None: return Team_0_Array;
		case EEDU_CORE_Team::Team_1: return Team_1_Array;
		case EEDU_CORE_Team::Team_2: return Team_2_Array;
		case EEDU_CORE_Team::Team_3: return Team_3_Array;
		case EEDU_CORE_Team::Team_4: return Team_4_Array;
		case EEDU_CORE_Team::Team_5: return Team_5_Array;
		case EEDU_CORE_Team::Team_6: return Team_6_Array;
		case EEDU_CORE_Team::Team_7: return Team_7_Array;
		case EEDU_CORE_Team::Team_8: return Team_8_Array;
		case EEDU_CORE_Team::Team_9: return Team_9_Array;
		case EEDU_CORE_Team::Team_10: return Team_10_Array;

		default: return TArray<TObjectPtr<AActor>>(); // Return an empty array in case of an invalid enum value
	}
}

//------------------------------------------------------------------------------
// Public API: Actor Management
//------------------------------------------------------------------------------

void AEDU_CORE_GameMode::AddToGuidActorMap(FGuid GUID,AActor* Actor)
{ FLOW_LOG
	GuidActorMap.Add(GUID, Actor);
	//UE_LOG(FLOWLOG_CATEGORY, Log, TEXT("Entity added: %s"), *Actor->GetName());
}

TObjectPtr<AActor> AEDU_CORE_GameMode::FindActorInMap(FGuid GUID) const
{ FLOW_LOG
	if(TObjectPtr<AActor> Actor = GuidActorMap.FindRef(GUID))
	{
		return Actor;
	}
	return nullptr;
}

//------------------------------------------------------------------------------
// Public API: Waypoint Pool Management
//------------------------------------------------------------------------------

TObjectPtr<AEDU_CORE_Waypoint> AEDU_CORE_GameMode::GetFreshWaypointFromPool(const EEDU_CORE_Team InTeam, const FVector& WorldLocation, const FRotator& WorldRotation)
{ FLOW_LOG
	if(AvailableWaypointPool.IsValidIndex(0))
	{
		// Retrieve the first element
		TObjectPtr<AEDU_CORE_Waypoint> Waypoint = AvailableWaypointPool[0];

		// Move it form Available to busy pool
		AvailableWaypointPool.RemoveAt(0, EAllowShrinking::No);
		BusyWaypointPool.AddUnique(Waypoint);

		// Initiate and return it
		Waypoint->InitiateWaypoint(InTeam, WorldLocation, WorldRotation);
		
		UE_LOG(FLOWLOG_CATEGORY, Display, TEXT("%s::%hs - Waypoint Fetched from Pool: %s"), *GetClass()->GetName(), __FUNCTION__, *Waypoint->GetClass()->GetName());
		return Waypoint;
	}
	
	// If the pool is empty, we'll spawn a new one
	FActorSpawnParameters SpawnParams;
	if(const TObjectPtr<AEDU_CORE_Waypoint> Waypoint = GetWorld()->SpawnActor<AEDU_CORE_Waypoint>(WaypointClass, WorldLocation, WorldRotation, SpawnParams))
	{
		// Move it to the busy pool
		BusyWaypointPool.AddUnique(Waypoint);

		// Initiate and return it
		Waypoint->InitiateWaypoint(InTeam, WorldLocation, WorldRotation);
		
		UE_LOG(FLOWLOG_CATEGORY, Display, TEXT("%s::%hs - No available Waypoints, new spawn: %s"), *GetClass()->GetName(), __FUNCTION__, *Waypoint->GetClass()->GetName());
		return Waypoint;
	}
	
	UE_LOG(FLOWLOG_CATEGORY, Display, TEXT("%s::%hs - Waypoint Spawning failed."), *GetClass()->GetName(), __FUNCTION__);
	return nullptr;
}

void AEDU_CORE_GameMode::ReturnWaypointToPool(const TObjectPtr<AEDU_CORE_Waypoint>& Waypoint)
{ FLOW_LOG
	if(Waypoint)
	{
		Waypoint->ResetWaypoint();
		
		// Remove it from the pool
		BusyWaypointPool.RemoveSingleSwap(Waypoint, EAllowShrinking::No);

		// Mark it as busy
		AvailableWaypointPool.AddUnique(Waypoint);
	}
}