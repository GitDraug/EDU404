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

//  Struct to store target-related information, used to evaluate target priority.
USTRUCT(BlueprintType)
struct FTargetInformation
{
	GENERATED_BODY()

	// Pointer To TargetEntity
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Target Information")
	TObjectPtr<AEDU_CORE_SelectableEntity> TargetEntity;

	// The health of the target
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Target Information")
	float Health;

	// The Armor value of the target
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Target Information")
	float Armor;

	// The distance from the actor to the target
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Target Information")
	float Distance;

	// The danger level of the target, where higher values indicate more dangerous targets
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Target Information")
	float DangerLevel;
	
};

UENUM()
enum class ESimultainiusTurrets : uint8
{
	None			UMETA(DisplayName = "None"),
	SingleTurret	UMETA(DisplayName = "Single Turret"),
	MultiTurret		UMETA(DisplayName = "Multi Turret"),
	Max				UMETA(Hidden)
};

UENUM()
enum class ETargetPriority : uint8
{
	Nearest				UMETA(DisplayName = "Nearest"),
	Farthest			UMETA(DisplayName = "Farthest"),

	LowestHealth		UMETA(DisplayName = "Lowest Health"),
	HighestHealth		UMETA(DisplayName = "Highest Health"),
	
	LowestDamage		UMETA(DisplayName = "Lowest Damage"),
	HighestDamage		UMETA(DisplayName = "Highest Damage"),
	
	Max					UMETA(Hidden)
};
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EDU_CORE_API UEngagementComponent : public UActorComponent
{
	GENERATED_BODY()

//------------------------------------------------------------------------------
// Get/Set
//------------------------------------------------------------------------------
public:
	// Checks if the Entity has any fixed weapons that require the entity to align with the target.
	FORCEINLINE bool HasFixedWeapon() const { return bHasFixedWeapons; };
	
	// Add a turret to TurretArray
	void AddToTurretArray(UTurretWeaponComponent* TurretComponent);

	// Add a Fixed Weapons to TurretArray
	void AddToFixedWeaponsArray(UFixedWeaponComponent* FixedWeapon);

	//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------
public:
	// Sets default values for this component's properties
	UEngagementComponent();
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
	bool bDrawSightDebugShape = false;
		
	// What object tyoe are we searching for?
	UPROPERTY(EditAnywhere)
	TEnumAsByte<ECollisionChannel> TargetObjectType = ECC_Pawn ;

	// Do we have Fixed weapons that need the Entity to align with target?
	UPROPERTY(EditAnywhere)
	bool bHasFixedWeapons = false;
	
//------------------------------------------------------------------------------
// Components
//------------------------------------------------------------------------------
protected:
	
	// Array of available Turrets
	UPROPERTY(VisibleAnywhere, Category = "Engagement | Weapons")
	TArray<TObjectPtr<UTurretWeaponComponent>> TurretArray;

	// Array of Fixed Weapons Turrets
	UPROPERTY(VisibleAnywhere, Category = "Engagement | Weapons")
	TArray<TObjectPtr<UFixedWeaponComponent>> FixedWeaponsArray;
	
	// Array of possible Targets
	UPROPERTY(VisibleAnywhere, Category = "Engagement | Targets")
	TArray<FOverlapResult> TargetArray;
	
	// Pointer to GameMode for easy access to Team Arrays
	UPROPERTY()
	TObjectPtr<AEDU_CORE_GameMode> GameMode = nullptr;

	// Pointer to StatusComponent
	UPROPERTY()
	TObjectPtr<UStatusComponent> StatusComponent;

	// How many weapon systems are we managing?
	UPROPERTY()
	ESimultainiusTurrets SimultainiusTurrets = ESimultainiusTurrets::None;

	UPROPERTY()
	ETargetPriority TargetPriority = ETargetPriority::Nearest;

	/*----------------------Max Weapon Damage Info -----------------------
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
	// -666 = Error
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapons")
	float MaxDamage = -666.f;
	
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

	// Searches the proximity for Viable Targets based on our furthest range.
	void SearchForTargets();

	TObjectPtr<AEDU_CORE_SelectableEntity> GetPriorityTarget(EDamageType DamageType, float Damage, float Range, ETargetPriority Priority = ETargetPriority::Nearest);
	
};
