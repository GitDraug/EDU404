// Fill out your copyright notice in the Description page of Project Settings.


#include "EDU_CORE/Public/Entities/Waypoints/EDU_CORE_Waypoint.h"
#include "Interfaces/EDU_CORE_CommandInterface.h"
#include "Framework/Data/FLOWLOGS/FLOWLOG_AI.h"
#include "Framework/Managers/GameModes/EDU_CORE_GameMode.h"

#include "Net/UnrealNetwork.h"

//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------
AEDU_CORE_Waypoint::AEDU_CORE_Waypoint(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{ FLOW_LOG

	// Disable ticking
	PrimaryActorTick.bCanEverTick = false;
	
	// We don't do normal Selection with Waypoints.
	bCanBeRectangleSelected = false;
	
	// Enable replication
	SetReplicates(true);
	SetNetUpdateFrequency(10);
}

void AEDU_CORE_Waypoint::InitiateWaypoint(const EEDU_CORE_Team Team, const FVector& WorldLocation, const FRotator& WorldRotation)
{
	// Give it an ID that works across instances
	SetWaypointID(FGuid::NewGuid());

	SetTeam(Team);

	SetActorHiddenInGame(false);
	bAlwaysRelevant = true;

	SetActorLocationAndRotation(WorldLocation, WorldRotation);
	
	Rep_Location = WorldLocation;
	Rep_Rotation = WorldRotation;
}
//------------------------------------------------------------------------------
// Functionality
//------------------------------------------------------------------------------

void AEDU_CORE_Waypoint::AddActorToWaypoint(const TObjectPtr<AEDU_CORE_SelectableEntity>& Entity)
{ FLOW_LOG
	if(!ListenerArray.Contains(Entity))
	{
		ListenerArray.AddUnique(Entity);
		UE_LOG(FLOWLOG_CATEGORY, Warning, TEXT("%s::%hs - Adding entity: %s"), *GetClass()->GetName(), __FUNCTION__, *Entity->GetName());
		UE_LOG(FLOWLOG_CATEGORY, Warning, TEXT("%s::%hs - ListenerArray.Num(): %d"), *GetClass()->GetName(), __FUNCTION__, ListenerArray.Num());
	}
}

void AEDU_CORE_Waypoint::RemoveActorFromWaypoint(const TObjectPtr<AEDU_CORE_SelectableEntity>& Entity)
{ FLOW_LOG
	if(ListenerArray.Contains(Entity))
	{
		ListenerArray.RemoveSingle(Entity);
		FLOW_LOG_WARNING("Removing Actor From Waypoint")

		UE_LOG(FLOWLOG_CATEGORY, Warning, TEXT("%s::%hs - ListenerArray.Num(): %d"), *GetClass()->GetName(), __FUNCTION__, ListenerArray.Num());

		if(ListenerArray.Num() == 0)
		{
			if(HasAuthority())
			{
				if(TObjectPtr<AEDU_CORE_GameMode> GameMode = Cast<AEDU_CORE_GameMode>(GetWorld()->GetAuthGameMode()))
				{
					// Reset the waypoint
					GameMode->ReturnWaypointToPool(this);
				}
			}
			FLOW_LOG_WARNING("Waypoint is empty, Returning to pool")
		}
	}
}

void AEDU_CORE_Waypoint::SetWaypointID(FGuid GUID)
{ FLOW_LOG
	WaypointID = GUID;
}

int32 AEDU_CORE_Waypoint::GetFormationIndex(const TObjectPtr<AEDU_CORE_SelectableEntity>& Listener) const
{ FLOW_LOG
	if(ListenerArray.Contains(Listener))
	{
		// Find the index of the actor in the array
		int32 Index = ListenerArray.Find(Listener);

		if (Index != INDEX_NONE)
		{
			UE_LOG(FLOWLOG_CATEGORY, Log, TEXT("Listener found in array at index: %d"), Index);
			return Index;
		}
	}
	UE_LOG(FLOWLOG_CATEGORY, Log, TEXT("Listener not found in array."));
	return 0;
}

void AEDU_CORE_Waypoint::NotifyListeners()
{ FLOW_LOG
	for (int32 Index = 0; Index < ListenerArray.Num(); ++Index)
	{
		if(AActor* Actor = ListenerArray[Index])
		{
			if(IEDU_CORE_CommandInterface* Listener = Cast<IEDU_CORE_CommandInterface>(Actor))
			{
				Params.WaypointPtr = this;
				Params.WaypointPosition = GetActorLocation();
				Params.WaypointRotation = GetActorRotation();
				Params.WaypointRightVector = GetActorRightVector();
				Params.WaypointForwardVector = GetActorForwardVector();
				
				Listener->AddWaypoint(Params);
			}
		}
	}
}

void AEDU_CORE_Waypoint::ResetWaypoint()
{ FLOW_LOG
	SetActorHiddenInGame(true);
	Owner = nullptr;
	ListenerArray.Reset();
	
	// Reset FGuid to default (all zeroes)
	FGuid DefaultGuid;
	WaypointID = DefaultGuid;

	// Reset Params with default constructor
	FWaypointParams NewParams;
	Params = NewParams;
	
	bAlwaysRelevant = false;
}

//------------------------------------------------------------------------------
// WaypointSelection
//------------------------------------------------------------------------------

void AEDU_CORE_Waypoint::RectangleHighlightActor()
{
	//Super::RectangleHighlightActor();
}

void AEDU_CORE_Waypoint::UnRectangleHighlightActor()
{
	//Super::UnRectangleHighlightActor();
}

//------------------------------------------------------------------------------
// Replication
//------------------------------------------------------------------------------

void AEDU_CORE_Waypoint::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AEDU_CORE_Waypoint, Rep_Location);
	DOREPLIFETIME(AEDU_CORE_Waypoint, Rep_Rotation);
}

void AEDU_CORE_Waypoint::OnRep_ReplicatedLocation()
{ FLOW_LOG
	SetActorLocation(Rep_Location);
}

void AEDU_CORE_Waypoint::OnRep_ReplicatedRotation()
{ FLOW_LOG
	SetActorRotation(Rep_Rotation);
}
