#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h" // Required?
#include "EDU_CORE_TitleScreensTable.generated.h" // UnrealBuildTool

/*------------------------------------------------------------------------------
  Abstract SUPER Class intended to be inherited from.
--------------------------------------------------------------------------------
  This FStruct allows a DataTable to be constructed in the editor with
  these predetermined rows. It's used by EDU_CORE_GameMode_Splash to load the
  desired TtleScreen.
------------------------------------------------------------------------------*/

USTRUCT(BlueprintType)
struct EDU_CORE_API FEDU_CORE_TitleScreensTable	: public FTableRowBase // We inherit from FTableRowBase to make it a table in the Editor
{
	GENERATED_BODY()

	// Path to the map you wish to load on startup without quotation. Example: /Game/Maps/Main/MainMenu
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Title Screen")
	FString LevelPath;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Title Screen")
	FString Description;
};
