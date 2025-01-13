// Fill out your copyright notice in the Description page of Project Settings.

// THIS
#include "Entities/Components/EngagementComponent.h"

// CORE
#include "Entities/EDU_CORE_SelectableEntity.h"
#include "Entities/Components/StatusComponent.h"
#include "Entities/Components/TurretWeaponComponent.h"
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

	    TurretComponent->EvaluateMaxDamageWeapon();
		if(TurretComponent->GetMaxRange() > MaxRange)
		{
			MaxRange = TurretComponent->GetMaxRange();
		}
		
		if(TurretComponent->GetMaxDamage() > MaxDamage
		&& TurretComponent->GetMaxDamageAmmo() > 0)
		{
			// Stats
			MaxDamage = TurretComponent->GetMaxDamage();
			MaxDamageType = TurretComponent->GetMaxDamageType();
			MaxDamageRange = TurretComponent->GetMaxDamageRange();

			// Weapon Reference for fast lookup
			MaxDamageTurret = TurretComponent;
			WeaponArrayIndex = TurretComponent->GetMaxDamageArrayIndex();
			MaxDamageWeaponID = TurretComponent->GetMaxDamageWeaponID();
		}
		
	    using enum ESimultainiusTurrets;
		if(!bHasFixedWeapons && TurretArray.Num() == 1) 
		{
			SimultainiusTurrets = SingleTurret;
		}
		
		else if(bHasFixedWeapons && TurretArray.Num() == 1)
		{
			SimultainiusTurrets = MultiTurret;
		}
		else if(TurretArray.Num() >= 2)
		{
			SimultainiusTurrets = MultiTurret;
		}
	}
}

void UEngagementComponent::AddToFixedWeaponsArray(UFixedWeaponComponent* FixedWeapon)
{ FLOW_LOG
	
}

//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------
UEngagementComponent::UEngagementComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UEngagementComponent::OnRegister()
{ FLOW_LOG
	Super::OnRegister();
	
	// Ensure that we have a Status Component; this component is useless otherwise
	EnsureStatusComponent();
	
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
}

//------------------------------------------------------------------------------
// Aggregated Server tick
//------------------------------------------------------------------------------

void UEngagementComponent::ServerEngagementComponentCalc(float AsyncDeltaTime)
{ // FLOW_LOG
	// Temp Array
	TArray<TObjectPtr<UTurretWeaponComponent>> AvailablePlatforms;

	// Only run this if we have 2 or more Platforms to evaluate.
	switch(SimultainiusTurrets)
	{
		case ESimultainiusTurrets::SingleTurret:
		break;
		
		case ESimultainiusTurrets::MultiTurret:
			
			for(const TObjectPtr<UTurretWeaponComponent>& WeaponPlatform : TurretArray)
			{
				// Ensure the platform is valid and has the correct status
				if (WeaponPlatform && WeaponPlatform->GetTurretStatus() >= ETurretStatus::Supporting)
				{
					// Add the valid element to the new array
					AvailablePlatforms.AddUnique(WeaponPlatform); 
				}
			}

			// No need to continue if no weapon platforms are available, 
			if(AvailablePlatforms.Num() == 0) return;

			// Search and list Targets witthin our Maximum Weapon Radius
			SearchForTargets();
		
			// No need to continue if no Targets are available, 
			if(TargetArray.Num() == 0) return;

			for(const TObjectPtr<UTurretWeaponComponent>& WeaponPlatform : AvailablePlatforms)
			{
				float AttackerDamage = WeaponPlatform->GetMaxDamage();
				float AttackerRange = WeaponPlatform->GetMaxDamageRange();
				EDamageType AttackerDamageType = WeaponPlatform->GetMaxDamageType();
				
				// Get the Best Target
				GetPriorityTarget(AttackerDamageType, AttackerDamage, AttackerRange);
			}
		break;
		
	default: ;
	}
}

void UEngagementComponent::ServerEngagementComponentExec(float AsyncDeltaTime)
{
}

//------------------------------------------------------------------------------
// Functionality
//------------------------------------------------------------------------------

void UEngagementComponent::SearchForTargets()
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
	ObjectQueryParams.AddObjectTypesToQuery(TargetObjectType);

	// Team 0 is used here unless a team is specified.
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActors(GameMode->GetTeamHiddenActorsArray(OurTeam));
	CollisionQueryParams.AddIgnoredActors(GameMode->GetTeamArray(OurTeam));

	FQuat Rotation = FQuat::Identity;
	
	FCollisionShape Shape = FCollisionShape::MakeSphere(MaxRange);
	FVector Center = GetOwner()->GetActorLocation();

	// Make ready the TargetArray for new targets.
	TargetArray.Reset();
	
	// Perform the shape overlap
	GetWorld()->OverlapMultiByObjectType(
		TargetArray,			// Array to hold results
		Center,					// Center of the sphere
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
			AsyncTask(ENamedThreads::GameThread, [this, Center]()
			{
				DrawDebugSphere(GetWorld(), Center, MaxRange, 24, FColor::Orange, false, 0.5f);
			});
		}
	#endif
}

TObjectPtr<AEDU_CORE_SelectableEntity> UEngagementComponent::GetPriorityTarget(EDamageType DamageType, float Damage, float Range, ETargetPriority Priority)
{
	for (const FOverlapResult& OverlapResult : TargetArray)
	{
		if(AActor* OverlappingActor = OverlapResult.GetActor())
		{
			if (AEDU_CORE_SelectableEntity* Target = Cast<AEDU_CORE_SelectableEntity>(OverlappingActor))
			{
				if (UStatusComponent* TargetStatusComponent = Target->GetStatusComponent())
				{
					// Only select the target if its defense is less than the specific damage type
					if (Damage > TargetStatusComponent->GetDefenceAgainst(DamageType))
					{
						/*------------------------------------------------------------------
						  Here we should add all available targets to new array and sort
						  that after priority. For now, we will return early.
						------------------------------------------------------------------*/
						return Target;
					}
				}
			}
		}
	}
	
	return nullptr;
}

void UEngagementComponent::EnsureStatusComponent()
{ FLOW_LOG
	if(AActor* Owner = GetOwner())
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
