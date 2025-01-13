// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Framework/Data/DataTypes/EDU_CORE_DataTypes.h"

#include "CoreMinimal.h"

#include "GameFramework/GameModeBase.h"
#include "EDU_CORE_GameMode.generated.h"

class UFixedWeaponComponent;
class UTurretWeaponComponent;
class AEDU_CORE_MobileEntity;
class AEDU_CORE_AbstractEntity;
class AEDU_CORE_PhysicsEntity;

class AEDU_CORE_Waypoint;
class URTS_CORE_GameDataAsset;
class IEDU_CORE_CommandInterface;

class USenseComponent;
class UStatusComponent;
class UEngagementComponent;

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

// Struct to hold the visibility arrays for each team
USTRUCT(BlueprintType)
struct FVisibilityStruct
{
	GENERATED_BODY()

	//------------------------------------------------------------------------------
	// Visibility arrays, used to skip actors during FOV checks.
	//------------------------------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visible Actors")
	TArray<AActor*> Team_0_VisibleActors;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visible Actors")
	TArray<AActor*> Team_1_VisibleActors;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visible Actors")
	TArray<AActor*> Team_2_VisibleActors;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visible Actors")
	TArray<AActor*> Team_3_VisibleActors;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visible Actors")
	TArray<AActor*> Team_4_VisibleActors;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visible Actors")
	TArray<AActor*> Team_5_VisibleActors;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visible Actors")
	TArray<AActor*> Team_6_VisibleActors;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visible Actors")
	TArray<AActor*> Team_7_VisibleActors;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visible Actors")
	TArray<AActor*> Team_8_VisibleActors;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visible Actors")
	TArray<AActor*> Team_9_VisibleActors;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visible Actors")
	TArray<AActor*> Team_10_VisibleActors;

	//------------------------------------------------------------------------------
	// Hidden actor arrays, used to skip actors during Weapon Checks.
	//------------------------------------------------------------------------------
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hidden Actors")
	TArray<AActor*> Team_0_HiddenActors;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hidden Actors")
	TArray<AActor*> Team_1_HiddenActors;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hidden Actors")
	TArray<AActor*> Team_2_HiddenActors;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hidden Actors")
	TArray<AActor*> Team_3_HiddenActors;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hidden Actors")
	TArray<AActor*> Team_4_HiddenActors;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hidden Actors")
	TArray<AActor*> Team_5_HiddenActors;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hidden Actors")
	TArray<AActor*> Team_6_HiddenActors;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hidden Actors")
	TArray<AActor*> Team_7_HiddenActors;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hidden Actors")
	TArray<AActor*> Team_8_HiddenActors;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hidden Actors")
	TArray<AActor*> Team_9_HiddenActors;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hidden Actors")
	TArray<AActor*> Team_10_HiddenActors;

};

UCLASS(Abstract)
class EDU_CORE_API AEDU_CORE_GameMode : public AGameModeBase
{
	GENERATED_BODY()

//------------------------------------------------------------------------------
// Initialization & Object lifetime management
//------------------------------------------------------------------------------	
public:
	AEDU_CORE_GameMode(const FObjectInitializer& ObjectInitializer);

	// Reserve space for arrays
	virtual void InitiateArrays();

	virtual void Tick(float DeltaTime) override;

	virtual void BeginPlay() override;
	
//------------------------------------------------------------------------------
// Get/Set
//------------------------------------------------------------------------------
public:
	// Aggregated Tick Arrays
	void AddToAbstractEntityArray(AEDU_CORE_AbstractEntity* AbstractEntity);
	void AddToPhysicsEntityArray(AEDU_CORE_PhysicsEntity* PhysicsEntity);
	void AddToMobileEntityArray(AEDU_CORE_MobileEntity* MobileEntity);
	void AddToSightComponentArray(USenseComponent* SightComponent);
	void AddToStatusComponentArray(UStatusComponent* StatusComponent);
	void AddToTurretComponentArray(UTurretWeaponComponent* TurretComponent);
	void AddToFixedWeaponComponentArray(UFixedWeaponComponent* FixedWeaponComponent);
	void AddToEngagementComponentArray(UEngagementComponent* EngagementComponent);

	// Used when checking if a Unit Order came from the right Team.
	void AddActorToTeamArray(AActor* Actor, EEDU_CORE_Team TeamArray = EEDU_CORE_Team::None);
	void RemoveActorFromTeamArray(AActor* Actor, EEDU_CORE_Team TeamArray = EEDU_CORE_Team::None);

	// Used to keep track of entities that are visible to other teams, mainly to skip them in visual checks.
	void AddActorToTeamVisibleActorsArray(AActor* Actor, EEDU_CORE_Team TeamArray = EEDU_CORE_Team::None);
	void RemoveActorFromTeamVisibleActorsArray(AActor* Actor, EEDU_CORE_Team TeamArray = EEDU_CORE_Team::None);
	
	// For other Actors to see what friendly actors are on their team.
	TArray<AActor*>& GetTeamArray(EEDU_CORE_Team TeamArray = EEDU_CORE_Team::None);

	// For Actors to see what enemy actors are visible to their team.
	TArray<AActor*>& GetTeamVisibleActorsArray(EEDU_CORE_Team TeamArray = EEDU_CORE_Team::None);

	// For Weapons to see what enemy actors are Hidden to their team (to ignore them).
	TArray<AActor*>& GetTeamHiddenActorsArray(EEDU_CORE_Team TeamArray = EEDU_CORE_Team::None);
	
	// For Entities and Actors to register across instances.
	void AddToGuidActorMap(FGuid GUID, AActor* Actor);

	// Exchanges a GUID that works across instances for a pointer of the same Entity on the server
	TObjectPtr<AActor> FindActorInMap(FGuid GUID) const;

	// Waypoint Management
	TObjectPtr<AEDU_CORE_Waypoint> GetFreshWaypointFromPool(EEDU_CORE_Team Team = EEDU_CORE_Team::None, const FVector& WorldLocation = FVector::ZeroVector, const FRotator& WorldRotation = FRotator::ZeroRotator);
	
	void ReturnWaypointToPool(const TObjectPtr<AEDU_CORE_Waypoint>& Waypoint);
	
//------------------------------------------------------------------------------
// Components
//------------------------------------------------------------------------------
protected:
	
	/*--------------------------- AI Waypoint pool ---------------------------------
  
	------------------------------------------------------------------------------*/
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoints")
	TSubclassOf<AEDU_CORE_Waypoint> WaypointClass;

	UPROPERTY()
	TArray<TObjectPtr<AEDU_CORE_Waypoint>> AvailableWaypointPool;

	UPROPERTY()
	TArray<TObjectPtr<AEDU_CORE_Waypoint>> BusyWaypointPool;	
	
	/*------------------------------- Teams ----------------------------------------
  
	------------------------------------------------------------------------------*/
	
	// Enum to easily select Team.
	EEDU_CORE_Team Team;

	//------------------------------------------------------------------------------
	// Team Arrays
	//	<!> FCollisionQueryParams AddIgnoredActors takes a const TArray<AActor*>&
	//------------------------------------------------------------------------------
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	TArray<AActor*> Team_0_Array;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	TArray<AActor*> Team_1_Array;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	TArray<AActor*> Team_2_Array;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	TArray<AActor*> Team_3_Array;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	TArray<AActor*> Team_4_Array;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	TArray<AActor*> Team_5_Array;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	TArray<AActor*> Team_6_Array;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	TArray<AActor*> Team_7_Array;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	TArray<AActor*> Team_8_Array;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	TArray<AActor*> Team_9_Array;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	TArray<AActor*> Team_10_Array;

	//------------------------------------------------------------------------------
	// Visibility arrays, used to skip actors during FOV checks.
	//------------------------------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visibility")
	FVisibilityStruct VisibilityStruct;
	
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
	------------------------------------------------------------------------------*/
	
	UPROPERTY()
	TArray<TObjectPtr<AEDU_CORE_AbstractEntity>> AbstractEntityArray;
	
	UPROPERTY()
	TArray<TObjectPtr<AEDU_CORE_PhysicsEntity>> PhysicsEntityArray;

	UPROPERTY()
	TArray<TObjectPtr<AEDU_CORE_MobileEntity>> MobileEntityArray;

	UPROPERTY()
	TArray<TObjectPtr<USenseComponent>> SightComponentArray;
	
	UPROPERTY()
	TArray<TObjectPtr<UStatusComponent>> StatusComponentArray;

	UPROPERTY()
	TArray<TObjectPtr<UTurretWeaponComponent>> TurretComponentArray;

	UPROPERTY()
	TArray<TObjectPtr<UFixedWeaponComponent>> FixedWeaponComponentArray;
	
	UPROPERTY()
	TArray<TObjectPtr<UEngagementComponent>> EngagementComponentArray;

	/*------------------------------ BatchIndex ------------------------------------
	  BatchIndex allows us to pass tick groups to our array members, so only
	  members of the CurrentBatchIndex will process logic. BatchIndex_10 will run
	  10 indexes over cycle, while BatchIndex_100 will run 100 batches over cycle.

	  In other words, if 100 entities are active, it will take 10 or 100 frames
	  respectivly before the Index return.
	------------------------------------------------------------------------------*/
	
	// This Batch divides the slice into 10.
	
	int32 BatchIndex_10;
	int32 CurrentBatchIndex_10 = 0;

	// This Batch divides the slice into 100.
	int32 BatchIndex_100;
	int32 CurrentBatchIndex_100 = 0;

	UPROPERTY()
	TScriptInterface<IEDU_CORE_CommandInterface> CommandInterface;
	// IEDU_CORE_CommandInterface* CommandInterface;

	int32 MobileEntityBatchIndex;
	int32 SightComponentBatchIndex;
	int32 StatusComponentBatchIndex;
	int32 TurretComponentBatchIndex;
	int32 FixedWeaponComponentBatchIndex;
	int32 EngagementComponentBatchIndex;
	
	float LastMobilEntityBatchTime;
	float LastStatusComponentTime;
	float LastEngagementComponentTime;
	float LastSightComponentTime;
	float LastTurretComponentTime;
	float LastFixedWeaponComponentTime;

	UPROPERTY()
	float AsyncedClock = 0;

	UPROPERTY()
	float LastAsyncedClock = 0;
	
	UPROPERTY()
	float AsyncDeltaTime = 0.f;

	UPROPERTY()
	float AccumulatedAsyncDeltatime = 0.f;
	
	UPROPERTY()
	float LastAccumulatedAsyncTicks = 0.f;
	

//------------------------------------------------------------------------------
// Components > Debug
//------------------------------------------------------------------------------
	
	FColor DeltaTimeDisplayColor;

	int32 CurrentFrameTime = 0;
	float FPSCounter = 0;
	float FPSTotal = 0;

	int32 FrameTimeCounter;
	
//------------------------------------------------------------------------------
// Console Commands
//------------------------------------------------------------------------------
protected:
	
	// Server only: change team
	UFUNCTION(Exec)
	void ChangeTeamCommand(const int32 NewTeam) const;
	
};