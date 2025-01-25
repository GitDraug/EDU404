// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// CORE
#include "Framework/Data/DataTypes/EDU_CORE_DataTypes.h"

// UE
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/OverlapResult.h" // USed for sweeping

// THIS
#include "EngagementComponent.generated.h"

class AEDU_CORE_GameMode;
class UStatusComponent;
class UFixedWeaponComponent;
class UTurretWeaponComponent;
class AEDU_CORE_SelectableEntity;

/*------------------------------------------------------------------------------
  Engagement Component
--------------------------------------------------------------------------------
  Manages Engagement Behavior such as Target Priority, Fire Solutions and other
  general offencive compabilities.
 
  Each Turret or WeaponComponent acts as a separate component; therefore,
  settings should not be configured in the EngagementComponent but in each
  Weapon/Turret Component separately.
------------------------------------------------------------------------------*/

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EDU_CORE_API UEngagementComponent : public UActorComponent
{
	GENERATED_BODY()

//------------------------------------------------------------------------------
// Get/Set
//------------------------------------------------------------------------------
public:
	// Add a turret to TurretArray
	void AddToTurretArray(UTurretWeaponComponent* TurretComponent);

	// Add a Fixed Weapons to TurretArray
	void AddToFixedWeaponsArray(UFixedWeaponComponent* FixedWeapon);

	FORCEINLINE float		GetMaxDamage()		const { return MaxDamage; }
	FORCEINLINE EDamageType GetMaxDamageType()	const { return MaxDamageType; }
	
//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------
public:
	// Sets default values for this component's properties
	UEngagementComponent(const FObjectInitializer& ObjectInitializer);
	
	virtual void OnRegister() override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

//------------------------------------------------------------------------------
// Aggregated Server tick
//------------------------------------------------------------------------------
public:
	
	virtual void ServerEngagementComponentCalc(float AsyncDeltaTime);
	virtual void ServerEngagementComponentExec(float AsyncDeltaTime);

//------------------------------------------------------------------------------
// Editable Data: General
//------------------------------------------------------------------------------
protected:
	
	UPROPERTY(EditAnywhere)
	bool bDrawSearchForTargetsDebugShape = false;
		
	// What object tyoe are we searching for?
	UPROPERTY(EditAnywhere)
	TEnumAsByte<ECollisionChannel> TargetObjectType = ECC_Pawn ;

//------------------------------------------------------------------------------
// Components
//------------------------------------------------------------------------------
protected:
	
	// Array of available Turrets.
	UPROPERTY(VisibleAnywhere, Category = "Engagement | Weapon Platforms")
	TArray<TObjectPtr<UTurretWeaponComponent>> TurretArray;

	// Array of Fixed Weapons Turrets.
	UPROPERTY(VisibleAnywhere, Category = "Engagement | Weapon Platforms")
	TArray<TObjectPtr<UFixedWeaponComponent>> FixedWeaponsArray;

	// Array of targets we can damage.
	UPROPERTY(VisibleAnywhere, Category = "Engagement | Targets")
	TArray<TObjectPtr<AEDU_CORE_SelectableEntity>> ViableTargetsArray;

	// Array of targets we can damage, that can also hurt us.
	UPROPERTY(VisibleAnywhere, Category = "Engagement | Targets")
	TArray<TObjectPtr<AEDU_CORE_SelectableEntity>> PriorityTargetArray;
	
	// Pointer to GameMode for easy access to Team Arrays
	UPROPERTY()
	TObjectPtr<AEDU_CORE_GameMode> GameMode = nullptr;

	// Pointer to StatusComponent
	UPROPERTY()
	TObjectPtr<UStatusComponent> StatusComponent;
	
	/*--------------------- Combat Effectiveness -------------------------
	  There is no use engaging the neemy if this entity doesn't have any
	  weapons that can hurt the enemy.
	--------------------------------------------------------------------*/
	
	UPROPERTY(VisibleAnywhere, Category = "Engagement | Targets")
	bool bCombatEffective = true;

	// Cache the team we're on
	UPROPERTY(VisibleAnywhere, Category = "Engagement | Targets")
	EEDU_CORE_Team OurTeam = EEDU_CORE_Team::None;

	// Target Sorting Priority
	UPROPERTY(VisibleAnywhere, Category = "Engagement | Targets")
	ETargetPriority TargetPriority = ETargetPriority::Nearest;

	/*--------------------- Max Weapon Damage Info -----------------------
	  This is used by Tactical agents to make decisions.

	  Damage is the most important stat, because if a weapon can't
	  penetrate the target's armor, then no other stat matters.
	--------------------------------------------------------------------*/
	
	// The Turret with most Damage in the Array
	UPROPERTY()
	TObjectPtr<UTurretWeaponComponent> MaxDamageTurret = nullptr;

	// The Mount Array Index of this weapon
	UPROPERTY()
	int32 WeaponArrayIndex = 0;
	
	// The highest single damage this entity can output.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapons")
	EDamageType MaxDamageType = EDamageType::EDT_None;

	// The longest range our most powerful weapon can reach.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapons")
	float MaxDamageRange = 0.f;
	
	// The highest single damage this entity can output.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapons")
	float MaxDamage = 0.f;
	
	// Mount of the most damaging weapon available.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapons")
	FGuid MaxDamageWeaponID;

	//------------------------------------------
	// The Mamximum Range of all our Weapons
	//------------------------------------------
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapons")
	float MaxRange = 0.f;
	
//------------------------------------------------------------------------------
// Functionality
//------------------------------------------------------------------------------
protected:

	// Ensure that we have a statusComponent active. This component will not function properly without it.
	void EnsureStatusComponent();

	// This works with both Turrets and Fixed Weapons
	template <typename WeaponComponentType>
	int8 EvaluateTargetsInRange(TObjectPtr<WeaponComponentType>& WeaponMount, const TArray<FOverlapResult>& TargetsInRangeArray);
	
	// Searches the proximity for Viable Targets based on our furthest range.
	void SearchForTargets(float SearchRange);
	
};