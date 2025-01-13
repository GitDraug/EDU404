// Fill out your copyright notice in the Description page of Project Settings.

// CORE
#include "Entities/Components/TurretWeaponComponent.h"

// CORE
#include "Entities/EDU_CORE_MobileEntity.h"
#include "Entities/Components/EngagementComponent.h"

#include "FunctionLibrary/UtilityLibrary.h"

#include "Framework/Data/FLOWLOGS/FLOWLOG_COMPONENTS.h"
#include "Framework/Managers/GameModes/EDU_CORE_GameMode.h"

//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------
UTurretWeaponComponent::UTurretWeaponComponent()
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
	
	EvaluateMaxDamageWeapon();
	EvaluateMaxRangeWeapon();

	// Register with Owners Engagement Component
	EnsureEngagementComponent();
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
		}
	}
}

//------------------------------------------------------------------------------
// Aggregated Server tick
//------------------------------------------------------------------------------
void UTurretWeaponComponent::ServerTurretCalc(float AsyncDeltaTime)
{
	// if(TargetPosition == FVector::ZeroVector && TargetActor == nullptr) return;
		
	// Check if we have a target
	// Check if the Mount is aligned already (Note that we're using the barrel's position to align properly).
	FRotator BarrelStartRotation = this->GetComponentRotation();
	FRotator BarrelEndRotation = UtilityLibrary::GetRotationToTargetPos(this, FVector(0.f, 0.f, 500.f));

	//------------------------------------------------------------------------------------------------------
	// Mount (Yaw) rotation calculation
	//------------------------------------------------------------------------------------------------------
	// If the missalignment is larger than our tolerence for error, we will align.
	float MountRotationDistance = UtilityLibrary::CalculateRotationDistance(BarrelStartRotation.Yaw, BarrelEndRotation.Yaw, AlignmentTolerance);
	if(FMath::Abs(MountRotationDistance) > 0.1f)
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
	if(FMath::Abs(BarrelRotationDistance) > 0.1f)
	{
		BarrelTurnRate = UtilityLibrary::CalculateFastestTurnRate(BarrelRotationSpeed, BarrelRotationDistance, BarrelSlowRotationDistance);

		// Check bounds
		if(BarrelEndRotation.Pitch <= BarrelMaxElevation && BarrelEndRotation.Pitch >= BarrelMinElevation)
		{
			bBarrelShouldAlign = true;
		}
		else
		{
			// If the target is out of bounds, we should pick another target.
		}
	}
	else
	{
		bBarrelShouldAlign = false;
	}
		
	// GEngine->AddOnScreenDebugMessage(31, GetWorld()->DeltaTimeSeconds, FColor::Cyan, 
	// FString::Printf(TEXT("BarrelStartRotation: %f"), BarrelStartRotation.Pitch));

	// GEngine->AddOnScreenDebugMessage(32, GetWorld()->DeltaTimeSeconds, FColor::Cyan, 
	// FString::Printf(TEXT("BarrelEndRotation: %f"), BarrelEndRotation.Pitch));
    	
	// GEngine->AddOnScreenDebugMessage(33, GetWorld()->DeltaTimeSeconds, FColor::Cyan, 
	// FString::Printf(TEXT("BarrelRotationDistance: %f"), BarrelRotationDistance));
}

void UTurretWeaponComponent::ServerTurretExec(float AsyncDeltaTime)
{

}

void UTurretWeaponComponent::ServerTimeGatedTurretExec(float AsyncDeltaTime)
{

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

void UTurretWeaponComponent::EvaluateMaxDamageWeapon()
{
	// Loop through each weapon in the array
	for (FProjectileWeaponInformation& Weapon : WeaponStructArray)
	{
		// Skip weapons with no ammo
		if (Weapon.CurrentAmmo <= 0)
		{
			continue;  // Move to the next weapon
		}

		// Loop through each damage type and amount of the current weapon
		for (const TPair<EDamageType, float>& Damage : Weapon.DamageTypeAndAmount)
		{
			// Check if this weapon's damage exceeds the current MaxDamage
			if (Damage.Value > MaxDamage)
			{
				// MaxDamage Stats
				MaxDamage = Damage.Value;
				MaxDamageType = Damage.Key;
				MaxDamageAmmo = Weapon.CurrentAmmo;
				MaxDamageRange = Weapon.MaxDistance;

				// Weapon Reference for fast lookup
				MaxDamageWeaponID = Weapon.WeaponID;
				MaxDamageArrayIndex = Weapon.WeaponArrayIndex;
			}
		}
	}
}

void UTurretWeaponComponent::EvaluateMaxRangeWeapon()
{
	// Loop through each weapon in the array
	for (FProjectileWeaponInformation& Weapon : WeaponStructArray)
	{
		// Skip weapons with no ammo
		if (Weapon.CurrentAmmo <= 0)
		{
			continue;  // Move to the next weapon
		}

		// Loop through each damage type and amount of the current weapon
		for (const TPair<EDamageType, float>& Damage : Weapon.DamageTypeAndAmount)
		{
			// Check if this weapon's damage exceeds the current MaxDamage
			if (Damage.Value > MaxDamage)
			{
				// MaxRange Stats
				// MaxRangeDamage = Damage.Value;
				// MaxRangeDamageType = Damage.Key;
				// MaxRangeDamageAmmo = Weapon.CurrentAmmo;
				MaxRange = Weapon.MaxDistance;

				// Weapon Reference for fast lookup
				MaxRangeWeaponID = Weapon.WeaponID;
				MaxRangeArrayIndex = Weapon.WeaponArrayIndex;
			}
		}
	}
}