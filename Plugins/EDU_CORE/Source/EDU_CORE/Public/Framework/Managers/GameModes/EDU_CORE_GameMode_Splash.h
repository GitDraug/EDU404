// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "EDU_CORE_GameMode_Splash.generated.h"

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

  ---------------------------------
   <!> Don't make this default <!>
  ---------------------------------
  This SplashScreen is the first thing the game loads up, choosing a game menu
  depending on a save file. If you set this to a default GameMode for new maps,
  it will cause endless loops, loading levels that loads new levels.
  
------------------------------------------------------------------------------*/

UCLASS(Abstract)
class EDU_CORE_API AEDU_CORE_GameMode_Splash : public AGameModeBase
{
  GENERATED_BODY()

//------------------------------------------------------------------------------
// Initialization & Object lifetime management
//------------------------------------------------------------------------------	
protected:
  AEDU_CORE_GameMode_Splash(const FObjectInitializer& ObjectInitializer);
  
  virtual void BeginPlay() override;

//------------------------------------------------------------------------------
// Components
//------------------------------------------------------------------------------
protected:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Title Screen")
  TSoftObjectPtr<UWorld> DefaultTitleScreen;
  
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Title Screen")
  UDataTable* TitleScreenLevelTable;
  
//------------------------------------------------------------------------------
// Functionality
//------------------------------------------------------------------------------
protected:
  UFUNCTION(BlueprintCallable, Category = "Title Screen")
  void LoadTitleScreenFromGameMode();
  
  UFUNCTION(BlueprintCallable, Category = "Title Screen")
  void LoadTitleScreenFromTable(FName RowName = TEXT("Default")) const;
};
