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

UENUM(BlueprintType)
enum class ETurretStatus : uint8
{
	Disabled		UMETA(DisplayName = "Disabled"),
	Damaged			UMETA(DisplayName = "Damaged"),
	Engaged			UMETA(DisplayName = "Engaged"),
	Supporting		UMETA(DisplayName = "Supporting "),
	Ready			UMETA(DisplayName = "Ready"),

	Max        UMETA(Hidden)
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EDU_CORE_API UTurretWeaponComponent : public UStaticMeshComponent
{
	GENERATED_BODY()
	
//------------------------------------------------------------------------------
// Get/Set
//------------------------------------------------------------------------------
public:
	
	// Evaluate weapon systems and determine the weapon with the highest damage
	FORCEINLINE TArray<FProjectileWeaponInformation>& GetAllWeaponsInfo() { return WeaponStructArray; }; 
	
	FORCEINLINE ETurretStatus GetTurretStatus() const { return TurretStatus; }
	
	//-------------------------------
	// Max Range Info
	//-------------------------------
	
	// MaxRange Stats
	FORCEINLINE float		GetMaxRange() const { return MaxRange; }
	//FORCEINLINE int32		GetMaxRangeAmmo() const { return MaxRangeAmmo; }
	//FORCEINLINE float		GetMaxRangeDamage() const { return MaxRangeDamage; }
	//FORCEINLINE EDamageType GetMaxRangeDamageType() const { return MaxRangeDamageType; }

	// Weapon Reference for fast lookup
	// FORCEINLINE FGuid		GetMaxRangeWeaponID() const { return MaxRangeWeaponID; }
	// FORCEINLINE int32		GetMaxRangeArrayIndex() const { return MaxRangeArrayIndex; }

	void EvaluateMaxRangeWeapon();
	
	//-------------------------------
	// Max Damage Info
	//-------------------------------
	
	// MaxDamage Stats
	FORCEINLINE float		GetMaxDamage() const { return MaxDamage; }
	FORCEINLINE EDamageType GetMaxDamageType() const { return MaxDamageType; }
	FORCEINLINE int32		GetMaxDamageAmmo() const { return MaxDamageAmmo; }
	FORCEINLINE float		GetMaxDamageRange() const { return MaxDamageRange; }

	// Weapon Reference for fast lookup
	FORCEINLINE FGuid		GetMaxDamageWeaponID() const { return MaxDamageWeaponID; }
	FORCEINLINE int32		GetMaxDamageArrayIndex() const { return MaxDamageArrayIndex; }
	
	// Updates Max Damage Weapon
	void EvaluateMaxDamageWeapon();
	
//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------
public:
	UTurretWeaponComponent();

protected:
	
	// Called when a component is registered, after Scene is set, but before CreateRenderState_Concurrent or OnCreatePhysicsState are called.
	virtual void OnRegister() override;
	
	// Called when the game starts
	virtual void BeginPlay() override;

//------------------------------------------------------------------------------
// Aggregated Server tick
//------------------------------------------------------------------------------
public:
	
	virtual void ServerTurretCalc(float AsyncDeltaTime);
	virtual void ServerTurretExec(float AsyncDeltaTime);
	virtual void ServerTimeGatedTurretExec(float AsyncDeltaTime);

//------------------------------------------------------------------------------
// Editable Data: General
//------------------------------------------------------------------------------
protected:
	
	// Status of this turret
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ETurretStatus TurretStatus = ETurretStatus::Ready;
	
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
	
	// Array of WeaponStructs
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons")
	TArray<FProjectileWeaponInformation> WeaponStructArray;

	// Pointer to current TargetActor
	UPROPERTY(VisibleAnywhere, Category = "Target")
	TObjectPtr<AActor> TargetActor = nullptr;
	
	// Target Position for AttackGround commands, such as Supressing Fire
	UPROPERTY(VisibleAnywhere, Category = "Target")
	FVector GroundTargetPosition = FVector::ZeroVector;
	
	// Pointer to EngagementComponent
	UPROPERTY()
	TObjectPtr<UEngagementComponent> EngagementComponent = nullptr;

	// Cached references to the turret that the barrel attaches to.
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> TurretMountMesh;

	/*---------------------- Max Damage Weapon Info -----------------------
	  This is used by the EngagementComponent and other Tactical agents
	  to make decisions. Damage is the most important stat, because
	  if a weapon can't penetrate the target's armor, then no other stat
	  matters.
	---------------------------------------------------------------------*/

	// Cached: ID of the most potent weapon on this Turret.
	UPROPERTY()
	FGuid MaxDamageWeaponID;
	
	// The Mount Array Index of the most potent weapon on this Turret.
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Characteristics")
	int32 MaxDamageArrayIndex = 0;
	
	// Cached: Damage Amount of the most potent weapon on this Turret.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapons")
	EDamageType MaxDamageType = EDamageType::EDT_None;

	// Cached: Range of the most potent weapon on this Turret.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapons")
	float MaxDamageRange = 0.f;

	// Cached: Damage Type of the most potent weapon on this Turret.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapons")
	float MaxDamage = 0.f;

	// Cached: Damage Type of the most potent weapon on this Turret.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapons")
	int32 MaxDamageAmmo = 0.f;

	
	/*---------------------- Max Range Weapon Info -----------------------
	  This is used by the EngagementComponent and other Tactical agents
	  to make decisions. Range is the second most important stat, because
	  it allows us to engage targets without moving.
	---------------------------------------------------------------------*/

	UPROPERTY()
	float MaxRange = 0.f;

	UPROPERTY()
	FGuid MaxRangeWeaponID;

	UPROPERTY()
	int32 MaxRangeArrayIndex = 0;
	
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
	float debug_CurrentPitch = 0.f;
	float debug_TargetPitch = 0.f;

	float debug_PitchDifferene = 0.f;

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

	
	
};
