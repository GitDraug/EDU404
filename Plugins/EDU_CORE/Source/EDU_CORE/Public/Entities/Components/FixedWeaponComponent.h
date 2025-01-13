// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// CORE
#include "Framework/Data/DataTypes/EDU_CORE_DataTypes.h"

// UE
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

// THIS
#include "FixedWeaponComponent.generated.h"

/*------------------------------------------------------------------------------
  Fixed Weapon Component
--------------------------------------------------------------------------------
  Acts as an abstract barrel wich logic for a weapon to fire from.

  This component is intended to be used with a MobileEntity, which acts as a
  mount for YawMovement, while this component can still pitch. The component
  is pure data and logic, no graphics are involved.
------------------------------------------------------------------------------*/

class UEngagementComponent;
class AEDU_CORE_MobileEntity;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EDU_CORE_API UFixedWeaponComponent : public UActorComponent
{
	GENERATED_BODY()
	
//------------------------------------------------------------------------------
// Get/Set
//------------------------------------------------------------------------------
public:

//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------
public:
	// Sets default values for this component's properties
	UFixedWeaponComponent();

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
