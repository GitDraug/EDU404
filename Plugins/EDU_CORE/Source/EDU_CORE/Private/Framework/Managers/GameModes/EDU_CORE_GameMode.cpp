// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Managers/GameModes/EDU_CORE_GameMode.h"

#include "Entities/EDU_CORE_AbstractEntity.h"
#include "Entities/EDU_CORE_MobileEntity.h"
#include "Entities/EDU_CORE_PhysicalEntity.h"
#include "Framework/Data/FLOWLOGS/FLOWLOG_MANAGERS.h"

//------------------------------------------------------------------------------
// Construction & Object Lifetime Management
//------------------------------------------------------------------------------
AEDU_CORE_GameMode::AEDU_CORE_GameMode(const FObjectInitializer& ObjectInitializer)
{ FLOW_LOG
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.SetTickFunctionEnable(true);

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
	// Server-Side Aggregated Tick: PhysicalEntityArray
	//------------------------------------------------------------------------------	
	ParallelFor(PhysicalEntityArray.Num(), [this, &DeltaTime ](int32 Index)
	{
		if(PhysicalEntityArray[Index])
		{
			PhysicalEntityArray[Index]->ServerPhysicsCalc(DeltaTime);
		}
	});

	for (AEDU_CORE_PhysicalEntity* PhysicalEntity : PhysicalEntityArray)
	{
		if (PhysicalEntity)
		{
			PhysicalEntity->ServerPhysicsExec(DeltaTime);
		}
	}

	//------------------------------------------------------------------------------
	// Server-Side Aggregated Tick: PhysicalEntityArray
	//------------------------------------------------------------------------------
	
	ParallelFor(MobileEntityArray.Num(), [this, &DeltaTime ](int32 Index)
	{
		if(MobileEntityArray[Index])
		{
			MobileEntityArray[Index]->ServerMobileCalc(DeltaTime);
		}
	});

	MobileEntityIndex++;
	
	for (AEDU_CORE_MobileEntity* MobileEntity : MobileEntityArray)
	{
		if (MobileEntity)
		{
			MobileEntity->ServerMobilesExec(DeltaTime);
		}
	}
	
	/*------------------------ Old Server-Side Simple Gravity ------------------------------*/
	/* Saved For Educational Purpose
	/*--------------------------------------------------------------------------------------
	// FCriticalSection Mutex; // Mutex for thread synchronization to avoid data races
	if (false)
	{
		// Arrays to store Updated Location vector and a bool to check if we should update at all, best stored on the stack.
		TArray<FVector> UpdatedLocationArray;
		TArray<bool> bUpdateArray;

		// Ensure arrays are sized to match the number of PhysicalEntity objects
		bUpdateArray.SetNum(MobileEntityArray.Num());
		UpdatedLocationArray.SetNum(PhysicalEntityArray.Num());
		
		ParallelFor(MobileEntityArray.Num(), [this, &UpdatedLocationArray, &DeltaTime, &bUpdateArray](int32 Index)
		{
			// Get the actor's bounding box
			FVector Origin, BoxExtent;
			MobileEntityArray[Index]->GetActorBounds(false, Origin, BoxExtent); // BoxExtent – Set to half the actor's size in 3d space
			
			// Define the start and end points for the line trace
			FVector TraceStartLocation = MobileEntityArray[Index]->GetActorLocation();
			FVector TraceEndLocation = TraceStartLocation - FVector(0.f, 0.f, BoxExtent.Z + 100.f); // Trace 1m  downward

			// Perform the line trace
			FHitResult HitResult;
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(MobileEntityArray[Index]); // Ignore the actor itself

			// Line trace downwards
			if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStartLocation, TraceEndLocation, ECC_Visibility, QueryParams))
			{
				if(HitResult.bBlockingHit)
				{
					// If we hit something, set the new center location to the impact point
					FVector FinalLocation = HitResult.ImpactPoint + FVector(0.f, 0.f, BoxExtent.Z) ;
					UpdatedLocationArray[Index] = FinalLocation;
					bUpdateArray[Index] = true; // Set the bool to update
				}

			}
			else
			{
				// If nothing was hit, it means we are falling.
				FVector FinalLocation = TraceEndLocation;
				UpdatedLocationArray[Index] = FinalLocation;
				bUpdateArray[Index] = true; // Set the bool to update
			}
		});

		/* GAME THREAD 
		// Post-processing loop to set the actor location for each PhysicalEntity
		for (int32 Index = 0; Index < PhysicalEntityArray.Num(); ++Index)
		{
			// Check if the entity pointer is valid (not null) to avoid dereferencing a null pointer
			if (AEDU_CORE_PhysicalEntity* PhysicalEntity = PhysicalEntityArray[Index])
			{
				// Set the actor's location to the new computed location from NewLocationArray
				// DrawDebugLine(GetWorld(), PhysicalEntity->GetActorLocation(), UpdatedLocationArray[Index], FColor::Green, false, 1.f, 0, 1.0f);
				PhysicalEntity->SetActorLocation(UpdatedLocationArray[Index]);
			}
		}
	}

	FrameCounter = 0;*/
}	

//------------------------------------------------------------------------------
// Public API
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

void AEDU_CORE_GameMode::AddToPhysicalEntityArray(AEDU_CORE_PhysicalEntity* PhysicalEntity)
{ FLOW_LOG
	   
	if (PhysicalEntity)  // Check if the Entity is valid
	{
		// Add the entity to the TArray
		PhysicalEntityArray.Add(PhysicalEntity); // Adds the pointer to the array
		//UE_LOG(FLOWLOG_CATEGORY, Log, TEXT("Entity added: %s"), *PhysicalEntity->GetName());
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
// Functionality
//------------------------------------------------------------------------------

