// Fill out your copyright notice in the Description page of Project Settings.

// THIS
#include "Entities/Components/FixedWeaponComponent.h"

// CORE
#include "Entities/EDU_CORE_MobileEntity.h"
#include "Framework/Data/FLOWLOGS/FLOWLOG_COMPONENTS.h"
#include "Entities/Components/EngagementComponent.h"
#include "Framework/Managers/GameModes/EDU_CORE_GameMode.h"

//------------------------------------------------------------------------------
// Get/Set
//------------------------------------------------------------------------------

UFixedWeaponComponent::UFixedWeaponComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{ // FLOW_LOG
	PrimaryComponentTick.bCanEverTick = false;
}

void UFixedWeaponComponent::OnRegister()
{
	Super::OnRegister();

	// Clients need WeaponInfo for UI.
	SetIsReplicated(true);
	
	// Find whatever component this component is attached to and save it.
	MobileEntity = Cast<AEDU_CORE_MobileEntity>(GetOwner());
	if(MobileEntity)
	{
		MobileEntity->SetAlignWithTarget(true);
		FLOW_LOG_WARNING("FixedWeaponComponent is Active!")
	}
	else
	{
		FLOW_LOG_ERROR("Owner is not a Mobile Entity! This will result in unwanted behaviour!")
	}
	
}

void UFixedWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	// Server Tick
	if(GetNetMode() != NM_Client)
	{
		if (AEDU_CORE_GameMode* GameMode = Cast<AEDU_CORE_GameMode>(GetWorld()->GetAuthGameMode()))
		{
			GameMode->AddToFixedWeaponComponentArray(this);

			EvaluateWeapons();

			// Register with Owners Engagement Component
			EnsureEngagementComponent();
		}
	}
}

//------------------------------------------------------------------------------
// Aggregated Server tick
//------------------------------------------------------------------------------

void UFixedWeaponComponent::ServerFixedWeaponCalc(float AsyncDeltaTime)
{
	// DummyTarget
	// TODO: Remember to remove this.
	// TargetPosition = FVector(0.f, 0.f, 400.f);
	if(TargetPosition != FVector::ZeroVector)
	{
		MobileEntity->SetTargetPosition(TargetPosition);
	}	
}

void UFixedWeaponComponent::ServerFixedWeaponExec(float AsyncDeltaTime)
{
}


void UFixedWeaponComponent::ServerTimeGatedFixedWeaponExec(float AsyncDeltaTime)
{
	if(PriorityTargetsArray.Num() > 0)
	{
		SortTargets(TargetPriority, PriorityTargetsArray);

		const FVector MyPos = GetOwner()->GetActorLocation();
		for(const TObjectPtr<AEDU_CORE_SelectableEntity>& Target: PriorityTargetsArray)
		{
			FVector TargetPos = Target->GetActorLocation();
			if(FVector::DistSquared(MyPos,Target->GetActorLocation()) > MaxRange) continue;
			
			if(!HasLineOfSight(MyPos, TargetPos, Target)) continue;

			TargetEntity = Target;
			FixedWeaponStatus = EWeaponStatus::Engaged;
			return;
		}
	}
	else
	{
		if(ViableTargetsArray.Num() > 0)
		{
			SortTargets(TargetPriority, ViableTargetsArray);

			const FVector MyPos = GetOwner()->GetActorLocation();
			for(const TObjectPtr<AEDU_CORE_SelectableEntity>& Target: PriorityTargetsArray)
			{
				FVector TargetPos = Target->GetActorLocation();
				if(FVector::DistSquared(MyPos,Target->GetActorLocation()) > MaxRange) continue;
				
				if(!HasLineOfSight(MyPos, TargetPos, Target)) continue;

				TargetEntity = Target;
				FixedWeaponStatus = EWeaponStatus::Supporting;
				return;
			}
		}
	}

	if(FixedWeaponStatus > EWeaponStatus::Ready)
	FixedWeaponStatus = EWeaponStatus::Ready;
}

//------------------------------------------------------------------------------
// Functionality > Setup
//------------------------------------------------------------------------------

void UFixedWeaponComponent::EnsureEngagementComponent()
{
	if(GetOwner())
	{
		EngagementComponent = GetOwner()->FindComponentByClass<UEngagementComponent>();
		
		if(EngagementComponent)
		{
			EngagementComponent->AddToFixedWeaponsArray(this);
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

void UFixedWeaponComponent::EvaluateWeapons()
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

void UFixedWeaponComponent::SortTargets(const ETargetPriority InTargetPriority, TArray<TObjectPtr<AEDU_CORE_SelectableEntity>> TargetArray) const
{
	switch (TargetPriority)
	{
		case ETargetPriority::Nearest:
		{
			FVector MyPos = GetOwner()->GetActorLocation();

			// Sort in-place with validity and distance combined
			Algo::Sort(TargetArray, [&MyPos](const TObjectPtr<AEDU_CORE_SelectableEntity>& A, const TObjectPtr<AEDU_CORE_SelectableEntity>& B)
			{
				// Handle invalid entries as "greater" to push them to the end
				if(!A) return false; // A is invalid, B is preferred
				if(!B) return true;  // B is invalid, A is preferred

				// Compare distances
				const float DistanceA = FVector::DistSquared(MyPos, A->GetActorLocation());
				const float DistanceB = FVector::DistSquared(MyPos, B->GetActorLocation());
				return DistanceA < DistanceB;
			});

			// Remove invalid targets at the end (avoids multiple passes)
			TargetArray.RemoveAt(TargetArray.FindLastByPredicate([](const TObjectPtr<AEDU_CORE_SelectableEntity>& Target)
			{
				return !Target;
			}), TargetArray.Num());
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

bool UFixedWeaponComponent::HasLineOfSight(const FVector& StartPos, const FVector& EndPos, const TObjectPtr<AEDU_CORE_SelectableEntity>& Target) const
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
