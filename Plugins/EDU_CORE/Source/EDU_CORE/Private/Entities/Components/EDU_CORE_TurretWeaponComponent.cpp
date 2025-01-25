// Fill out your copyright notice in the Description page of Project Settings.

// CORE
#include "Entities/Components/TurretWeaponComponent.h"

// CORE
#include "Algo/Partition.h"
#include "Entities/EDU_CORE_MobileEntity.h"
#include "Entities/Components/EngagementComponent.h"
#include "Entities/Components/StatusComponent.h"

#include "FunctionLibrary/UtilityLibrary.h"

#include "Framework/Data/FLOWLOGS/FLOWLOG_COMPONENTS.h"
#include "Framework/Managers/GameModes/EDU_CORE_GameMode.h"

//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------

UTurretWeaponComponent::UTurretWeaponComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTurretWeaponComponent::OnRegister()
{ FLOW_LOG
	Super::OnRegister();

	// Clients need WeaponInfo for UI.
	SetIsReplicated(true);

	// Find whatever component this component is attached to and save it.
	if(UStaticMeshComponent* ParentMesh = Cast<UStaticMeshComponent>(GetAttachParent()))
	{
		TurretMountMesh = ParentMesh;
		FLOW_LOG_WARNING("Turret attached, taking control.")
			
		if(ParentMesh->GetIsReplicated() == false)
		{
			ParentMesh->SetIsReplicated(true);
			FLOW_LOG_WARNING("Turret mount is not replicated, adjusting.")
			
			if(ParentMesh->GetIsReplicated() == false)
			{
				FLOW_LOG_ERROR("Replication Failed, do it manually.")
			}
			else
			{
				FLOW_LOG_WARNING("Replication succeded.")
			}
		}
		if(ParentMesh == GetOwner()->GetRootComponent() && ParentMesh->GetOwner()->IsA<AEDU_CORE_MobileEntity>())
		{
			FLOW_LOG_ERROR("Attached Mesh is a Mobile Entity! This will result in unwanted behaviour, detach the component and attach it to a scenecomponent or staticmesh that is not a Mobile Entity!")
		}
	}
	else
	{
		FLOW_LOG_ERROR("Attached Mesh is not a static Mesh!")
	}
	
	for(FProjectileWeaponInformation& Weapon : WeaponStructArray)
	{
		Weapon.WeaponID = FGuid::NewGuid(); // Assign a new unique GUID to each Weapon
		Weapon.OwningTurret = this;			
		Weapon.OwningActor = GetOwner();
	}
}

void UTurretWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	//------------------------------------------------------------------------------
	// Disable collision and physics simulation for both barrel and mount
	//------------------------------------------------------------------------------
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TurretMountMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SetSimulatePhysics(false);
	TurretMountMesh->SetSimulatePhysics(false);

	// Server Tick
	if(GetNetMode() != NM_Client)
	{
		if (AEDU_CORE_GameMode* GameMode = Cast<AEDU_CORE_GameMode>(GetWorld()->GetAuthGameMode()))
		{
			GameMode->AddToTurretComponentArray(this);

			// Check Weapons for Range and Damage
			EvaluateWeapons();

			// Register with Owners Engagement Component
			EnsureEngagementComponent();
		}
	}
}

//------------------------------------------------------------------------------
// Aggregated Server tick
//------------------------------------------------------------------------------

void UTurretWeaponComponent::ServerTurretCalc(float AsyncDeltaTime)
{
	if(TurretStatus < EWeaponStatus::Searching) return;

	//------------------------------------------------------------------------------------------------------
	// Check if we have a target
	//------------------------------------------------------------------------------------------------------
	
	FVector TargetPos;
	if(TargetEntity && TurretStatus > EWeaponStatus::Searching)
	{
		TargetPos = TargetEntity->GetActorLocation();
	}
	else if(GroundTargetPosition != FVector::ZeroVector)
	{
		TargetPos = GroundTargetPosition;
	}
	else // If we are searching
	{
		
		TargetPos = LastKnownTargetPosition;
	}

	//------------------------------------------------------------------------------------------------------
	// Mount (Yaw) rotation calculation
	//------------------------------------------------------------------------------------------------------
	
	// Check if the Mount is aligned already (Note that we're using the barrel's position to align properly).
	FRotator BarrelStartRotation = this->GetComponentRotation();
	FRotator BarrelEndRotation;
	if(!TargetEntity)
	{
		BarrelEndRotation = GetOwner()->GetActorRotation();
	}
	else
	{
		BarrelEndRotation = UtilityLibrary::GetRotationToTargetPos(this, TargetPos);
	}
	
	
	// If the missalignment is larger than our tolerence for error, we will align.
	float MountRotationDistance = UtilityLibrary::CalculateRotationDistance(BarrelStartRotation.Yaw, BarrelEndRotation.Yaw, AlignmentTolerance);
	if(FMath::Abs(MountRotationDistance) > AlignmentTolerance)
	{
		bMountShouldAlign = true;
		MountTurnRate = UtilityLibrary::CalculateFastestTurnRate(MountRotationSpeed, MountRotationDistance, MountSlowRotationDistance);
	}
	else
	{
		bMountShouldAlign = false;
	}
	
	//------------------------------------------------------------------------------------------------------
	// Barrel (Pitch) rotation calculation
	//------------------------------------------------------------------------------------------------------
	// If the missalignment is larger than our tolerence for error, we will align.
	float BarrelRotationDistance = UtilityLibrary::CalculateRotationDistance(BarrelStartRotation.Pitch, BarrelEndRotation.Pitch, AlignmentTolerance);
	if(FMath::Abs(BarrelRotationDistance) > AlignmentTolerance)
	{
		BarrelTurnRate = UtilityLibrary::CalculateFastestTurnRate(BarrelRotationSpeed, BarrelRotationDistance, BarrelSlowRotationDistance);

		// Check bounds
		if(BarrelEndRotation.Pitch <= BarrelMaxElevation && BarrelEndRotation.Pitch >= BarrelMinElevation)
		{
			bBarrelShouldAlign = true;
		}
		else
		{
			bBarrelShouldAlign = false;
		}
	}
	else
	{
		bBarrelShouldAlign = false;
	}

	if (!bBarrelShouldAlign && !bMountShouldAlign && !TargetEntity)
	{
		// If we don't have a target, and we are not aligning = Ready.
		TurretStatus = EWeaponStatus::Ready; 
	}
	
	// GEngine->AddOnScreenDebugMessage(30, GetWorld()->DeltaTimeSeconds, FColor::Cyan, 
	//FString::Printf(TEXT("TurretStatus: %d"), TurretStatus));
		
	//GEngine->AddOnScreenDebugMessage(31, GetWorld()->DeltaTimeSeconds, FColor::Cyan, 
	//FString::Printf(TEXT("BarrelStartRotation: %f"), BarrelStartRotation.Yaw));

	//GEngine->AddOnScreenDebugMessage(32, GetWorld()->DeltaTimeSeconds, FColor::Cyan, 
	//FString::Printf(TEXT("BarrelEndRotation: %f"), BarrelEndRotation.Yaw));
    	
	//GEngine->AddOnScreenDebugMessage(33, GetWorld()->DeltaTimeSeconds, FColor::Cyan, 
	//FString::Printf(TEXT("BarrelRotationDistance: %f"), BarrelRotationDistance));
}

void UTurretWeaponComponent::ServerTurretExec(float AsyncDeltaTime)
{
	if(TurretStatus < EWeaponStatus::Searching) return;
	
	//-------------------------------------------------------------------------------------------------
	// Using 0.02f as Fixed DeltaTick will make the rotation degrees per second when not slowed down.
	//-------------------------------------------------------------------------------------------------
	constexpr float FixedDeltaTick = 0.02f; // This needs to be the same as fixed Async tick
	 
	if(bMountShouldAlign)
	{
		// Yaw
		TurretMountMesh->AddRelativeRotation(FRotator( 0.f, MountTurnRate * AsyncDeltaTime * FixedDeltaTick, 0.0f ));
	}

	if(bBarrelShouldAlign)
	{
		// Pitch
		this->AddRelativeRotation(FRotator( BarrelTurnRate * AsyncDeltaTime * FixedDeltaTick, 0.f, 0.0f ));
	}
}

void UTurretWeaponComponent::ServerTimeGatedTurretExec(float AsyncDeltaTime)
{
	if(PriorityTargetsArray.Num() > 0)
	{
		// Always do this on the main thread.
		AsyncTask(ENamedThreads::GameThread, [this]
		{
			SortTargets(PriorityTargetsArray);
		});
		
		const FVector MyPos = GetOwner()->GetActorLocation();
		const float MaxRangeSquared = MaxRange * MaxRange; // Squared for a proper comparison with squared distances.

		for(const TObjectPtr<AEDU_CORE_SelectableEntity>& Target : PriorityTargetsArray)
		{
			// Forgett the Target if it is Deleted.
			if(!Target)	{ PriorityTargetsArray.RemoveSingleSwap(Target); continue; }
			if(!Target->GetStatusComponent()->GetVisibleForTeam(OurTeam)) continue;
			
			const FVector TargetPos = Target->GetActorLocation();
			const float Distance = FVector::DistSquared(MyPos, TargetPos);

			// Forgett the Target if it is outside our combat range.
			if(Distance > MaxRangeSquared)	{ PriorityTargetsArray.RemoveSingleSwap(Target); continue; }

			// Move on to the next target if we don't have LOS.
			if(!HasLineOfSight(MyPos, TargetPos, Target)) { continue; }
			LastKnownTargetPosition = TargetPos;

			AsyncTask(ENamedThreads::GameThread, [this, Target]()
			{
				TargetEntity = Target;
				TurretStatus = EWeaponStatus::Engaged;
				NoTargetTimer = 0;
			});

			// Valid Priority Target, exit.
			return;
		}
		// No Valid Priority Target
	}

	if(ViableTargetsArray.Num() > 0)
	{
	// Always do this on the main thread.
		AsyncTask(ENamedThreads::GameThread, [this]
		{
			SortTargets(ViableTargetsArray);
		});
		
		const FVector MyPos = GetOwner()->GetActorLocation();
		const float MaxRangeSquared = MaxRange * MaxRange; // Squared for a proper comparison with squared distances.

		for(const TObjectPtr<AEDU_CORE_SelectableEntity>& Target : ViableTargetsArray)
		{
			// Forgett the Target if it is Deleted.
			if(!Target)	{ ViableTargetsArray.RemoveSingleSwap(Target); continue; }
			if(!Target->GetStatusComponent()->GetVisibleForTeam(OurTeam)) continue;
			
			const FVector TargetPos = Target->GetActorLocation();
			const float Distance = FVector::DistSquared(MyPos, TargetPos);

			// Forgett the Target if it is outside our combat range.
			if(Distance > MaxRangeSquared)	{ ViableTargetsArray.RemoveSingleSwap(Target); continue; }

			// Move on to the next target if we don't have LOS.
			if(!HasLineOfSight(MyPos, TargetPos, Target)) { continue; }

			AsyncTask(ENamedThreads::GameThread, [this, Target]()
			{
				TargetEntity = Target;
				TurretStatus = EWeaponStatus::Engaged;
				NoTargetTimer = 0;
			});
			// Valid Target, exit.
			return;
		}
		// No Valid Target
	} 
	
	NoTargetTimer++;
	TurretStatus = EWeaponStatus::Searching;
	if(NoTargetTimer > 10)
	{
		NoTargetTimer = 0;
		TargetEntity = nullptr;
	}
}

//------------------------------------------------------------------------------
// Functionality: Setup
//------------------------------------------------------------------------------

void UTurretWeaponComponent::EnsureEngagementComponent()
{
	if(GetOwner())
	{
		EngagementComponent = GetOwner()->FindComponentByClass<UEngagementComponent>();
		
		if(EngagementComponent)
		{
			EngagementComponent->AddToTurretArray(this);
		}
		else
		{
			FLOW_LOG_ERROR("EngagementComponent not detected in owning actor.")
		}
	}
	else
	{
		FLOW_LOG_ERROR("Owner actor is null.")
	}
}

void UTurretWeaponComponent::EvaluateWeapons()
{
	// Loop through each weapon in the array
	for (FProjectileWeaponInformation& Weapon : WeaponStructArray)
	{
		// Skip weapons with no ammo
		if (Weapon.CurrentAmmo <= 0)
		{
			continue;  // Move to the next weapon
		}

		// Check if this weapon's damage exceeds the current MaxDamage
		if (Weapon.Damage > MaxDamage)
		{
			// MaxDamage Stats
			MaxDamage = Weapon.Damage;
			MaxDamageType = Weapon.DamageType;
		}

		// Check if this weapon's MaxDistance exceeds the current MaxRange
		if (Weapon.MaxDistance > MaxRange)
		{
			// Update the maximum range and associated stats
			MaxRange = Weapon.MaxDistance;
		}
	}
}

void UTurretWeaponComponent::SortTargets(TArray<TObjectPtr<AEDU_CORE_SelectableEntity>>& TargetArray) const
{
	switch (TargetPriority)
	{
	case ETargetPriority::Nearest:
		{
			FVector MyPos = GetOwner()->GetActorLocation();

			// Partition the array: valid targets to the left, invalid to the right
			const int32 PartitionIndex = Algo::Partition(TargetArray, [](const TObjectPtr<AEDU_CORE_SelectableEntity>& Target)
			{
				return IsValid(Target); // Keep valid targets on the left
			});

			// Sort only the valid portion of the array
			Algo::Sort(TargetArray, [&MyPos](const TObjectPtr<AEDU_CORE_SelectableEntity>& A, const TObjectPtr<AEDU_CORE_SelectableEntity>& B)
				{
					const float DistanceA = FVector::Dist(MyPos, A->GetActorLocation());
					const float DistanceB = FVector::Dist(MyPos, B->GetActorLocation());
					return DistanceA < DistanceB; // Compare by distance
				});

			// Remove invalid targets from the array (those after the valid partition)
			TargetArray.RemoveAt(PartitionIndex, TargetArray.Num() - PartitionIndex);
		}
		break;
		
		case ETargetPriority::Farthest:
		{
			FVector MyPos = GetOwner()->GetActorLocation();

			// Sort in-place with validity and distance combined
			Algo::Sort(TargetArray, [&MyPos](const TObjectPtr<AEDU_CORE_SelectableEntity>& A, const TObjectPtr<AEDU_CORE_SelectableEntity>& B)
			{
				// Handle invalid entries as "lesser" to push them to the end
				if (!A) return false; // A is invalid, B is preferred
				if (!B) return true;  // B is invalid, A is preferred

				// Compare distances (reversed order for farthest)
				const float DistanceA = FVector::DistSquared(MyPos, A->GetActorLocation());
				const float DistanceB = FVector::DistSquared(MyPos, B->GetActorLocation());
				return DistanceA > DistanceB; // Reverse comparison
			});

			// Remove invalid targets at the end (avoids multiple passes)
			TargetArray.RemoveAt(TargetArray.FindLastByPredicate([](const TObjectPtr<AEDU_CORE_SelectableEntity>& Target)
			{
				return !Target;
			}), TargetArray.Num());
		}
		break;
	
		case ETargetPriority::LowestHealth:
		break;
		
		case ETargetPriority::HighestHealth:
		break;
		
		case ETargetPriority::LowestDamage:
		break;
		
		case ETargetPriority::HighestDamage:
		break;
		
		case ETargetPriority::LowestDefense:
		break;
		
		case ETargetPriority::HighestDefense:
		break;
		
		case ETargetPriority::Max:
		break;
	default: ;
	}
}

bool UTurretWeaponComponent::HasLineOfSight(const FVector& StartPos, const FVector& EndPos, const TObjectPtr<AEDU_CORE_SelectableEntity>& Target) const
{
	// Define collision parameters (ignore the owner or any specific actors if needed)
	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(GetOwner()); // Ignore the weapon owner (e.g., the player or entity)

	// Perform the line trace
	FHitResult HitResult;
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,        // Out hit result
		StartPos,         // Trace start position
		EndPos,           // Trace end position
		ECC_Visibility,   // Collision channel (visibility channel)
		TraceParams       // Collision query parameters
	);

	// If a hit occurs, check if it's the target or an obstacle
	if (bHit)
	{
		// Check if the hit actor is the target
		if(HitResult.GetActor() && HitResult.GetActor() == Target)
		{
			#if WITH_EDITOR
			//if(bDrawSightDebugLine)
			{
				// Green line indicates confirmation.
				if(!IsInGameThread())
				{
					// If not, queue the function to run on the main thread
					AsyncTask(ENamedThreads::GameThread, [this, StartPos, EndPos]
					{
						DrawDebugLine(GetWorld(), StartPos, EndPos, FColor::Emerald, false, 1.0f, 0, 1.0f);
					});
				}
				else
				{
					DrawDebugLine(GetWorld(), StartPos, EndPos, FColor::Emerald, false, 1.0f, 0, 1.0f);
				}
			}
			#endif
			return true; // The target is hit directly
		}

		// If no hit occurs, check the distance to the target position
		const float DistanceFromImpactSquared = FVector::DistSquared(EndPos, HitResult.ImpactPoint);
		if(DistanceFromImpactSquared < FMath::Square(100.f))
		{
			return true; // Close enough to the target, return true (i.e., line of sight exists)
		}
	}
	
	return false; // No hit and not within range of the target
}