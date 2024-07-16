// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EDU_CORE_SelectableEntity.h"
#include "EDU_CORE_TickingEntity.generated.h"

UCLASS()
class EDU_CORE_API AEDU_CORE_TickingEntity : public AEDU_CORE_SelectableEntity
{
	GENERATED_BODY()

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	virtual void ParallelTick();
};
