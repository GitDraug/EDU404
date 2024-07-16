// Fill out your copyright notice in the Description page of Project Settings.


#include "Entities/Units/EDU_CORE_TestPawn.h"
#include "Framework/Data/FLOWLOG/FLOWLOG_ENTITIES.h"
#include "Framework/Managers/GameModes/EDU_CORE_GameMode.h"

//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------
AEDU_CORE_TestPawn::AEDU_CORE_TestPawn()
{ FLOW_LOG
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

//------------------------------------------------------------------------------
// Object Lifetime Management
//------------------------------------------------------------------------------

void AEDU_CORE_TestPawn::Tick(float DeltaTime)
{ // FLOW_LOG_TICK
	Super::Tick(DeltaTime);


}

void AEDU_CORE_TestPawn::ParallelTick()
{
	Super::ParallelTick();
	
	// FScopeLock Lock(&CriticalSection); // Lock the critical section
	
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Yellow, FString::Printf(TEXT("bMouseHighlighted.X %hhd"), bMouseHighlighted));
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Yellow, FString::Printf(TEXT("bRectangleHighlighted.X %hhd"), bRectangleHighlighted));
	GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Yellow, FString::Printf(TEXT("bSelected.X %hhd"), bSelected));
}
//------------------------------------------------------------------------------
// Interfacing
//------------------------------------------------------------------------------



