// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Managers/GameModes/EDU_CORE_GameMode.h"

// CORE
#include "AI/WayPoints/EDU_CORE_Waypoint.h"

#include "Entities/EDU_CORE_AbstractEntity.h"
#include "Entities/EDU_CORE_PhysicsEntity.h"
#include "Entities/EDU_CORE_MobileEntity.h"

#include "Entities/Components/EDU_CORE_SenseComponent.h"
#include "Entities/Components/EDU_CORE_StatusComponent.h"

#include "Framework/Data/FLOWLOGS/FLOWLOG_MANAGERS.h"
#include "Framework/Pawns/EDU_CORE_C2_Camera.h"

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
	else if (DeltaTime < 0.02f)		DeltaTimeDisplayColor = FColor::Yellow;
	else 							DeltaTimeDisplayColor = FColor::Red;
	// Add an on-screen debug message with the specified DisplayColor and DeltaTime information
	GEngine->AddOnScreenDebugMessage(12, GetWorld()->DeltaTimeSeconds, DeltaTimeDisplayColor, 
	FString::Printf(TEXT("FPS: %f"), 1.f/DeltaTime));

	FPSCounter++;
	FPSTotal += 1.f/DeltaTime;
	
	GEngine->AddOnScreenDebugMessage(13, GetWorld()->DeltaTimeSeconds, DeltaTimeDisplayColor, 
	FString::Printf(TEXT("FPS: %f"), FPSTotal / FPSCounter));
	
	if(FPSCounter > 500)
	{
		FPSCounter = 0;
		FPSTotal = 0;
	}
	
	//------------------------------------------------------------------------------
	// Normalized Time
	//	<!> Because of our Async Physics Tick, we want to normalize our internal
	//		clock around the physics tick. As the simulation slows down, so does
	//		the clock. In turn, simulation never goes above 50FPS.
	//------------------------------------------------------------------------------

	constexpr float AsyncPhysicsTick = 0.02f;
	CurrentTime += FMath::Min(DeltaTime, AsyncPhysicsTick) / AsyncPhysicsTick / 50.f;
	
	// GEngine->AddOnScreenDebugMessage(20, GetWorld()->DeltaTimeSeconds, DeltaTimeDisplayColor, 
	// FString::Printf(TEXT("Real Clock: %f"), GetWorld()->GetTimeSeconds()));
	// GEngine->AddOnScreenDebugMessage(21, GetWorld()->DeltaTimeSeconds, DeltaTimeDisplayColor, 
	// FString::Printf(TEXT("Sycned Clock: %f"), CurrentTime ));

	// GEngine->AddOnScreenDebugMessage(22, GetWorld()->DeltaTimeSeconds, DeltaTimeDisplayColor, 
	// FString::Printf(TEXT("Sycned Clock / Second: %f"), GetWorld()->GetTimeSeconds() / CurrentTime));
	
	//------------------------------------------------------------------------------
	// Server-Side Aggregated Tick > AbstractEntityArray // Not in use.
	//------------------------------------------------------------------------------

	
	//------------------------------------------------------------------------------
	// Server-Side Aggregated Tick > PhysicsEntityArray
	//------------------------------------------------------------------------------
	
		ParallelFor(PhysicsEntityArray.Num(), [this, DeltaTime](int32 Index)
		{
			if(AEDU_CORE_PhysicsEntity* Entity = PhysicsEntityArray[Index])  // Single access with conditional initialization
			{
				Entity->ServerPhysicsCalc(DeltaTime);
			}
		});

		// Sequential execution of ServerPhysicsExec on each entity
		for(TObjectPtr<AEDU_CORE_PhysicsEntity> PhysicsEntity : PhysicsEntityArray)
		{
			if (PhysicsEntity)  // Check for valid entity
			{
				PhysicsEntity->ServerPhysicsExec(DeltaTime);
			}
		}

	//------------------------------------------------------------------------------
	// Server-Side Aggregated Tick > MobileEntityArray
	//------------------------------------------------------------------------------
	
		ParallelFor(MobileEntityArray.Num(), [this, DeltaTime](int32 Index)
		{
			if(AEDU_CORE_MobileEntity* MobileEntity = MobileEntityArray[Index])  // Checking for validity only once
			{
				MobileEntity->ServerMobileCalc(DeltaTime, CurrentBatchIndex_10);
			}
		});

		// Sequential execution of ServerMobilesExec on each entity
		for(AEDU_CORE_MobileEntity* MobileEntity : MobileEntityArray)
		{
			if (MobileEntity)
			{
				MobileEntity->ServerMobileExec(DeltaTime, CurrentBatchIndex_10);
			}
		}

		// If more than 1 second has passed since the last cycle, start a new batch cycle
		if (CurrentTime - LastMobileBatchTime >= 0.75f)
		{
			if (MobileBatchIndex == 0)
			{
				LastMobileBatchTime = CurrentTime;
			}
			constexpr int32 BatchSize = 40;
	    		
			// Calculate the end index for the current batch
			const int32 ThisTickEndIndex = FMath::Min(MobileBatchIndex + BatchSize, MobileEntityArray.Num());
		        
			//* Parallel batch processing of ServerMobileBatchedCalc
			ParallelFor(ThisTickEndIndex - MobileBatchIndex, [this, StartIndex = MobileBatchIndex](int32 LocalIndex)
			{
				int32 EntityIndex = StartIndex + LocalIndex;
				if (AEDU_CORE_MobileEntity* MobileEntity = MobileEntityArray[EntityIndex])
				{
					MobileEntity->ServerMobileBatchedCalc();
				}
			});//*/

			/*/ Singe Core Sequential ServerMobileBatchedCalc
			for (int32 EntityIndex = MobileBatchIndex; EntityIndex < ThisTickEndIndex; ++EntityIndex)
			{
				if (AEDU_CORE_MobileEntity* MobileEntity = MobileEntityArray[EntityIndex])
				{
					MobileEntity->ServerMobileBatchedCalc();
				}
			}//*/

			// Update the batch index and last processing time to begin a new cycle
			MobileBatchIndex = ThisTickEndIndex;
		}

		// If we've completed processing the entire array, reset the batch index to 0
		if (MobileBatchIndex >= MobileEntityArray.Num())
		{
			MobileBatchIndex = 0;
		}

	//------------------------------------------------------------------------------
	// Server-Side Aggregated Tick > SightComponentArray
	//------------------------------------------------------------------------------
		
		// Define batch size
		//  GEngine->AddOnScreenDebugMessage(14, GetWorld()->DeltaTimeSeconds, DeltaTimeDisplayColor, 
		// 								 FString::Printf(TEXT("LastStatusTime: %f"), LastStatusTime));

		// Define batch size
		//  GEngine->AddOnScreenDebugMessage(13, GetWorld()->DeltaTimeSeconds, DeltaTimeDisplayColor, 
		// 								 FString::Printf(TEXT("LastSightTime: %f"), LastSightTime));
		
		// If more than 1 second has passed since the last cycle, start a new batch cycle
		if (CurrentTime - LastSightTime >= 1.0f)
		{
			if (SightBatchIndex == 0)
			{
				LastSightTime = CurrentTime;
			}
			constexpr int32 BatchSize = 20;
	    		
			// Calculate the end index for the current batch
			const int32 ThisTickEndIndex = FMath::Min(SightBatchIndex + BatchSize, SightComponentArray.Num());
		        
			// Parallel batch processing of ServerSightCalc
			ParallelFor(ThisTickEndIndex - SightBatchIndex, [this, DeltaTime, StartIndex = SightBatchIndex](int32 LocalIndex)
			{
				int32 ComponentIndex = StartIndex + LocalIndex;
				//if (SightComponentArray.IsValidIndex(ComponentIndex)) // Ensure index validity
		            //{
		                if (UEDU_CORE_SenseComponent* Component = SightComponentArray[ComponentIndex])
		                {
		                    Component->ServerSightCalc(DeltaTime);
		                }
		           // }
			});

			// Sequential batch processing of ServerSightExec
			for (int32 ComponentIndex = SightBatchIndex; ComponentIndex < ThisTickEndIndex; ++ComponentIndex)
			{
				//if (SightComponentArray.IsValidIndex(ComponentIndex)) // Ensure index validity
				//{
					if (UEDU_CORE_SenseComponent* Component = SightComponentArray[ComponentIndex])
		                {
		                    Component->ServerSightExec(DeltaTime);
		                }
				//}
			}

			// Update the batch index and last processing time to begin a new cycle
			SightBatchIndex = ThisTickEndIndex;
		}

		// If we've completed processing the entire array, reset the batch index to 0
		if (SightBatchIndex >= SightComponentArray.Num())
		{
		    SightBatchIndex = 0;
		}
		

	//------------------------------------------------------------------------------
	// Server-Side Aggregated Tick > StatusComponentArray
	//------------------------------------------------------------------------------
	
		// If more than 1 second has passed since the last cycle, start a new batch cycle
		if (CurrentTime - LastStatusTime >= 1.0f)
		    {
	    		//  GEngine->AddOnScreenDebugMessage(13, GetWorld()->DeltaTimeSeconds, DeltaTimeDisplayColor, 
				// 								 FString::Printf(TEXT("Tracing!")));
	    		
	    		if (StatusBatchIndex == 0)
	    		{
	    			LastStatusTime = CurrentTime;
	    		}
			    constexpr int32 BatchSize = 20;
	    		
			    // Calculate the end index for the current batch
		        const int32 ThisTickEndIndex = FMath::Min(StatusBatchIndex + BatchSize, StatusComponentArray.Num());
		        
		        // Parallel batch processing of ServerStatusCalc
		        ParallelFor(ThisTickEndIndex - StatusBatchIndex, [this, DeltaTime, StartIndex = StatusBatchIndex](int32 LocalIndex)
		        {
		            int32 ComponentIndex = StartIndex + LocalIndex;
		            //if (StatusComponentArray.IsValidIndex(ComponentIndex)) // Ensure index validity
		            //{
		                if (UEDU_CORE_StatusComponent* Component = StatusComponentArray[ComponentIndex])
		                {
		                    Component->ServerStatusCalc(DeltaTime);
		                }
		           // }
		        });

		        // Sequential batch processing of ServerStatusExec
		        for (int32 ComponentIndex = StatusBatchIndex; ComponentIndex < ThisTickEndIndex; ++ComponentIndex)
		        {
		          //  if (StatusComponentArray.IsValidIndex(ComponentIndex)) // Ensure index validity
		           // {
		                if (UEDU_CORE_StatusComponent* Component = StatusComponentArray[ComponentIndex])
		                {
		                    Component->ServerStatusExec(DeltaTime);
		                }
		          //  }
		        }

		        // Update the batch index and last processing time to begin a new cycle
		        StatusBatchIndex = ThisTickEndIndex;
		}

		// If we've completed processing the entire array, reset the batch index to 0
		if (StatusBatchIndex >= StatusComponentArray.Num())
		{
		    StatusBatchIndex = 0;
		}
	
	//------------------------------------------------------------------------------
	// Reset batch Index
	//------------------------------------------------------------------------------

		// Original code
		// CurrentBatchIndex = (CurrentBatchIndex < 10) ? (CurrentBatchIndex + 1) : 1;

		// Refactored version (faster)
		CurrentBatchIndex_10 = (CurrentBatchIndex_10 % 10) + 1;
		CurrentBatchIndex_100 = (CurrentBatchIndex_100 % 100) + 1;
}

void AEDU_CORE_GameMode::BeginPlay()
{
	Super::BeginPlay();
}

//------------------------------------------------------------------------------
// Public API > Aggregated Tick Arrays
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
	if(MobileEntity)  // Check if the entity is valid
	{
		// Check if the entity is not already in the array and add it if unique
		int32 Index = MobileEntityArray.AddUnique(MobileEntity);

		if (Index != INDEX_NONE) // Verify the entity was added or already exists
		{
			// Calculate the BatchIndex based on Index size
			if(Index < 100)
			{
				 BatchIndex_10 = (Index / 10) + 1;
			}
			else
			{
				BatchIndex_10 = (Index / 100) + 1;
			}
			
			// Call UpdateBatchIndex on the newly added entity, passing its BatchIndex rather than ArrayIndex
			MobileEntity->UpdateBatchIndex(BatchIndex_10);
		}
	}
}

void AEDU_CORE_GameMode::AddToSightComponentArray(UEDU_CORE_SenseComponent* SightComponent)
{ FLOW_LOG
	if(SightComponent)  // Check if the entity is valid
	{
		// Check if the entity is not already in the array and add it if unique
		int32 Index = SightComponentArray.AddUnique(SightComponent);

		if (Index != INDEX_NONE) // Verify the entity was added or already exists
		{
			// Calculate the BatchIndex based on Index size
			if(Index < 10) // <!> SightComponents Work a lot slower than other components. 
			{
				BatchIndex_100 = (Index / 1) + 1;
			}
			else
			{
				BatchIndex_100 = (Index / 10) + 1;
			}
			
			// Call UpdateBatchIndex on the newly added entity, passing its BatchIndex rather than ArrayIndex
			SightComponent->UpdateBatchIndex(BatchIndex_100);
		}
	}
}

void AEDU_CORE_GameMode::AddToStatusComponentArray(UEDU_CORE_StatusComponent* StatusComponent)
{ FLOW_LOG
	if(StatusComponent)  // Check if the entity is valid
	{
		StatusComponentArray.AddUnique(StatusComponent);
	}
}

//------------------------------------------------------------------------------
// Get/Set: Teams
//------------------------------------------------------------------------------

void AEDU_CORE_GameMode::AddActorToTeamArray(AActor* Actor, EEDU_CORE_Team TeamArray)
{ FLOW_LOG
	if (Actor)
	{
		switch (TeamArray)
		{
			case EEDU_CORE_Team::None:		Team_0_Array.AddUnique(Actor);	break;
			case EEDU_CORE_Team::Team_1:	Team_1_Array.AddUnique(Actor);	break;
			case EEDU_CORE_Team::Team_2:	Team_2_Array.AddUnique(Actor);	break;
			case EEDU_CORE_Team::Team_3:	Team_3_Array.AddUnique(Actor);	break;
			case EEDU_CORE_Team::Team_4:	Team_4_Array.AddUnique(Actor);	break;
			case EEDU_CORE_Team::Team_5:	Team_5_Array.AddUnique(Actor);	break;
			case EEDU_CORE_Team::Team_6:	Team_6_Array.AddUnique(Actor);	break;
			case EEDU_CORE_Team::Team_7:	Team_7_Array.AddUnique(Actor);	break;
			case EEDU_CORE_Team::Team_8:	Team_8_Array.AddUnique(Actor);	break;
			case EEDU_CORE_Team::Team_9:	Team_9_Array.AddUnique(Actor);	break;
			case EEDU_CORE_Team::Team_10:	Team_10_Array.AddUnique(Actor);	break;
			
		default: ;
		}
	}
	
	// Log successful addition
	UE_LOG(LogTemp, Log, TEXT("Added actor %s to team %d"), *Actor->GetName(), static_cast<int32>(TeamArray));
}

void AEDU_CORE_GameMode::RemoveActorFromTeamArray(AActor* Actor, EEDU_CORE_Team TeamArray)
{ FLOW_LOG
	if (Actor)
	{
		switch(TeamArray)
		{
			case EEDU_CORE_Team::None:		Team_0_Array.Remove(Actor);		break;
			case EEDU_CORE_Team::Team_1:	Team_1_Array.Remove(Actor);		break;
			case EEDU_CORE_Team::Team_2:	Team_2_Array.Remove(Actor);		break;
			case EEDU_CORE_Team::Team_3:	Team_3_Array.Remove(Actor);		break;
			case EEDU_CORE_Team::Team_4:	Team_4_Array.Remove(Actor);		break;
			case EEDU_CORE_Team::Team_5:	Team_5_Array.Remove(Actor);		break;
			case EEDU_CORE_Team::Team_6:	Team_6_Array.Remove(Actor);		break;
			case EEDU_CORE_Team::Team_7:	Team_7_Array.Remove(Actor);		break;
			case EEDU_CORE_Team::Team_8:	Team_8_Array.Remove(Actor);		break;
			case EEDU_CORE_Team::Team_9:	Team_9_Array.Remove(Actor);		break;
			case EEDU_CORE_Team::Team_10:	Team_10_Array.Remove(Actor);	break;
			
		default: ;
		}
	}
}

void AEDU_CORE_GameMode::AddActorToTeamVisibleActorsArray(AActor* Actor, EEDU_CORE_Team TeamArray)
{ FLOW_LOG
	if (Actor)
	{
		switch(TeamArray)
		{
			case EEDU_CORE_Team::None:		VisibilityStruct.Team_0_VisibleActors.AddUnique(Actor);	break;
			case EEDU_CORE_Team::Team_1:	VisibilityStruct.Team_1_VisibleActors.AddUnique(Actor);	break;
			case EEDU_CORE_Team::Team_2:	VisibilityStruct.Team_2_VisibleActors.AddUnique(Actor);	break;
			case EEDU_CORE_Team::Team_3:	VisibilityStruct.Team_3_VisibleActors.AddUnique(Actor);	break;
			case EEDU_CORE_Team::Team_4:	VisibilityStruct.Team_4_VisibleActors.AddUnique(Actor);	break;
			case EEDU_CORE_Team::Team_5:	VisibilityStruct.Team_5_VisibleActors.AddUnique(Actor);	break;
			case EEDU_CORE_Team::Team_6:	VisibilityStruct.Team_6_VisibleActors.AddUnique(Actor);	break;
			case EEDU_CORE_Team::Team_7:	VisibilityStruct.Team_7_VisibleActors.AddUnique(Actor);	break;
			case EEDU_CORE_Team::Team_8:	VisibilityStruct.Team_8_VisibleActors.AddUnique(Actor);	break;
			case EEDU_CORE_Team::Team_9:	VisibilityStruct.Team_9_VisibleActors.AddUnique(Actor);	break;
			case EEDU_CORE_Team::Team_10:	VisibilityStruct.Team_10_VisibleActors.AddUnique(Actor);break;
			
		default: ;
		}
	}

	UE_LOG(FLOWLOG_CATEGORY, Warning, TEXT("Server: Added actor %s to team %d Visible Actors Array."), *Actor->GetName(), static_cast<int32>(TeamArray));
}

void AEDU_CORE_GameMode::RemoveActorFromTeamVisibleActorsArray(AActor* Actor, EEDU_CORE_Team TeamArray)
{ FLOW_LOG
	if (Actor)
	{
		switch(TeamArray)
		{
			case EEDU_CORE_Team::None:		VisibilityStruct.Team_0_VisibleActors.Remove(Actor);		break;
			case EEDU_CORE_Team::Team_1:	VisibilityStruct.Team_1_VisibleActors.Remove(Actor);		break;
			case EEDU_CORE_Team::Team_2:	VisibilityStruct.Team_2_VisibleActors.Remove(Actor);		break;
			case EEDU_CORE_Team::Team_3:	VisibilityStruct.Team_3_VisibleActors.Remove(Actor);		break;
			case EEDU_CORE_Team::Team_4:	VisibilityStruct.Team_4_VisibleActors.Remove(Actor);		break;
			case EEDU_CORE_Team::Team_5:	VisibilityStruct.Team_5_VisibleActors.Remove(Actor);		break;
			case EEDU_CORE_Team::Team_6:	VisibilityStruct.Team_6_VisibleActors.Remove(Actor);		break;
			case EEDU_CORE_Team::Team_7:	VisibilityStruct.Team_7_VisibleActors.Remove(Actor);		break;
			case EEDU_CORE_Team::Team_8:	VisibilityStruct.Team_8_VisibleActors.Remove(Actor);		break;
			case EEDU_CORE_Team::Team_9:	VisibilityStruct.Team_9_VisibleActors.Remove(Actor);		break;
			case EEDU_CORE_Team::Team_10:	VisibilityStruct.Team_10_VisibleActors.Remove(Actor);		break;
			
		default: ;
		}
	}
}

TArray<AActor*>& AEDU_CORE_GameMode::GetTeamArray(EEDU_CORE_Team TeamArray)
{
	switch(TeamArray)
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

	default:
		// Static empty array to return as a fallback
		static TArray<AActor*> EmptyArray;
			UE_LOG(LogTemp, Warning, TEXT("Invalid TeamArray enum value"));
		return EmptyArray;
	}
}

TArray<AActor*>& AEDU_CORE_GameMode::GetTeamVisibleActorsArray(EEDU_CORE_Team TeamArray)
{
	switch(TeamArray)
	{
		case EEDU_CORE_Team::None: return VisibilityStruct.Team_0_VisibleActors;
		case EEDU_CORE_Team::Team_1: return VisibilityStruct.Team_1_VisibleActors;
		case EEDU_CORE_Team::Team_2: return VisibilityStruct.Team_2_VisibleActors;
		case EEDU_CORE_Team::Team_3: return VisibilityStruct.Team_3_VisibleActors;
		case EEDU_CORE_Team::Team_4: return VisibilityStruct.Team_4_VisibleActors;
		case EEDU_CORE_Team::Team_5: return VisibilityStruct.Team_5_VisibleActors;
		case EEDU_CORE_Team::Team_6: return VisibilityStruct.Team_6_VisibleActors;
		case EEDU_CORE_Team::Team_7: return VisibilityStruct.Team_7_VisibleActors;
		case EEDU_CORE_Team::Team_8: return VisibilityStruct.Team_8_VisibleActors;
		case EEDU_CORE_Team::Team_9: return VisibilityStruct.Team_9_VisibleActors;
		case EEDU_CORE_Team::Team_10: return VisibilityStruct.Team_10_VisibleActors;

	default:
		// Static empty array to return as a fallback
			static TArray<AActor*> EmptyArray;
		UE_LOG(LogTemp, Warning, TEXT("Invalid TeamArray enum value"));
		return EmptyArray;
	}
}

//------------------------------------------------------------------------------
// Get/Set: Actor Management
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
// Get/Set: Waypoint Pool Management
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

//------------------------------------------------------------------------------
// Console Commands
//------------------------------------------------------------------------------

void AEDU_CORE_GameMode::ChangeTeamCommand(const int32 InTeam) const
{
	// Cast the integer to the enum type
	EEDU_CORE_Team NewTeam = static_cast<EEDU_CORE_Team>(FMath::Clamp(InTeam,0, 12));

	// Perform the team change logic
	UE_LOG(FLOWLOG_CATEGORY, Display, TEXT("Changing team to: %d"), NewTeam);

	// This should never fail, unless we are a dedicated server.
	TObjectPtr<APlayerController> LocalPlayerController = GetWorld()->GetFirstPlayerController();
	if(LocalPlayerController && LocalPlayerController->GetPawn())
	{
		TObjectPtr<AEDU_CORE_C2_Camera> LocalCamera = Cast<AEDU_CORE_C2_Camera>(LocalPlayerController->GetPawn());
		LocalCamera->ChangeTeam(NewTeam);
	}
	else
	{
		FLOW_LOG_ERROR("Cast to Camera failed?")
	}
}
