// Fill out your copyright notice in the Description page of Project Settings.


#include "Entities/Components/EDU_CORE_SightComponent.h"
#include "Entities/Components/EDU_CORE_StatusComponent.h"

#include "Kismet/KismetSystemLibrary.h"


// Sets default values for this component's properties
USightComponent::USightComponent()
{
	// Disable ticking at the start
	PrimaryComponentTick.bCanEverTick = false;
}

void USightComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

//------------------------------------------------------------------------------
// Functionality
//------------------------------------------------------------------------------

void USightComponent::PerformVisionFieldBoxTrace() const
{
	TArray<FHitResult> OutHits; // Array to hold hit results

	// Start location, flattened to the X-Y plane
	FVector Start = FVector(GetComponentLocation().X, GetComponentLocation().Y, 0.f);

	// Flattened forward vector for a horizontal direction, scaled by SightLength and added to Start to form End
	FVector Forward2D = FVector(GetForwardVector().X, GetForwardVector().Y, 0.f);
	FVector End = Start + (Forward2D * SightLength);
	
	FRotator Rotation = FRotator(GetComponentRotation().Yaw, 0.f, 0.f);

	// Perform the box trace
	bool bHit = UKismetSystemLibrary::BoxTraceMulti(
		this,																// WorldContextObject
		Start,																// Start
		End,																// End
		FVector(SightLength / 2.0f, SightWidth / 2.0f, 0.0f),				// No height,
		Rotation,															// Orientation
		SightDetectionVolumeChannel,										// Trace channel		
		false,																// bTraceComplex?
		IgnoredActors,														// ActorsToIgnore
		bDrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,	// DrawDebugTrace::Type DrawDebugType, 
		OutHits,															// TArray<FHitResult>& OutHits, 
		true,																// bIgnoreSelf, 
		FLinearColor::Green,												// FLinearColor TraceColor = FLinearColor::Red, 
		FLinearColor::Red,													// FLinearColor TraceHitColor = FLinearColor::Green, 
		30.f																// float DrawTime = 5)
	);

	// Process the hit results
	for (const FHitResult& Hit : OutHits)
	{
		AActor* SensedActor = Hit.GetActor();
		
		// Check if the actor has a Status Component
		if (UEDU_CORE_StatusComponent* StatusComponent = SensedActor->FindComponentByClass<UEDU_CORE_StatusComponent>())
		{
			// We can stop here if it's impossible to see the target
			if(this->SightQuality - StatusComponent->GetCamouflage() > 0)
			{
				// Get visual confirmation of the sensed entity
				if(VisualConfirmation())
				{
					// Add tor detectedarray.
				}
			}
		}
	}
}

bool USightComponent::VisualConfirmation() const
{
	// Define start and end points of the trace
	FVector Start = GetComponentLocation();
	FVector End = Start + (GetForwardVector() * SightLength);

	// Structure to hold hit result information
	FHitResult HitResult;

	// Define collision parameters (optional)
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(GetOwner()); // Ignore the actor performing the trace, if needed
	
	// Perform the line trace and check if anything was hit
	if (bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,                 // Out parameter for hit information
		Start,                     // Starting point of the trace
		End,                       // End point of the trace
		ECC_Visibility,            // Trace channel to use (visibility, camera, custom, etc.)
		CollisionParams            // Optional collision parameters (ignored actors, etc.)
	))
	{
		return true;
	}
		return false;
}

