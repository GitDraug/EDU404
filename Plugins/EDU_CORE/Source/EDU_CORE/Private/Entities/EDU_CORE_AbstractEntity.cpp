// Fill out your copyright notice in the Description page of Project Settings.


#include "Entities/EDU_CORE_AbstractEntity.h"
#include "Framework/Data/FLOWLOGS/FLOWLOG_ENTITIES.h"
#include "Framework/Managers/GameModes/EDU_CORE_GameMode.h"
#include "Framework/Player/EDU_CORE_PlayerController.h"

//------------------------------------------------------------------------------
// Construction & Object Lifetime Management
//------------------------------------------------------------------------------
void AEDU_CORE_AbstractEntity::BeginPlay()
{ FLOW_LOG
	Super::BeginPlay();
	
	// Add entity to ServerTick
	if(bServerTickEnabled && GetNetMode() != NM_Client)
	{
		if (AEDU_CORE_GameMode* GameMode = Cast<AEDU_CORE_GameMode>(GetWorld()->GetAuthGameMode()))
		{
			GameMode->AddToAbstractEntityArray(this);
		}
	}

	// Add entity to ClientTick
	if(bClientTickEnabled && GetNetMode() == NM_Client)
	{
		if (AEDU_CORE_PlayerController* LocalController = Cast<AEDU_CORE_PlayerController>(GetWorld()->GetFirstPlayerController()))
		{
			LocalController->AddToAbstractEntityArray(this);
		}
	}
}

//------------------------------------------------------------------------------
// Construction & Object Lifetime Management
//------------------------------------------------------------------------------

void AEDU_CORE_AbstractEntity::ServerTick(float DeltaTime)
{ // FLOW_LOG_TICK
	
}

void AEDU_CORE_AbstractEntity::ClientTick(float DeltaTime)
{ // FLOW_LOG_TICK
	
}

void AEDU_CORE_AbstractEntity::Destroyed()
{ FLOW_LOG
	Super::Destroyed();
}
