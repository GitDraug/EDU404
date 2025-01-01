// Fill out your copyright notice in the Description page of Project Settings.

// THIS
#include "Entities/Components/EDU_CORE_SenseComponent.h"

// CORE
#include "Entities/EDU_CORE_PhysicsEntity.h"
#include "Entities/Components/EDU_CORE_StatusComponent.h"
#include "Framework/Data/FLOWLOGS/FLOWLOG_COMPONENTS.h"
#include "Framework/Managers/GameModes/EDU_CORE_GameMode.h"
#include "Framework/Pawns/EDU_CORE_C2_Camera.h"

//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------

UEDU_CORE_SenseComponent::UEDU_CORE_SenseComponent()
{
	// Don't Run logs in a component constructor, run it in OnRegister instead.

	// Disable ticking at the start
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UEDU_CORE_SenseComponent::OnRegister()
{
	FLOW_LOG
	Super::OnRegister();

	// Save this component owning actor
	Owner = GetOwner();

	// Ensure that we have a Status Component, this component is useless otherwise
	EnsureStatusComponent();
}

void UEDU_CORE_SenseComponent::BeginPlay()
{
	FLOW_LOG
	Super::BeginPlay();

	// Server Tick
	if (GetNetMode() != NM_Client)
	{
		if (AEDU_CORE_GameMode* GameModePtr = Cast<AEDU_CORE_GameMode>(GetWorld()->GetAuthGameMode()))
		{
			GameModePtr->AddToSightComponentArray(this);
			GameMode = GameModePtr;
		}
	}
}

//------------------------------------------------------------------------------
// Aggregated Server tick
//------------------------------------------------------------------------------

void UEDU_CORE_SenseComponent::ServerSightCalc(float DeltaTime)
{
}

void UEDU_CORE_SenseComponent::ServerSightExec(float DeltaTime)
{
	/*-------------------------------------------------------------------
	  I haven't spotted any difference running this in parallel for
	  or on the main thread. Mayde there's a difference if we run them
	  in more frequent batches.

	  More testing needed. // Draug
	-------------------------------------------------------------------*/
	switch (SenseType) {
		case EFSenseType::ESense_Sight:
			DetectActorsInFOV();
		break;

		case EFSenseType::ESense_SightAndHearing:
			DetectActorsInFOV();
			Listen();
		break;

		case EFSenseType::ESense_SightAndThermal:
			DetectActorsInFOV();
		break;

		case EFSenseType::ESense_All:
			DetectActorsInFOV();
			Listen();
		break;
	default: ;
	}
	

}

void UEDU_CORE_SenseComponent::UpdateBatchIndex(const int32 ServerBatchIndex)
{
	FLOW_LOG
	BatchIndex = ServerBatchIndex;
}

//------------------------------------------------------------------------------
// Functionality
//------------------------------------------------------------------------------

void UEDU_CORE_SenseComponent::DetectActorsInFOV()
{
	// FLOW_LOG
	/*---------------------------------------------------------------------
	  This function checks for actors of a certain pbjectype in a sphere.
	  
	  The actors don't need or have "generate overlap events" enabled,
	  nor do they need to block any specific channel. They only have
	  to be of the object type we search for.

	  #include "Engine/OverlapResult.h"
	---------------------------------------------------------------------*/
	EEDU_CORE_Team OurTeam = StatusComponent->GetActiveTeam();

	FVector ForwardVector = GetForwardVector();
	FVector Location = GetComponentLocation() + (ForwardVector * SightFocusLength);

	// Define object types to check, add more as needed
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(SenseObjectType);

	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(Owner);

	// Team 0 is used here unless a team is specified.
	CollisionQueryParams.AddIgnoredActors(GameMode->GetTeamVisibleActorsArray(OurTeam));
	CollisionQueryParams.AddIgnoredActors(GameMode->GetTeamArray(OurTeam));

	FQuat Rotation = FQuat::Identity;

	FCollisionShape Shape;
	if (FieldOfVisionType == EFieldOfVisionType::EFOV_Sphere)
	{
		Shape = FCollisionShape::MakeSphere(SightRadius);
	}
	else
	{
		Rotation = FRotationMatrix::MakeFromZX(ForwardVector, GetUpVector()).ToQuat();
		Shape = FCollisionShape::MakeCapsule(SightRadius, SightFocusLength);
	}

	// Perform the shape overlap
	GetWorld()->OverlapMultiByObjectType(
		SensedActorsArray, // Array to hold results
		Location, // Center of the sphere
		Rotation, // Rotation
		ObjectQueryParams, // Object types to overlap (e.g., ECC_Destructible)
		Shape, // Shape to check (FieldOfVisionType)
		CollisionQueryParams // Collision filter
	);

	// Optional: Draw a debug sphere (only in debug mode)
	#if WITH_EDITOR
		if (bDrawSightDebugShape)
		{
			// Always do this on the main thread.
			AsyncTask(ENamedThreads::GameThread, [this, Location, Rotation]()
			{
				if (FieldOfVisionType == EFieldOfVisionType::EFOV_Sphere)
				{
					DrawDebugSphere(GetWorld(), Location, SightRadius, 24, FColor::Yellow, false, 0.5f);
				}
				else
				{
					DrawDebugCapsule(GetWorld(), Location, SightFocusLength, SightRadius, Rotation, FColor::Yellow, false,
					                 0.5f);
				}
			});
		}
	#endif

	// Log overlapping actors if any
	if (SensedActorsArray.Num() > 0)
	{
		for (const FOverlapResult& OverlapResult : SensedActorsArray)
		{
			if (AActor* OverlappingActor = OverlapResult.GetActor())
			{
				if (AEDU_CORE_SelectableEntity* SelectableEntity = Cast<AEDU_CORE_SelectableEntity>(OverlappingActor))
				{
					if (UEDU_CORE_StatusComponent* TargetStatusComponent = SelectableEntity->GetStatusComponent())
					{
						int32 DetectionChance; // Default value

						switch (SenseType)
						{
							case EFSenseType::ESense_SightAndThermal:
								// Compare ThermalQuality with Camouflage
								DetectionChance = ThermalQuality - SelectableEntity->GetStatusComponent()->GetThermalCamouflage();

								// If DetectionChance is positive, check it
								if (DetectionChance > 0 && FMath::RandRange(1, 100) <= DetectionChance)
								{
									if (GetVisualConfirmation(SelectableEntity->GetActorLocation(), SelectableEntity))
									{
										SetEntityTeamVisibility(SelectableEntity, OurTeam, TargetStatusComponent);
										continue; // Continue for loop if thermal detection is successful.
									}
								}
							// Fall through to Sight detection if no detection yet.
							
							case EFSenseType::ESense_Sight:
								// Compare SightQuality with Camouflage
								DetectionChance = SightQuality - SelectableEntity->GetStatusComponent()->GetVisualCamouflage();

								// If DetectionChance is positive, check it
								if (DetectionChance > 0 && FMath::RandRange(1, 100) <= DetectionChance)
								{
									if (GetVisualConfirmation(SelectableEntity->GetActorLocation(), SelectableEntity))
									{
										SetEntityTeamVisibility(SelectableEntity, OurTeam, TargetStatusComponent);
									}
								}
							break;
						default:;
						}
					}
				}
			}
		}
	}

	// If we don't reset this, the entity will remember the unit it sensed indefinitly.
	SensedActorsArray.Reset();
}

bool UEDU_CORE_SenseComponent::GetVisualConfirmation(const FVector& EndLocation, const AActor* ActorToConfirm) const
{
	// FLOW_LOG
	// Check if we have a valid world context
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	// Set up the trace parameters (collision settings)
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Owner); // Optionally ignore the actor to confirm

	// Set up the collision channel (ECC_Visibility or another suitable channel)
	FHitResult HitResult;

	// Perform the line trace (single hit)
	World->LineTraceSingleByChannel(
		HitResult, // The result of the trace
		GetComponentLocation(), // Starting location of the trace
		EndLocation, // Ending location of the trace
		SenseObjectType, // Collision channel to trace for
		QueryParams // Query parameters
	);

	// Optional: draw a debug line for visualization
	if (HitResult.GetActor() == ActorToConfirm)
	{
	#if WITH_EDITOR
		if (bDrawSightDebugLine)
		{
			if (!IsInGameThread())
			{
				// If not, queue the function to run on the main thread
				AsyncTask(ENamedThreads::GameThread, [this, World, EndLocation]
				{
					// Green line indicates confirmati
					DrawDebugLine(World, GetComponentLocation(), EndLocation, FColor::Green, false, 1.0f, 0, 1.0f);
				});
			}
			else
			{
				// Green line indicates confirmati
				DrawDebugLine(World, GetComponentLocation(), EndLocation, FColor::Green, false, 1.0f, 0, 1.0f);
			}
		}
	#endif
	return true;
	}
	#if WITH_EDITOR
		if (bDrawSightDebugLine)
		{
			if (!IsInGameThread())
			{
				// If not, queue the function to run on the main thread
				AsyncTask(ENamedThreads::GameThread, [this, World, EndLocation]
				{
					// Red line indicates fail
					DrawDebugLine(World, GetComponentLocation(), EndLocation, FColor::Red, false, 1.0f, 0, 1.0f);
				});
			}
			else
			{
				// Red line indicates fail
				DrawDebugLine(World, GetComponentLocation(), EndLocation, FColor::Red, false, 1.0f, 0, 1.0f);
			}
		}
	#endif
	return false;
}

void UEDU_CORE_SenseComponent::Listen()
{ // FLOW_LOG
	/*---------------------------------------------------------------------
	  This function checks for actors of a certain pbjectype in a sphere.
	  
	  The actors don't need or have "generate overlap events" enabled,
	  nor do they need to block any specific channel. They only have
	  to be of the object type we search for.

	  #include "Engine/OverlapResult.h"
	---------------------------------------------------------------------*/
	EEDU_CORE_Team OurTeam = StatusComponent->GetActiveTeam();

	// Define object types to check, add more as needed
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(SenseObjectType);

	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(Owner);

	// Team 0 is used here unless a team is specified.
	CollisionQueryParams.AddIgnoredActors(GameMode->GetTeamVisibleActorsArray(OurTeam));
	CollisionQueryParams.AddIgnoredActors(GameMode->GetTeamArray(OurTeam));

	// Perform the shape overlap
	GetWorld()->OverlapMultiByObjectType(
		SensedActorsArray, // Array to hold results
		GetComponentLocation(), // Center of the sphere
		FQuat::Identity, // Rotation
		ObjectQueryParams, // Object types to overlap (e.g., ECC_Destructible)
		FCollisionShape::MakeSphere(HearingRadius), // Shape to check
		CollisionQueryParams // Collision filter
	);

	// Optional: Draw a debug sphere (only in debug mode)
	#if WITH_EDITOR
		if (bDrawHearningDebugShape)
		{
			// Always do this on the main thread.
			AsyncTask(ENamedThreads::GameThread, [this]()
			{
				DrawDebugSphere(GetWorld(), GetComponentLocation(), HearingRadius, 24, FColor::Orange, false, 0.5f);
			});
		}
	#endif

	// Log overlapping actors if any
	if (SensedActorsArray.Num() > 0)
	{
		for (const FOverlapResult& OverlapResult : SensedActorsArray)
		{
			if (AActor* OverlappingActor = OverlapResult.GetActor())
			{
				if (AEDU_CORE_SelectableEntity* SelectableEntity = Cast<AEDU_CORE_SelectableEntity>(OverlappingActor))
				{
					if (UEDU_CORE_StatusComponent* TargetStatusComponent = SelectableEntity->GetStatusComponent())
					{
						// Compare SightQuality with Camouflage, then assign the result to DetectionChance
						int32 DetectionChance = HearingQuality - SelectableEntity->GetStatusComponent()->GetNoiseCamouflage();

						// If DetectionChance is not positive, continua for loop early.
						if (DetectionChance <= 0)
						{
							continue;
						}

						// Generate a random failure between 1 and 100 and pray it's less than DetectionChance
						if (FMath::RandRange(1, 100) <= DetectionChance)
						{
							SetEntityTeamVisibility(SelectableEntity, OurTeam, TargetStatusComponent);
						}
					}
				}
			}
		}
	}

	// If we don't reset this, the entity will remember the unit it sensed indefinitly.
	SensedActorsArray.Reset();
}

void UEDU_CORE_SenseComponent::SetEntityTeamVisibility(AEDU_CORE_SelectableEntity* SelectableEntity,
                                                       EEDU_CORE_Team OurTeam,
                                                       UEDU_CORE_StatusComponent* TargetStatusComponent) const
{
	FLOW_LOG
	if (!HasAuthority())
	{
		return;
	}
	if (GetNetMode() == NM_Client)
	{
		return;
	}

	// It's easy to get confused here, we are adding the enemy to OUR array of visible actors.
	GameMode->AddActorToTeamVisibleActorsArray(SelectableEntity, OurTeam);

	// Notify Enemy that it is spotted by ActiveTeam
	TargetStatusComponent->ResetVisibilityForTeam(OurTeam);

	UE_LOG(FLOWLOG_CATEGORY, Warning, TEXT("Visual Confirmation Succeded! Adding %s to VisibleActorsArray %d"), *SelectableEntity->GetName(), OurTeam);

	// Check if the value at that index is 1 (true/visible) or 0 (false/hidden)
	if (OurTeam == StatusComponent->GetC2Camera()->ActiveTeam)
	{
		// If it's 1, make the actor visible
		SelectableEntity->SetActorHiddenInGame(false);
	}
}

//------------------------------------------------------------------------------
// Functionality > Setup
//------------------------------------------------------------------------------

void UEDU_CORE_SenseComponent::EnsureStatusComponent()
{
	FLOW_LOG
	if (Owner)
	{
		StatusComponent = Owner->FindComponentByClass<UEDU_CORE_StatusComponent>();

		if (!StatusComponent)
		{
			FLOW_LOG_ERROR("StatusComponent not detected in owning actor.")
		}
	}
	else
	{
		FLOW_LOG_ERROR("Owner actor is null.")
	}
}
