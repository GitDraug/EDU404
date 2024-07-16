// Fill out your copyright notice in the Description page of Project Settings.


#include "Entities/EDU_CORE_TickingEntity.h"
#include "Framework/Data/FLOWLOG/FLOWLOG_ENTITIES.h"
#include "Framework/Managers/GameModes/EDU_CORE_GameMode.h"


//------------------------------------------------------------------------------
// Construction & Object Lifetime Management
//------------------------------------------------------------------------------
// Called when the game starts or when spawned
void AEDU_CORE_TickingEntity::BeginPlay()
{ FLOW_LOG
	Super::BeginPlay();

	if (AEDU_CORE_GameMode* GameMode = Cast<AEDU_CORE_GameMode>(GetWorld()->GetAuthGameMode()))
	{
		GameMode->AddToTickingEntityArray(this);
	}
}

// Called every frame by GameMode
void AEDU_CORE_TickingEntity::ParallelTick()
{ FLOW_LOG_TICK
	
}

