// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EDU_CORE_SelectableEntity.h"
#include "EDU_CORE_MobileEntity.generated.h"

UCLASS()
class EDU_CORE_API AMobileEntity : public AEDU_CORE_SelectableEntity
{
	GENERATED_BODY()

//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------
public:
	AMobileEntity();

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
// Functionality
//------------------------------------------------------------------------------
};
