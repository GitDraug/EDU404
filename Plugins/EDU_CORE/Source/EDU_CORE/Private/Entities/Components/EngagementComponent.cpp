// Fill out your copyright notice in the Description page of Project Settings.

// THIS
#include "Entities/Components/EngagementComponent.h"

// CORE
#include "Entities/EDU_CORE_SelectableEntity.h"

#include "Entities/Components/StatusComponent.h"
#include "Entities/Components/FixedWeaponComponent.h"
#include "Entities/Components/TurretWeaponComponent.h"
#include "EntitySystem/MovieSceneEntitySystemRunner.h"

#include "Framework/Data/FLOWLOGS/FLOWLOG_COMPONENTS.h"
#include "Framework/Managers/GameModes/EDU_CORE_GameMode.h"

//------------------------------------------------------------------------------
// Get/Set
//------------------------------------------------------------------------------

void UEngagementComponent::AddToTurretArray(UTurretWeaponComponent* TurretComponent)
{ FLOW_LOG
	if(TurretComponent)
	{
 		TurretArray.AddUnique(TurretComponent); // Adds the pointer to the array
	  	UE_LOG(FLOWLOG_CATEGORY, Display, TEXT("Entity added: %s"), *TurretComponent->GetName());

		// Example of retreiving a weapon directly.
		// FProjectileWeaponInformation& MaxDamageWeapon = TurretComponent->GetAllWeaponsInfo()[TurretComponent->GetMaxDamageArrayIndex()];

		TurretComponent->SetTurretTeam(OurTeam);
	    TurretComponent->EvaluateWeapons();
		if(TurretComponent->GetMaxRange() > MaxRange)
		{
			MaxRange = TurretComponent->GetMaxRange();
		}
		
		if(TurretComponent->GetMaxDamage() > MaxDamage)
		{
			// Stats
			MaxDamage = TurretComponent->GetMaxDamage();
			MaxDamageType = TurretComponent->GetMaxDamageType();
			MaxDamageTurret = TurretComponent;
		}
	}
}

void UEngagementComponent::AddToFixedWeaponsArray(UFixedWeaponComponent* FixedWeapon)
{ FLOW_LOG
	
}

//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------
UEngagementComponent::UEngagementComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UEngagementComponent::OnRegister()
{ FLOW_LOG
	Super::OnRegister();
	
}

void UEngagementComponent::BeginPlay()
{
	Super::BeginPlay();

	// Get all Turrets attached to the actor
	GetOwner()->GetComponents<UTurretWeaponComponent>(TurretArray);

	// Server Tick
	if(HasAuthority())
	{
		if (AEDU_CORE_GameMode* GameModePtr = Cast<AEDU_CORE_GameMode>(GetWorld()->GetAuthGameMode()))
		{
			GameModePtr->AddToEngagementComponentArray(this);
			GameMode = GameModePtr;
		}
	}

	// Ensure that we have a Status Component; this component is useless otherwise
	EnsureStatusComponent();
}

//------------------------------------------------------------------------------
// Aggregated Server tick
//------------------------------------------------------------------------------

void UEngagementComponent::ServerEngagementComponentCalc(float AsyncDeltaTime)
{ // FLOW_LOG

	float SearchRange = 0.f;

	// Are We prioritizing the nearest or furthest target?
	// Are any weapon mounts (Turrets, not weapons) available or interupteble?
	if(TurretArray.Num() > 0)
	{
		for(const TObjectPtr<UTurretWeaponComponent>& Turret : TurretArray)
		{
			// Skip if the turret is null or its status is not enough.
			if (!Turret || Turret->GetTurretStatus() < EWeaponStatus::Ready) { continue; }
                    
			if(const float TurretMaxRange = Turret->GetMaxRange(); TurretMaxRange > SearchRange)
			{
				SearchRange = TurretMaxRange;
			}
		}
	}

	for(const TObjectPtr<UFixedWeaponComponent>& FixedWeapon : FixedWeaponsArray)
	{
		// Skip if the turret is null or its status is not enough.
		if (!FixedWeapon || FixedWeapon->GetFixedWeaponStatus() < EWeaponStatus::Supporting) { continue; }
                    
		if(const float FixedWeaponMaxRange = FixedWeapon->GetMaxRange(); FixedWeaponMaxRange > SearchRange)
		{
			SearchRange = FixedWeaponMaxRange;
		}
	}
	
	if(SearchRange)
	{
		SearchForTargets(SearchRange);
	}
}

void UEngagementComponent::ServerEngagementComponentExec(float AsyncDeltaTime)
{

}

//------------------------------------------------------------------------------
// Functionality
//------------------------------------------------------------------------------

void UEngagementComponent::SearchForTargets(float SearchRange)
{  // FLOW_LOG
	/*---------------------------------------------------------------------
	  This function checks for actors of a certain objectype in a sphere.
	  
	  The actors don't need or have "generate overlap events" enabled,
	  nor do they need to block any specific channel. They only have
	  to be of the object type we search for.

	  #include "Engine/OverlapResult.h"
	---------------------------------------------------------------------*/
	
	// Define object types to check, add more as needed
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(TargetObjectType);
	
	// Team 0 is used here unless a team is specified.
	FCollisionQueryParams CollisionQueryParams;
	const TArray<AActor*>& HiddenActors = GameMode->GetTeamHiddenActorsArray(OurTeam);
	const TArray<AActor*>& TeamActors = GameMode->GetTeamArray(OurTeam);

	CollisionQueryParams.AddIgnoredActors(HiddenActors);
	CollisionQueryParams.AddIgnoredActors(TeamActors);
	
	FQuat Rotation = FQuat::Identity;
	
	FCollisionShape Shape = FCollisionShape::MakeSphere(SearchRange);
	FVector Center = GetOwner()->GetActorLocation();

	// Make ready the DetectedTargetsArray for new targets.
	TArray<FOverlapResult> TargetsInRangeArray;

	// Perform the shape overlap
	GetWorld()->OverlapMultiByObjectType(
		TargetsInRangeArray,		// Array to hold results
		Center,						// Center of the sphere
		Rotation,					// Rotation
		ObjectQueryParams,			// Object types to overlap (e.g., ECC_Destructible)
		Shape,						// Shape to check (FieldOfVisionType)
		CollisionQueryParams		// Collision filter
	);

	// Optional: Draw a debug sphere (only in debug mode)
	#if WITH_EDITOR
		if (bDrawSearchForTargetsDebugShape)
		{
			// Always do this on the main thread.
			AsyncTask(ENamedThreads::GameThread, [this, Center]()
			{
				DrawDebugSphere(GetWorld(), Center, MaxRange, 24, FColor::Emerald, false, 0.5f);
			});
		}
	#endif

	if(TargetsInRangeArray.Num() > 0)
	{
		int8 EffectiveWeapons = 0;
		
		for(TObjectPtr<UTurretWeaponComponent>& Turret : TurretArray)
		{
			EffectiveWeapons += EvaluateTargetsInRange(Turret, TargetsInRangeArray);
		}
		
		for(TObjectPtr<UFixedWeaponComponent>& FixedWeapons : FixedWeaponsArray)
		{
			EffectiveWeapons += EvaluateTargetsInRange(FixedWeapons, TargetsInRangeArray);
		}

		if(EffectiveWeapons > 0) bCombatEffective = true;
	}
}

void UEngagementComponent::EnsureStatusComponent()
{ FLOW_LOG
	if(AActor* Owner = GetOwner())
	{
		StatusComponent = Owner->FindComponentByClass<UStatusComponent>();
		OurTeam = StatusComponent->GetActiveTeam();
		
		if(!StatusComponent)
		{
			FLOW_LOG_ERROR("StatusComponent not detected in owning actor.")
		}
	}
	else
	{
		FLOW_LOG_ERROR("Owner actor is null.")
	}
}

template <typename WeaponComponentType>
int8 UEngagementComponent::EvaluateTargetsInRange(TObjectPtr<WeaponComponentType>& WeaponMount, const TArray<FOverlapResult>& TargetsInRangeArray)
{
	int8 EffectiveWeapons = 0;
	// WeaponMount->ViableTargetsArray.Reset();
	// WeaponMount->PriorityTargetsArray.Reset();
	
	// Cache weapon properties (constant for the loop)
	const TArray<FProjectileWeaponInformation>& WeaponInfoStruct = WeaponMount->GetAllWeaponsInfo();

	// Check each weapon on the WeaponMount
	for (const FProjectileWeaponInformation& Weapon : WeaponInfoStruct)
	{
		// Iterate over all targets in range
		for (const FOverlapResult& OverlapResult : TargetsInRangeArray)
		{
			TObjectPtr<AActor> OverlappingActor = OverlapResult.GetActor();
			if (!OverlappingActor) continue;

			TObjectPtr<AEDU_CORE_SelectableEntity> Target = Cast<AEDU_CORE_SelectableEntity>(OverlappingActor);
			if (!Target) continue;

			TObjectPtr<UEngagementComponent> TargetEngagementComponent = Target->GetEngagementComponent();
			TObjectPtr<UStatusComponent> TargetStatusComponent = Target->GetStatusComponent();
			if (!TargetEngagementComponent || !TargetStatusComponent) continue;
		
			const float TargetDefense = TargetStatusComponent->GetDefenceAgainst(Weapon.DamageType);

			// Viability check
			if(Weapon.Damage < TargetDefense) continue;

			// Mark this weapon as effective and add to viable targets
			EffectiveWeapons++;
			WeaponMount->ViableTargetsArray.AddUnique(Target);

			// Cache threat evaluation properties
			const float TargetDamage = TargetEngagementComponent->GetMaxDamage();
			const EDamageType TargetDamageType = TargetEngagementComponent->GetMaxDamageType();
			const float OurDefense = StatusComponent->GetDefenceAgainst(TargetDamageType);

			// Evaluate if this target is a priority threat
			if(TargetDamage > OurDefense)
			{
				WeaponMount->PriorityTargetsArray.AddUnique(Target);
			}
		}
	}

    // Return the number of effective weapons on this Weapon Platform
    return EffectiveWeapons;
}

