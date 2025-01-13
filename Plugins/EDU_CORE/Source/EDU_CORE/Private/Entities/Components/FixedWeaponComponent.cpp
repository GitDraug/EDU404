// Fill out your copyright notice in the Description page of Project Settings.

// THIS
#include "Entities/Components/FixedWeaponComponent.h"

// CORE
#include "Entities/EDU_CORE_MobileEntity.h"
#include "Framework/Data/FLOWLOGS/FLOWLOG_COMPONENTS.h"
#include "Entities/Components/EngagementComponent.h"
#include "Framework/Managers/GameModes/EDU_CORE_GameMode.h"

//------------------------------------------------------------------------------
// Get/Set
//------------------------------------------------------------------------------

// Sets default values for this component's properties
UFixedWeaponComponent::UFixedWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
}

void UFixedWeaponComponent::OnRegister()
{
	Super::OnRegister();

	// Clients need WeaponInfo for UI.
	SetIsReplicated(true);
	
	// Find whatever component this component is attached to and save it.
	MobileEntity = Cast<AEDU_CORE_MobileEntity>(GetOwner());
	if(MobileEntity)
	{
		MobileEntity->SetAlignWithTarget(true);
		FLOW_LOG_WARNING("FixedWeaponComponent is Active!")
	}
	else
	{
		FLOW_LOG_ERROR("Owner is not a Mobile Entity! This will result in unwanted behaviour!")
	}
	
}

void UFixedWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	// Server Tick
	if(GetNetMode() != NM_Client)
	{
		if (AEDU_CORE_GameMode* GameMode = Cast<AEDU_CORE_GameMode>(GetWorld()->GetAuthGameMode()))
		{
			GameMode->AddToFixedWeaponComponentArray(this);
		}
	}
}

//------------------------------------------------------------------------------
// Aggregated Server tick
//------------------------------------------------------------------------------

void UFixedWeaponComponent::ServerFixedWeaponCalc(float AsyncDeltaTime)
{
	// DummyTarget
	// TODO: Remember to remove this.
	// TargetPosition = FVector(0.f, 0.f, 400.f);
	if(TargetPosition != FVector::ZeroVector)
	{
		MobileEntity->SetTargetPosition(TargetPosition);
	}	
}

void UFixedWeaponComponent::ServerFixedWeaponExec(float AsyncDeltaTime)
{
}

void UFixedWeaponComponent::ServerTimeGatedFixedWeaponExec(float AsyncDeltaTime)
{
}

//------------------------------------------------------------------------------
// Functionality > Setup
//------------------------------------------------------------------------------

void UFixedWeaponComponent::EnsureEngagementComponent()
{
	if(GetOwner())
	{
		EngagementComponent = GetOwner()->FindComponentByClass<UEngagementComponent>();
		
		if(EngagementComponent)
		{
			EngagementComponent->AddToFixedWeaponsArray(this);
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
