// Fill out your copyright notice in the Description page of Project Settings.


#include "Entities/Units/EDU_CORE_TestPawn.h"
#include "Framework/Data/FLOWLOGS/FLOWLOG_ENTITIES.h"

//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------
AEDU_CORE_TestPawn::AEDU_CORE_TestPawn(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{ FLOW_LOG
	  PrimaryActorTick.bCanEverTick = false;
}

//------------------------------------------------------------------------------
// Object Lifetime Management
//------------------------------------------------------------------------------

void AEDU_CORE_TestPawn::Tick(float DeltaTime)
{// FLOW_LOG_TICK
	
	Super::Tick(DeltaTime);
}