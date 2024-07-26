// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "EDU_CORE_GameMode.generated.h"

class AEDU_CORE_MobileEntity;
class AEDU_CORE_AbstractEntity;
class AEDU_CORE_PhysicalEntity;
class URTS_CORE_GameDataAsset;

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
	// Aggregated Tick Arrays
	void AddToAbstractEntityArray(AEDU_CORE_AbstractEntity* AbstractEntity);
	void AddToPhysicalEntityArray(AEDU_CORE_PhysicalEntity* PhysicalEntity);
	void AddToMobileEntityArray(AEDU_CORE_MobileEntity* MobileEntity);
	
	FColor DeltaTimeDisplayColor;
//------------------------------------------------------------------------------
// Components
//------------------------------------------------------------------------------
protected:
	// Give the server access to all classes that have aggregated tick.
	
	/*---------------------- Server-Side Aggregated Tick ---------------------------
	  This tick function allows us to aggregate ticks server-side. These are
	  excellent for batch executions, such as blending occasional Server updates.

	  EntityBatch: Represents the number of entities to process in each tick.
	  The loop will run EntityBatch times, allowing for controlled batch processing
	  of entities.
	  
	  Index tracks which element in the EntityArray is being processed.
	------------------------------------------------------------------------------*/
	UPROPERTY()
	TArray<AEDU_CORE_AbstractEntity*> AbstractEntityArray;

	UPROPERTY(EditDefaultsOnly)
	int32 AbstractEntityBatch = 100;
	int32 AbstractEntityIndex = 0;
	
	UPROPERTY()
	TArray<AEDU_CORE_PhysicalEntity*> PhysicalEntityArray;

	UPROPERTY(EditDefaultsOnly)
	int32 PhysicalEntityBatch = 10;
	int32 PhysicalEntityIndex = 0;

	UPROPERTY()
	TArray<AEDU_CORE_MobileEntity*> MobileEntityArray;

	UPROPERTY(EditDefaultsOnly)
	int32 MobileEntityBatch = 50;
	int32 MobileEntityIndex = 0;

	int32 FrameCounter;
//------------------------------------------------------------------------------
// Functionality
//------------------------------------------------------------------------------

};