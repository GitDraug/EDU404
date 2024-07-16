// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Managers/GameModes/EDU_CORE_GameMode.h"
#include "Entities/EDU_CORE_TickingEntity.h"
#include "Framework/Data/FLOWLOG/FLOWLOG_MANAGERS.h"

//------------------------------------------------------------------------------
// Construction & Object Lifetime Management
//------------------------------------------------------------------------------
AEDU_CORE_GameMode::AEDU_CORE_GameMode(const FObjectInitializer& ObjectInitializer)
{ FLOW_LOG
	PrimaryActorTick.bCanEverTick = true;
	//PrimaryActorTick.bStartWithTickEnabled = true;
	//PrimaryActorTick.bRunOnAnyThread = true;
	PrimaryActorTick.SetTickFunctionEnable(true);
	//PrimaryActorTick.TickInterval = 0.016; // 60FPS = 0.016
}

void AEDU_CORE_GameMode::Tick(float DeltaTime)
{ // FLOW_LOG
	  Super::Tick(DeltaTime);

	if(Iticked == false)
	{
		
		// Loop through each entity pointer in the TickingEntityArray
		for (AEDU_CORE_TickingEntity* Entity : TickingEntityArray)
		{
			// Check if the CurrentEntity index has reached the EntityBatch limit
			if (CurrentEntity >= EntityBatch)
			{
				// If the limit is reached, reset CurrentEntity to 0
				CurrentEntity = 0;
			}

			// Check if the entity pointer is valid (not null)
			if (Entity)
			{
				// If the entity pointer is valid, call the ParallelTick function on the entity
				Entity->ParallelTick();
			}

			// Increment the CurrentEntity index for the next iteration
			CurrentEntity++;
		}
	}	
}

//------------------------------------------------------------------------------
// Public API
//------------------------------------------------------------------------------

void AEDU_CORE_GameMode::AddToTickingEntityArray(AEDU_CORE_TickingEntity* TickingEntity)
{ FLOW_LOG
	   
    if (TickingEntity)  // Check if the Entity is valid
    {
        // Add the entity to the TArray
        TickingEntityArray.Add(TickingEntity); // Adds the pointer to the array

        // Optionally, log the addition
        UE_LOG(LogTemp, Log, TEXT("Entity added: %s"), *TickingEntity->GetName());
    }
}

//------------------------------------------------------------------------------
// Functionality
//------------------------------------------------------------------------------

