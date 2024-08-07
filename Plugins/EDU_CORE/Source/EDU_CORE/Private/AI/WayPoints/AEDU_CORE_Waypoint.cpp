// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/WayPoints/EDU_CORE_Waypoint.h"
#include "Framework/Data/FLOWLOGS/FLOWLOG_AI.h"
#include "Interfaces/EDU_CORE_CommandInterface.h"

//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------
AEDU_CORE_Waypoint::AEDU_CORE_Waypoint()
{ FLOW_LOG
	// We don't do normal Selection with Waypoints.
	bCanBeSelected = false;
}

void AEDU_CORE_Waypoint::BeginPlay()
{ FLOW_LOG
	Super::BeginPlay();
}

void AEDU_CORE_Waypoint::AddActorToWaypoint(AActor* Actor)
{ FLOW_LOG
	if(!ListenerArray.Contains(Actor))
	{
		ListenerArray.AddUnique(Actor);
		UE_LOG(FLOWLOG_CATEGORY, Warning, TEXT("%s::%hs - Adding entity: %s"), *GetClass()->GetName(), __FUNCTION__, *Actor->GetClass()->GetName());
	}
}

void AEDU_CORE_Waypoint::RemoveActorFromWaypoint(AActor* Actor)
{ FLOW_LOG
	if(ListenerArray.Contains(Actor))
	{
		ListenerArray.Remove(Actor);
		FLOW_LOG_WARNING("Removing Actor From Waypoint")
		
		if(ListenerArray.Num() == 0)
		{
			this->Destroy();
			FLOW_LOG_WARNING("Waypoint is empty, deleting")
		}

	}
}

void AEDU_CORE_Waypoint::NotifyListeners(bool Queue)
{ FLOW_LOG
	for (int32 Index = 0; Index < ListenerArray.Num(); ++Index)
	{
		if(AActor* Actor = ListenerArray[Index])
		{
			if(IEDU_CORE_CommandInterface* Listener = Cast<IEDU_CORE_CommandInterface>(Actor))
			{
				Listener->AddWaypoint(this, Index, Queue);
			}
		}
	}
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
// Network Functionality
//------------------------------------------------------------------------------