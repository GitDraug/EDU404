#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "EDU_CORE_GameInstance.generated.h"

/*------------------------------------------------------------------------------
  Abstract SUPER Class intended to be inherited from.
--------------------------------------------------------------------------------
  The GameInstance is a high-level manager object for an instance of the
  running game. Spawned at game creation and not destroyed until game
  instance is shut down.

  Running as a standalone game, there will be one of these.
  Running in PIE (play-in-editor) will generate one of these per PIE instance.
------------------------------------------------------------------------------*/
UCLASS(Abstract)
class EDU_CORE_API UEDU_CORE_GameInstance : public UGameInstance
{
	GENERATED_BODY()

//------------------------------------------------------------------------------
// Initialization & Object lifetime management
//------------------------------------------------------------------------------	
public:
	UEDU_CORE_GameInstance(const FObjectInitializer& ObjectInitializer);
	
};
