// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// CORE
#include "Framework/Data/DataTypes/EDU_CORE_DataTypes.h"

// UE
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

// THIS
#include "FixedWeaponComponent.generated.h"

class UEngagementComponent;
class AEDU_CORE_MobileEntity;

/*------------------------------------------------------------------------------
  Fixed Weapon Component
--------------------------------------------------------------------------------
  Acts as an abstract barrel wich logic for a weapon to fire from.

  This component is intended to be used with a MobileEntity, which acts as a
  mount for YawMovement, while this component can still pitch. The component
  is pure data and logic, no graphics are involved.
------------------------------------------------------------------------------*/

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EDU_CORE_API UFixedWeaponComponent : public UActorComponent
{
	GENERATED_BODY()
	
//------------------------------------------------------------------------------
// Get/Set
//------------------------------------------------------------------------------
public:

	// Checks the status of this Weapon Mount
	FORCEINLINE EWeaponStatus	GetFixedWeaponStatus()	const	{ return FixedWeaponStatus; }

	// Returns all weapons on this Weapon Mount
	FORCEINLINE TArray<FProjectileWeaponInformation>& GetAllWeaponsInfo() { return WeaponStructArray; }; 

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
	
	// Sets default values for this component's properties
	UFixedWeaponComponent(const FObjectInitializer& ObjectInitializer);

protected:

	// Called when a component is registered, after Scene is set, but before CreateRenderState_Concurrent or OnCreatePhysicsState are called.
	virtual void OnRegister() override;
	
	// Called when the game starts
	virtual void BeginPlay() override;

//------------------------------------------------------------------------------
// Aggregated Server tick
//------------------------------------------------------------------------------
public:

	void ServerFixedWeaponCalc(float AsyncDeltaTime);
	void ServerFixedWeaponExec(float AsyncDeltaTime);
	void ServerTimeGatedFixedWeaponExec(float AsyncDeltaTime);

//------------------------------------------------------------------------------
// Editable Data: General
//------------------------------------------------------------------------------
protected:

	// How much tolerance for alignment error does this turret have?
	// Weapons that deal with a lot of splash damage care less for aiming.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barrel")
	float AlignmentTolerance = 5.f;
	
	// Maximum elevation angle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barrel")
	float BarrelMaxElevation = 90.f;

	// Minimum elevation angle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barrel")
	float BarrelMinElevation = -90.f;

//------------------------------------------------------------------------------
// Components
//------------------------------------------------------------------------------
protected:

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Weapons")
	EWeaponStatus FixedWeaponStatus = EWeaponStatus::Ready;
	
	// Array of WeaponStructs
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons")
	TArray<FProjectileWeaponInformation> WeaponStructArray;

	// Pointer to current TargetActor
	UPROPERTY(VisibleAnywhere, Category = "Target")
	TObjectPtr<AActor> TargetActor = nullptr;
	
	// Target Position for AttackGround commands, such as Supressing Fire
	UPROPERTY(VisibleAnywhere, Category = "Target")
	FVector TargetPosition = FVector::ZeroVector;

	// Pointer to the User of this component
	UPROPERTY(VisibleAnywhere, Category = "MobileEntity")
	TObjectPtr<AEDU_CORE_MobileEntity> MobileEntity = nullptr;
	
	// Pointer to EngagementComponent
	UPROPERTY()
	TObjectPtr<UEngagementComponent> EngagementComponent = nullptr;

	/*----------------------------- Targets ------------------------------
	  We can hurt ViableTargets with at least one of our weapons,
	  Priority Targets can hit us with at least one of their weapons.
	---------------------------------------------------------------------*/
	public:
	
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

	void SortTargets(ETargetPriority InTargetPriority, TArray<TObjectPtr<AEDU_CORE_SelectableEntity>> TargetArray) const;

	bool HasLineOfSight(const FVector& Vector, const FVector& TargetPos, const TObjectPtr<AEDU_CORE_SelectableEntity>& Target) const;
};
