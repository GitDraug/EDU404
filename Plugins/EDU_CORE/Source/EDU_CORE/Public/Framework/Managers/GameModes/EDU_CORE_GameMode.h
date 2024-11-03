// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Framework/Data/DataTypes/EDU_CORE_DataTypes.h"

#include "CoreMinimal.h"

#include "GameFramework/GameModeBase.h"
#include "EDU_CORE_GameMode.generated.h"

class IEDU_CORE_CommandInterface;
class AEDU_CORE_MobileEntity;
class AEDU_CORE_AbstractEntity;
class AEDU_CORE_PhysicsEntity;
class AEDU_CORE_Waypoint;
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
  
  This CORE_GameMode makes use of an aggregated tick, and is meant to be used
  with all entities; actors with common tick functions. Use it for all levels
  where the player is expected to interact with entities.
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

	virtual void BeginPlay() override;
	
//------------------------------------------------------------------------------
// Public API
//------------------------------------------------------------------------------
public:
	// Aggregated Tick Arrays
	void AddToAbstractEntityArray(AEDU_CORE_AbstractEntity* AbstractEntity);
	void AddToPhysicsEntityArray(AEDU_CORE_PhysicsEntity* PhysicsEntity);
	void AddToMobileEntityArray(AEDU_CORE_MobileEntity* MobileEntity);

	// Used when checking if a Unit Order came from the right Team.
	void AddActorToTeamArray(AActor* Actor, EEDU_CORE_Team TeamArray = EEDU_CORE_Team::None);
	void RemoveActorFromTeamArray(AActor* Actor, EEDU_CORE_Team TeamArray = EEDU_CORE_Team::None);

	// For other Actors to see what actors are on their team.
	TArray<TObjectPtr<AActor>> GetTeamArray(EEDU_CORE_Team TeamArray = EEDU_CORE_Team::None) const;

	// For Entities and Actors to register across instances.
	void AddToGuidActorMap(FGuid GUID, AActor* Actor);
	TObjectPtr<AActor> FindActorInMap(FGuid GUID) const;

	// Waypoint Management
	TObjectPtr<AEDU_CORE_Waypoint> GetFreshWaypointFromPool(EEDU_CORE_Team Team = EEDU_CORE_Team::None, const FVector& WorldLocation = FVector::ZeroVector, const FRotator& WorldRotation = FRotator::ZeroRotator);
	
	void ReturnWaypointToPool(const TObjectPtr<AEDU_CORE_Waypoint>& Waypoint);
	
	FColor DeltaTimeDisplayColor;
	
//------------------------------------------------------------------------------
// Components
//------------------------------------------------------------------------------
//protected:
	/*--------------------------- AI Waypoint pool ---------------------------------
  
	------------------------------------------------------------------------------*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoints")
	TSubclassOf<AEDU_CORE_Waypoint> WaypointClass;
	
	TArray<TObjectPtr<AEDU_CORE_Waypoint>> AvailableWaypointPool;
	TArray<TObjectPtr<AEDU_CORE_Waypoint>> BusyWaypointPool;	
	
	/*------------------------------- Teams ----------------------------------------
  
	------------------------------------------------------------------------------*/
	
	// Enum to easily select Team.
	EEDU_CORE_Team Team;

	// Team Arrays
	TArray<TObjectPtr<AActor>> Team_0_Array;
	TArray<TObjectPtr<AActor>> Team_1_Array;
	TArray<TObjectPtr<AActor>> Team_2_Array;
	TArray<TObjectPtr<AActor>> Team_3_Array;
	TArray<TObjectPtr<AActor>> Team_4_Array;
	TArray<TObjectPtr<AActor>> Team_5_Array;
	TArray<TObjectPtr<AActor>> Team_6_Array;
	TArray<TObjectPtr<AActor>> Team_7_Array;
	TArray<TObjectPtr<AActor>> Team_8_Array;
	TArray<TObjectPtr<AActor>> Team_9_Array;
	TArray<TObjectPtr<AActor>> Team_10_Array;
	
	/*---------------------- Server ID for MP communication  -----------------------
	  Pointers are local, so we can't use them to send information to the server.
	  In order for a client to manipulate an instance on the server machine, we
	  need an ID that is unique across instances. We use a GUID for this, and a
	  HashMap on the server to tie each entity to the ID. The entity requests
	  a unique ID from the server upon BeginPlay.

	  HashMaps are really fast, but can't be replicated, so they are perfect
	  for this purpose.
	------------------------------------------------------------------------------*/
	UPROPERTY(Transient)
	TMap<FGuid, AActor*> GuidActorMap;
	
	/*---------------------- Server-Side Aggregated Tick ---------------------------
	  This tick function allows us to aggregate ticks server-side. These are
	  excellent for batch executions, such as blending occasional Server updates.

	  BatchIndex allows us to pass tick groups to our array members, so only
	  members of the CurrentBatchIndex will process logic.
	------------------------------------------------------------------------------*/
	UPROPERTY()
	TArray<AEDU_CORE_AbstractEntity*> AbstractEntityArray;
	
	UPROPERTY()
	TArray<AEDU_CORE_PhysicsEntity*> PhysicsEntityArray;

	UPROPERTY()
	TArray<AEDU_CORE_MobileEntity*> MobileEntityArray;

	// Used for time slicing, passing the BatchIndex being processed.
	int32 BatchIndex;
	int32 CurrentBatchIndex = 0;
	
	IEDU_CORE_CommandInterface* CommandInterface;
	
//------------------------------------------------------------------------------
// Functionality
//------------------------------------------------------------------------------

};