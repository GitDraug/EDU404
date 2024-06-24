// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EDU_USER_PlayerController.generated.h"

/*------------------------------------------------------------------------------
  Abstract SUPER Class intended to be inherited from.
--------------------------------------------------------------------------------
  PlayerControllers are used by human players to control Pawns.
  ControlRotation (accessed via GetControlRotation()), determines the aiming
  orientation of the controlled Pawn.
 
  In networked games, PlayerControllers exist on the server for every
  player-controlled pawn, and also on the controlling client's machine.
  They do NOT exist on a client's machine for pawns controlled by remote
  players elsewhere on the network.

  The Player controller can make remote Procedure Calls, so if a client needs
  something done on the server, the PlayerController is the way to go.

  The Player controller is created on the GameMode and replicated to the
  owning client. This means that the GameMode owns all Player controllers.
------------------------------------------------------------------------------*/
UCLASS(Abstract)
class EDU_USER_API AEDU_USER_PlayerController : public APlayerController
{
	GENERATED_BODY()

//------------------------------------------------------------------------------
// Initialization & Object lifetime management
//------------------------------------------------------------------------------	
public:
  AEDU_USER_PlayerController(const FObjectInitializer& ObjectInitializer);

//------------------------------------------------------------------------------
// Get/Set
//------------------------------------------------------------------------------  
public:
  // The GameMode in EDU_USER effect these
  virtual void SetPlayerStartLocation(const FVector Location) { PlayerStartLocation = Location; };
  virtual FVector GetPlayerStartLocation() const { return PlayerStartLocation; }

//------------------------------------------------------------------------------
// Functionality 
//------------------------------------------------------------------------------
public:
  // Traces the CameraAnchors position in the world.
  void GetTerrainPosition(FVector& TargetPos, FVector& LastValidPos) const;

  // Traces the MouseCursors position in the world.
  void GetMouseCursorOnTerrain(FVector& TerrainPosition) const;
  
//------------------------------------------------------------------------------
// Components
//------------------------------------------------------------------------------
protected:
  // Used to Spawn Entities at a certain location.
  UPROPERTY()
  FVector PlayerStartLocation;
  
};
