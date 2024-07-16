// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Interfaces/EDU_UNIT_Selectable.h"
#include "TestPawn.generated.h"

/*------------------------------------------------------------------------------
  This pawn only exists to test out new functionality.
------------------------------------------------------------------------------*/

UCLASS()
class EDU_UNIT_API ATestPawn : public APawn, public IEDU_UNIT_Selectable
{
	GENERATED_BODY()
	
//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------
public:
	// Sets default values for this pawn's properties
	ATestPawn();
	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
//------------------------------------------------------------------------------
// Object Lifetime Management
//------------------------------------------------------------------------------
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

//------------------------------------------------------------------------------
// Components
//------------------------------------------------------------------------------
	
//------------------------------------------------------------------------------
// Interfacing
//------------------------------------------------------------------------------
public:
	virtual void HighlightActor() override;
	virtual void UnHighlightActor() override;

	bool bHighlighted = false;
	
//------------------------------------------------------------------------------
// Functionality
//------------------------------------------------------------------------------
};
