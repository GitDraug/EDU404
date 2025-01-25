// Fill out your copyright notice in the Description page of Project Settings.

// THIS
#include "Entities/Components/SenseComponent.h"

// CORE
#include "Entities/EDU_CORE_PhysicsEntity.h"
#include "Entities/Components/StatusComponent.h"
#include "Entities/Components/TurretWeaponComponent.h"
#include "Framework/Data/FLOWLOGS/FLOWLOG_COMPONENTS.h"
#include "Framework/Managers/GameModes/EDU_CORE_GameMode.h"
#include "Framework/Pawns/EDU_CORE_C2_Camera.h"

//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------

USenseComponent::USenseComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{ 
	// Don't Run logs in a component constructor, run it in OnRegister instead.

	// Disable ticking at the start
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	// This should never exist on the client.
	SetIsReplicated(false);
}

void USenseComponent::OnRegister()
{ FLOW_LOG
	Super::OnRegister();

	// Save this component owning actor
	Owner = GetOwner();

	if(bAttachedToTurret && Owner)
	{
		TArray<UTurretWeaponComponent*> TurretComponents;
		Owner->GetComponents<UTurretWeaponComponent>(TurretComponents);

		for (UTurretWeaponComponent* TurretComp : TurretComponents)
		{
			if(TurretComp && TurretComp->GetName() == TurretName)
			{
				Parent = TurretComp;
				break;
			}
		}
	}
	
	// Ensure that we have a Status Component; this component is useless otherwise
	EnsureStatusComponent();
}


void USenseComponent::BeginPlay()
{	FLOW_LOG	
	Super::BeginPlay();
	
	// Server Tick
	if(HasAuthority())
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

void USenseComponent::ServerSightCalc(float DeltaTime)
{
	if (!HasAuthority()) return;
	
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

void USenseComponent::ServerSightExec(float DeltaTime)
{
	
}

void USenseComponent::UpdateBatchIndex(const int32 ServerBatchIndex)
{ FLOW_LOG
	BatchIndex = ServerBatchIndex;
}

//------------------------------------------------------------------------------
// Functionality
//------------------------------------------------------------------------------

void USenseComponent::DetectActorsInFOV()
{ // FLOW_LOG
	/*---------------------------------------------------------------------
	  This function checks for actors of a certain objectype in a sphere.
	  
	  The actors don't need or have "generate overlap events" enabled,
	  nor do they need to block any specific channel. They only have
	  to be of the object type we search for.

	  #include "Engine/OverlapResult.h"
	---------------------------------------------------------------------*/
	EEDU_CORE_Team OurTeam = StatusComponent->GetActiveTeam();

	FTransform ParentTransform;
	if(!Parent)
	{
		ParentTransform = Owner->GetTransform();
	}
	else
	{
		ParentTransform = Parent->GetComponentTransform();
	}

	FTransform CombinedTransform = ParentTransform * RelativeTransform;
	FVector ComponentLocation = ParentTransform.GetLocation(); 
	FVector ForwardVector = CombinedTransform.GetRotation().GetForwardVector();
	FVector LOSCenterLocation = ComponentLocation + (ForwardVector * SightFocusLength);

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
		Rotation = FRotationMatrix::MakeFromZX(ForwardVector, Owner->GetActorUpVector()).ToQuat();
		Shape = FCollisionShape::MakeCapsule(SightRadius, SightFocusLength);
	}

	// Perform the shape overlap
	GetWorld()->OverlapMultiByObjectType(
		SensedActorsArray,		// Array to hold results
		LOSCenterLocation,		// Center of the sphere
		Rotation,				// Rotation
		ObjectQueryParams,		// Object types to overlap (e.g., ECC_Destructible)
		Shape,					// Shape to check (FieldOfVisionType)
		CollisionQueryParams	// Collision filter
	);

	// Optional: Draw a debug sphere (only in debug mode)
	#if WITH_EDITOR
		if (bDrawSightDebugShape)
		{
			// Always do this on the main thread.
			AsyncTask(ENamedThreads::GameThread, [this, LOSCenterLocation, Rotation]()
			{
				if (FieldOfVisionType == EFieldOfVisionType::EFOV_Sphere)
				{
					DrawDebugSphere(GetWorld(), LOSCenterLocation, SightRadius, 24, FColor::Yellow, false, 0.5f);
				}
				else
				{
					DrawDebugCapsule(GetWorld(), LOSCenterLocation, SightFocusLength, SightRadius, Rotation, FColor::Yellow, false, 0.5f);
				}
			});
		}
	#endif

	// Log overlapping actors if any
	if (SensedActorsArray.Num() > 0)
	{
		for (const FOverlapResult& OverlapResult : SensedActorsArray)
		{
			if(AActor* OverlappingActor = OverlapResult.GetActor())
			{
				if(AEDU_CORE_SelectableEntity* SelectableEntity = Cast<AEDU_CORE_SelectableEntity>(OverlappingActor))
				{
					if(UStatusComponent* TargetStatusComponent = SelectableEntity->GetStatusComponent())
					{
						int32 DetectionChance; // Default value

						switch (SenseType)
						{
						case EFSenseType::ESense_SightAndThermal:
							// Compare ThermalQuality with Camouflage
							DetectionChance = ThermalQuality - TargetStatusComponent->GetThermalCamouflage();

						// If DetectionChance is positive, check it
							if (DetectionChance > 0 && FMath::RandRange(1, 100) <= DetectionChance)
							{
								if (GetVisualConfirmation(ComponentLocation, SelectableEntity->GetActorLocation(), SelectableEntity))
								{
									AsyncTask(ENamedThreads::GameThread, [this, SelectableEntity, OurTeam, TargetStatusComponent]()
									{
										// Make sure the entity is invisible to avoid duplicates.
										SetEntityTeamVisibility(SelectableEntity, OurTeam, TargetStatusComponent);
									});
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
								if (GetVisualConfirmation(ComponentLocation, SelectableEntity->GetActorLocation(), SelectableEntity))
								{
									AsyncTask(ENamedThreads::GameThread, [this, SelectableEntity, OurTeam, TargetStatusComponent]()
									{
										SetEntityTeamVisibility(SelectableEntity, OurTeam, TargetStatusComponent);
									});
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

bool USenseComponent::GetVisualConfirmation(const FVector& StartLocation, const FVector& EndLocation, const AActor* ActorToConfirm) const
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
		HitResult,					// The result of the trace
		StartLocation,				// Starting location of the trace
		EndLocation,				// Ending location of the trace
		SenseObjectType,			// Collision channel to trace for
		QueryParams					// Query parameters
	);

	// Optional: draw a debug line for visualization
	if (HitResult.GetActor() == ActorToConfirm)
	{
	#if WITH_EDITOR
		if(bDrawSightDebugLine)
		{
			// Green line indicates confirmation.
			if(!IsInGameThread())
			{
				// If not, queue the function to run on the main thread
				AsyncTask(ENamedThreads::GameThread, [this, World, EndLocation, StartLocation]
				{
					DrawDebugLine(World, StartLocation, EndLocation, FColor::Green, false, 1.0f, 0, 1.0f);
				});
			}
			else
			{
				DrawDebugLine(World, StartLocation, EndLocation, FColor::Green, false, 1.0f, 0, 1.0f);
			}
		}
	#endif
		
		return true;
	}
	
	#if WITH_EDITOR
	if (bDrawSightDebugLine)
	{
		// Red line indicates fail.
		if(!IsInGameThread())
		{
			// If not, queue the function to run on the main thread
			AsyncTask(ENamedThreads::GameThread, [this, World, EndLocation, StartLocation]
			{
				DrawDebugLine(World, StartLocation, EndLocation, FColor::Red, false, 1.0f, 0, 1.0f);
			});
		}
		else
		{
			DrawDebugLine(World, StartLocation, EndLocation, FColor::Red, false, 1.0f, 0, 1.0f);
		}
	}
	#endif
	
	return false;
}

void USenseComponent::Listen()
{ // FLOW_LOG
	/*---------------------------------------------------------------------
	  This function checks for actors of a certain objectype in a sphere.
	  
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
		Owner->GetActorLocation(), // Center of the sphere
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
			DrawDebugSphere(GetWorld(), Owner->GetActorLocation(), HearingRadius, 24, FColor::Orange, false, 0.5f);
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
					if (UStatusComponent* TargetStatusComponent = SelectableEntity->GetStatusComponent())
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
							AsyncTask(ENamedThreads::GameThread, [this, SelectableEntity, OurTeam, TargetStatusComponent]()
							{
								SetEntityTeamVisibility(SelectableEntity, OurTeam, TargetStatusComponent);
							});	
						}
					}
				}
			}
		}
	}
	
	// If we don't reset this, the entity will remember the unit it sensed indefinitly.
	SensedActorsArray.Reset();
}

void USenseComponent::SetEntityTeamVisibility(AEDU_CORE_SelectableEntity* SelectableEntity,
                                              EEDU_CORE_Team OurTeam,
                                              UStatusComponent* TargetStatusComponent) const
{// FLOW_LOG
	// Make sure Target Visibility for Our Team is less than half the visibility timer, else we will add duplicates.
	if (!(TargetStatusComponent->GetVisibleForTeam(OurTeam) < (TargetStatusComponent->GetVisibilityTimer() >> 1))) return;

	// It's easy to get confused here, we are adding the enemy to OUR array of visible actors.
	GameMode->AddActorToTeamVisibleActorsArray(SelectableEntity, OurTeam);

	// Notify Enemy that it is spotted by ActiveTeam
	TargetStatusComponent->ResetVisibilityForTeam(OurTeam);

	// UE_LOG(FLOWLOG_CATEGORY, Warning, TEXT("Visual Confirmation Succeded! Adding %s to VisibleActorsArray %d"), *SelectableEntity->GetName(), OurTeam);

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

void USenseComponent::EnsureStatusComponent()
{ FLOW_LOG
	if (Owner)
	{
		StatusComponent = Owner->FindComponentByClass<UStatusComponent>();

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
