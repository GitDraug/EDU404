// Fill out your copyright notice in the Description page of Project Settings.


#include "Entities/Components/EDU_CORE_TurretComponent.h"


//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------
UEDU_CORE_TurretComponent::UEDU_CORE_TurretComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
}

void UEDU_CORE_TurretComponent::BeginPlay()
{
	Super::BeginPlay();

	// Find components in the owner actor
	TowerMesh = GetOwner()->FindComponentByClass<UStaticMeshComponent>();
	BarrelMesh = GetOwner()->FindComponentByClass<UStaticMeshComponent>();
	
}

//------------------------------------------------------------------------------
// Aggregated Server tick
//------------------------------------------------------------------------------
void UEDU_CORE_TurretComponent::ServerTurretCalc(float DeltaTime)
{
}

void UEDU_CORE_TurretComponent::ServerTurretExec(float DeltaTime)
{
}

//------------------------------------------------------------------------------
// Functionality
//------------------------------------------------------------------------------

void UEDU_CORE_TurretComponent::RotateTurret(float DeltaTime, float TargetYaw) const
{
	if(!TowerMesh) return;

	// Calculate rotation step
	const FRotator CurrentRotation = TowerMesh->GetRelativeRotation();
	const float YawStep = RotationSpeed * DeltaTime;
	const float NewYaw = FMath::FInterpTo(CurrentRotation.Yaw, TargetYaw, DeltaTime, RotationSpeed);

	// Update turret rotation
	TowerMesh->SetRelativeRotation(FRotator(0, NewYaw, 0));
}

void UEDU_CORE_TurretComponent::ElevateTurret(float DeltaTime, float TargetPitch) const
{
	if(!BarrelMesh) return;

	// Clamp pitch within range
	TargetPitch = FMath::Clamp(TargetPitch, MinElevation, MaxElevation);

	// Calculate elevation step
	const FRotator CurrentRotation = BarrelMesh->GetRelativeRotation();
	const float NewPitch = FMath::FInterpTo(CurrentRotation.Pitch, TargetPitch, DeltaTime, RotationSpeed);

	// Update barrel elevation
	BarrelMesh->SetRelativeRotation(FRotator(NewPitch, 0, 0));
}


