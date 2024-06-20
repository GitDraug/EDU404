// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "EDU_CORE_GameMode_TitleScreen.generated.h"

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

  --------------------------
   Main Menu
  --------------------------
  This GameMode holds the Main menu, and should be the same no matter what
  Title Screen background (Wallpaper Level) is used
  
------------------------------------------------------------------------------*/

UCLASS(Abstract)
class EDU_CORE_API AEDU_CORE_GameMode_TitleScreen : public AGameModeBase
{
  GENERATED_BODY()

//------------------------------------------------------------------------------
// Initialization & Object lifetime management
//------------------------------------------------------------------------------	
protected:
  AEDU_CORE_GameMode_TitleScreen(const FObjectInitializer& ObjectInitializer);
  
  virtual void BeginPlay() override;

//------------------------------------------------------------------------------
// Components
//------------------------------------------------------------------------------
protected:

//------------------------------------------------------------------------------
// Functionality
//------------------------------------------------------------------------------

};
