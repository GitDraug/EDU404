﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Entities/EDU_CORE_MobileEntity.h"
#include "Entities/EDU_CORE_PhysicsEntity.h"
#include "GameFramework/Pawn.h"
#include "Interfaces/EDU_CORE_Selectable.h"
#include "EDU_CORE_TestPawn.generated.h"

/*------------------------------------------------------------------------------
  This pawn only exists to test out new functionality.
------------------------------------------------------------------------------*/

UCLASS()
class EDU_CORE_API AEDU_CORE_TestPawn : public AEDU_CORE_MobileEntity
{
	GENERATED_BODY()
	
//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------
public:
	// Sets default values for this pawn's properties
	AEDU_CORE_TestPawn(const FObjectInitializer& ObjectInitializer);
	
//------------------------------------------------------------------------------
// Object Lifetime Management
//------------------------------------------------------------------------------
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	FVector MyPos;

//------------------------------------------------------------------------------
// Components
//------------------------------------------------------------------------------
	
	FCriticalSection CriticalSection; // Declare a critical section
	int32 SharedResource = 0; // Shared resource
	
//------------------------------------------------------------------------------
// Interfacing
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Functionality
//------------------------------------------------------------------------------
};
