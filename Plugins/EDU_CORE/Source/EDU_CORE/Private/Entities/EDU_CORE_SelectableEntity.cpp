// Fill out your copyright notice in the Description page of Project Settings.

// THIS
#include "Entities/EDU_CORE_SelectableEntity.h"

// CORE
#include "Framework/Data/FLOWLOGS/FLOWLOG_ENTITIES.h"
#include "Framework/Managers/GameModes/EDU_CORE_GameMode.h"
#include "Entities/Components/StatusComponent.h"
#include "Entities/Components/EngagementComponent.h"

// UE
#include "Net/UnrealNetwork.h"

//------------------------------------------------------------------------------
// Construction & Object Lifetime Management
//------------------------------------------------------------------------------

AEDU_CORE_SelectableEntity::AEDU_CORE_SelectableEntity(const FObjectInitializer& ObjectInitializer)
{ FLOW_LOG
	PrimaryActorTick.bCanEverTick = false;
}

void AEDU_CORE_SelectableEntity::BeginPlay()
{ FLOW_LOG
	Super::BeginPlay();

	// Initiate a pointer to StatusComponent (if we have one)
	StatusComponent = FindComponentByClass<UStatusComponent>();
	
	// Initiate a pointer to StatusComponent (if we have one)
	EngagementComponent = FindComponentByClass<UEngagementComponent>();
	
	// Create, save and Replicate the Unique ID of this entity.
	if(HasAuthority())
	{
		if(AActor* ActorPtr = Cast<AActor>(this))
		{
			if(AEDU_CORE_GameMode* GameMode = Cast<AEDU_CORE_GameMode>(GetWorld()->GetAuthGameMode()))
			{
				// Create a GUID for the actor, this will be the same on all clients and server, so they can communicate.
				ServerEntityID = FGuid::NewGuid();
        
				// Associate the GUID with the actor in the GameMode's Map.
				GameMode->AddToGuidActorMap(ServerEntityID, ActorPtr);

				/* Debug: Verify the map addition
				#if UE_EDITOR
				// Check individual additions
					UE_LOG(FLOWLOG_CATEGORY, Warning, TEXT("%s::%hs - Added %s to GuidActorMap with ServerEntityID: %s"), *GetClass()->GetName(), __FUNCTION__, *this->GetClass()->GetName(), *ServerEntityID.ToString());
					if (AActor* ActorPtrPtr = GameMode->GuidActorMap.FindRef(ServerEntityID))
					{
						UE_LOG(FLOWLOG_CATEGORY, Warning, TEXT("%s::%hs - Verification succeded: %s is stored in GuidActorMap"), *GetClass()->GetName(), __FUNCTION__, *ActorPtrPtr->GetClass()->GetName());
					}
					else
					{
						UE_LOG(FLOWLOG_CATEGORY, Error, TEXT("%s::%hs - Verification Failed! No actor found with ServerID: %s"), *GetClass()->GetName(), __FUNCTION__, *ServerEntityID.ToString());
					}

				// Check all entities in the Map
					for (const TPair<FGuid, AActor*>& Pair : GameMode->GuidActorMap)
					{
						const FGuid& Key = Pair.Key;
						AActor* Value = Pair.Value;

						// Log key-value pairs
						UE_LOG(FLOWLOG_CATEGORY, Warning, TEXT("Key: %s, Value: %s"), *Key.ToString(), Value ? *Value->GetClass()->GetName() : TEXT("NULL"));
					}
				#endif*/
			}
		}
	}
}

//------------------------------------------------------------------------------
// Networking
//------------------------------------------------------------------------------
void AEDU_CORE_SelectableEntity::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, ServerEntityID);
}

//------------------------------------------------------------------------------
// Functionality
//------------------------------------------------------------------------------
void AEDU_CORE_SelectableEntity::MouseHighlightActor()
{ FLOW_LOG
	bMouseHighlighted = true;
}

void AEDU_CORE_SelectableEntity::MouseUnHighlightActor()
{ FLOW_LOG
	bMouseHighlighted = false;
}

void AEDU_CORE_SelectableEntity::RectangleHighlightActor()
{ FLOW_LOG
	bRectangleHighlighted = true;
}

void AEDU_CORE_SelectableEntity::UnRectangleHighlightActor()
{ FLOW_LOG
	bRectangleHighlighted = false;
}

void AEDU_CORE_SelectableEntity::SelectActor()
{ FLOW_LOG
	bSelected = true;
}

void AEDU_CORE_SelectableEntity::UnSelectActor()
{ FLOW_LOG
	bSelected = false;
}