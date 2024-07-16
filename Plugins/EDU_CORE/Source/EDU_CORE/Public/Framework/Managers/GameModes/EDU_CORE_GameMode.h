// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "EDU_CORE_GameMode.generated.h"

class URTS_CORE_GameDataAsset;
class AEDU_CORE_TickingEntity;
/*------------------------------------------------------------------------------
  Abstract SUPER Class intended to be inherited from.
--------------------------------------------------------------------------------
  The GameModeBase defines the game being played. It governs the game rules,
  scoring, what actors are allowed to exist in this game type, and who may
  enter the game.

  It is only instanced on the server and will never exist on the client.

  A GameModeBase actor is instantiated when the level is initialized for gameplay in
  C++ UGameEngine::LoadMap().  
  
  The class of this GameMode actor is determined by (in order) either the
  URL ?game=xxx, the GameMode Override value set in the World Settings, or the
  DefaultGameMode entry set in the game's Project Settings.
------------------------------------------------------------------------------*/
UCLASS(Abstract)
class EDU_CORE_API AEDU_CORE_GameMode : public AGameModeBase
{
	GENERATED_BODY()

//------------------------------------------------------------------------------
// Initialization & Object lifetime management
//------------------------------------------------------------------------------	
public:
	AEDU_CORE_GameMode(const FObjectInitializer& ObjectInitializer);

	virtual void Tick(float DeltaTime) override;
	
	
//------------------------------------------------------------------------------
// Public API
//------------------------------------------------------------------------------
public:
	void AddToTickingEntityArray(AEDU_CORE_TickingEntity* TickingEntity);
	
//------------------------------------------------------------------------------
// Components
//------------------------------------------------------------------------------
	UPROPERTY()
	TArray<AEDU_CORE_TickingEntity*> TickingEntityArray;
	
	// Index that tracks which entity in TickingEntityArray is being processed.
	int32 CurrentEntity = 0;

	// EntityBatch: Represents the number of entities to process in each tick.
	// The loop will run EntityBatch times, allowing for controlled batch
	// processing of entities.
	int32 EntityBatch = 100;
	
	bool Iticked = false;

//------------------------------------------------------------------------------
// Functionality
//------------------------------------------------------------------------------
	// void ParallelTick(AEDU_CORE_TickingEntity* Entity) const;

};