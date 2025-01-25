// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// CORE
#include "Framework/Data/DataTypes/EDU_CORE_DataTypes.h"

// UE
#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"

// THIS
#include "TurretWeaponComponent.generated.h"

class UEngagementComponent;

/*------------------------------------------------------------------------------
  Turret Wepon component
--------------------------------------------------------------------------------
  Acts as a turret barrel wich logic for a weapon to fire from.

  This component will hijack and control whatever static mesh it is attached to,
  turning that mesh into a Turret Mount Mesh.
------------------------------------------------------------------------------*/

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EDU_CORE_API UTurretWeaponComponent : public UStaticMeshComponent
{
	GENERATED_BODY()
	
//------------------------------------------------------------------------------
// Get/Set
//------------------------------------------------------------------------------
public:
	
	// Returns all weapons on this turret
	FORCEINLINE TArray<FProjectileWeaponInformation>& GetAllWeaponsInfo() { return WeaponStructArray; }; 

	// Checks the status of this Turret
	FORCEINLINE EWeaponStatus GetTurretStatus() const { return TurretStatus; }

	// Can be used by opposing team to hijack individual Turrets. 
	FORCEINLINE void SetTurretTeam(const EEDU_CORE_Team NewTeam) { OurTeam = NewTeam; };

	//-------------------------------
	// Max Stat Info
	//-------------------------------
	
	FORCEINLINE float		GetMaxRange()		const { return MaxRange; }
	FORCEINLINE float		GetMaxDamage()		const { return MaxDamage; }
	FORCEINLINE EDamageType GetMaxDamageType()	const { return MaxDamageType; }

	// Updates Max Damage Weapon
	void EvaluateWeapons();

//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------
public:
	
	UTurretWeaponComponent(const FObjectInitializer& ObjectInitializer);

protected:
	
	// Called when a component is registered, after Scene is set, but before CreateRenderState_Concurrent or OnCreatePhysicsState are called.
	virtual void OnRegister() override;
	
	// Called when the game starts
	virtual void BeginPlay() override;

//------------------------------------------------------------------------------
// Aggregated Server tick
//------------------------------------------------------------------------------
public:

	// Runs 1/Frame
	virtual void ServerTurretCalc(float AsyncDeltaTime);

	// Runs 1/50 Frame
	virtual void ServerTurretExec(float AsyncDeltaTime);

	// Runs 1/s
	virtual void ServerTimeGatedTurretExec(float AsyncDeltaTime);

//------------------------------------------------------------------------------
// Editable Data: General
//------------------------------------------------------------------------------
protected:
	
	// Turret rotation speed in degrees per second when not slowed down.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
	float MountRotationSpeed = 36.f;
	
	// At what distance should we adjust speed from full to complete stop?
	// At 180 the rotation will take roughly 5 times longer.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
	float MountSlowRotationDistance = 0.f;
	
	// How much tolerance for alignment error does this turret have?
	// Weapons that deal with a lot of splash damage care less for aiming.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barrel")
	float AlignmentTolerance = 5.f;
	
	// Turret rotation speed in degrees per second when not slowed down.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barrel")
	float BarrelRotationSpeed = 36.f;
	
	// At what distance should we adjust speed from full to complete stop?
	// At 180 the rotation will take roughly 5 times longer.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barrel")
	float BarrelSlowRotationDistance = 0.f;
	
	// Maximum elevation angle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barrel")
	float BarrelMaxElevation = 35.f;

	// Minimum elevation angle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barrel")
	float BarrelMinElevation = -35.f;
	
//------------------------------------------------------------------------------
// Components
//------------------------------------------------------------------------------
protected:

	// Cache the team we're on
	UPROPERTY(VisibleAnywhere, Category = "Engagement | Targets")
	EEDU_CORE_Team OurTeam = EEDU_CORE_Team::None;
	
	// Status of this turret
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EWeaponStatus TurretStatus = EWeaponStatus::Ready;
	
	// Array of WeaponStructs
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons")
	TArray<FProjectileWeaponInformation> WeaponStructArray;
	
	// Target Position for AttackGround commands, such as Supressing Fire
	UPROPERTY(VisibleAnywhere, Category = "Targets")
	FVector GroundTargetPosition = FVector::ZeroVector;
	
	// the position wher ethe target was last seen.
	UPROPERTY(VisibleAnywhere, Category = "Targets")
	FVector LastKnownTargetPosition = FVector::ZeroVector;
	
	// Pointer to EngagementComponent
	UPROPERTY()
	TObjectPtr<UEngagementComponent> EngagementComponent = nullptr;

	// Cached references to the turretMount that the barrel attaches to.
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> TurretMountMesh;

	/*----------------------------- Targets ------------------------------
	  We can hurt ViableTargets with at least one of our weapons,
	  Priority Targets can hit us with at least one of their weapons.
	---------------------------------------------------------------------*/
	public:

	// Counts how many seconds since we lost our Target.
	UPROPERTY(VisibleAnywhere, Category = "Targets")
	int32 NoTargetTimer = 0;

	// What to target first
	UPROPERTY(EditAnywhere, Category = "Targets")
	ETargetPriority TargetPriority = ETargetPriority::Nearest;
	
	// Pointer to current TargetEntity
	UPROPERTY(VisibleAnywhere, Category = "Targets")
	TObjectPtr<AEDU_CORE_SelectableEntity> TargetEntity = nullptr;
	
	// Array of targets we can damage.
	UPROPERTY(VisibleAnywhere, Category = "Targets")
	TArray<TObjectPtr<AEDU_CORE_SelectableEntity>> ViableTargetsArray;

	// Array of targets we can damage, that can also hurt us.
	UPROPERTY(VisibleAnywhere, Category = "Targets")
	TArray<TObjectPtr<AEDU_CORE_SelectableEntity>> PriorityTargetsArray;

	/*---------------------- Max Damage Weapon Info -----------------------
	  This is used by the EngagementComponent and other Tactical agents
	  to make decisions. Damage is the most important stat, because if a
	  weapon can't penetrate the target's armor, no other stat matters.
	---------------------------------------------------------------------*/
	protected:
	
	// Cached: ID of the most potent weapon on this Turret.
	UPROPERTY()
	FGuid MaxDamageWeaponID;
	
	// Cached: Damage Amount of the most potent weapon on this Turret.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapons")
	EDamageType MaxDamageType = EDamageType::EDT_None;

	// Cached: Damage Type of the most potent weapon on this Turret.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapons")
	float MaxDamage = 0.f;

	
	/*---------------------- Max Range Weapon Info -----------------------
	  This is used by the EngagementComponent and other Tactical agents
	  to make decisions. Range is the second most important stat because
	  it allows us to engage targets without moving.
	---------------------------------------------------------------------*/

	UPROPERTY()
	FGuid MaxRangeWeaponID;
	
	UPROPERTY()
	int32 MaxRangeArrayIndex = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapons")
	float MaxRange = 0.f;

	//----------------------------------
	// Turret Alignment
	//----------------------------------
	
	bool bMountShouldAlign = false;
	float MountTurnRate = 5.f;

	bool bBarrelShouldAlign = false;
	float BarrelTurnRate = 5.f;
	
	//----------------------------------
	// Debug
	//----------------------------------
	
	// float debug_CurrentPitch = 0.f;
	// float debug_TargetPitch = 0.f;

	// float debug_PitchDifferene = 0.f;

//------------------------------------------------------------------------------
// Functionality
//------------------------------------------------------------------------------
protected:	

//------------------------------------------------------------------------------
// Functionality > Setup
//------------------------------------------------------------------------------
protected:
	
	// Ensure that we have an EngagementComponent active. This component will not function properly without it.
	void EnsureEngagementComponent();

	void SortTargets(TArray<TObjectPtr<AEDU_CORE_SelectableEntity>>& TargetArray) const;
	
	bool HasLineOfSight(const FVector& StartPos, const FVector& EndPos,	const TObjectPtr<AEDU_CORE_SelectableEntity>& Target) const;
	
};
