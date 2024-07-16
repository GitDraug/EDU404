// Fill out your copyright notice in the Description page of Project Settings.


#include "TestPawns/TestPawn.h"
#include "Framework/Data/FlowLog.h"


//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------
ATestPawn::ATestPawn()
{ FLOW_LOG
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void ATestPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{ FLOW_LOG
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

//------------------------------------------------------------------------------
// Object Lifetime Management
//------------------------------------------------------------------------------

void ATestPawn::BeginPlay()
{ FLOW_LOG
	Super::BeginPlay();
	
}

void ATestPawn::Tick(float DeltaTime)
{ FLOW_LOG_TICK
	Super::Tick(DeltaTime);
}

//------------------------------------------------------------------------------
// Interfacing
//------------------------------------------------------------------------------
void ATestPawn::HighlightActor()
{ FLOW_LOG
	bHighlighted = true;
}

void ATestPawn::UnHighlightActor()
{ FLOW_LOG
	bHighlighted = false;
}

