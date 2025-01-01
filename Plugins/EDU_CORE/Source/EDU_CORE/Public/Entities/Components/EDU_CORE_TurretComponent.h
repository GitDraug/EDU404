// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EDU_CORE_TurretComponent.generated.h"


/*------------------------------------------------------------------------------
  Turret component
--------------------------------------------------------------------------------
  Acts as a Gimbal to attach weapons and senors. It's not a physics actor
  so its rotation should be rather lightweight.
------------------------------------------------------------------------------*/

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EDU_CORE_API UEDU_CORE_TurretComponent : public UActorComponent
{
	GENERATED_BODY()
	
//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------
public:
	// Sets default values for this component's properties
	UEDU_CORE_TurretComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

//------------------------------------------------------------------------------
// Aggregated Server tick
//------------------------------------------------------------------------------
public:
	virtual void ServerTurretCalc(float DeltaTime);
	virtual void ServerTurretExec(float DeltaTime);

//------------------------------------------------------------------------------
// Editable Data: General
//------------------------------------------------------------------------------

	// Turret rotation speed in degrees per second
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
	float RotationSpeed = 30.f;

	// Maximum elevation angle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
	float MaxElevation = 80.f;

	// Minimum elevation angle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
	float MinElevation = -15.f;
	
//------------------------------------------------------------------------------
// Components
//------------------------------------------------------------------------------
protected:
	
	// Pointer to owning Actor
	UPROPERTY()
	AActor* Owner = GetOwner();

	// Cached references to the turret and barrel components
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> TowerMesh;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> BarrelMesh;

//------------------------------------------------------------------------------
// Functionality
//------------------------------------------------------------------------------
protected:	

	// Rotates the turret horizontally
	void RotateTurret(float DeltaTime, float TargetYaw) const;

	// Elevates the turret vertically
	void ElevateTurret(float DeltaTime, float TargetPitch) const;

};
