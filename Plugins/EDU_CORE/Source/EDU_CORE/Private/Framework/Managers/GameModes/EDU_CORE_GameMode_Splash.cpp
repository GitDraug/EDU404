// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Managers/GameModes/EDU_CORE_GameMode_Splash.h"

#include "Framework/Data//DataTables/EDU_CORE_TitleScreensTable.h"
#include "Framework/Data/FLOWLOG/FLOWLOG_MANAGERS.h"
#include "Kismet/GameplayStatics.h"

//------------------------------------------------------------------------------
// Initialization & Object lifetime management
//------------------------------------------------------------------------------
AEDU_CORE_GameMode_Splash::AEDU_CORE_GameMode_Splash(const FObjectInitializer& ObjectInitializer)
{ FLOW_LOG
	// Disable ticking for this GameMode
	PrimaryActorTick.bCanEverTick = false;
}

void AEDU_CORE_GameMode_Splash::BeginPlay()
{ FLOW_LOG
	Super::BeginPlay();
	
	// Show Logo

	// Load TitleScreen based on Saved preference
	if(TitleScreenLevelTable)
	{
		LoadTitleScreenFromTable();	
	}
	else
	{
		FLOW_LOG_ERROR("TitleScreenLevelTable = nullptr. Did you set the table in the GameMode?")
	}

	// Load TitleScreen from GameMode if the table doesn't work.
	LoadTitleScreenFromGameMode();
}

//------------------------------------------------------------------------------
// Functionality
//------------------------------------------------------------------------------
void AEDU_CORE_GameMode_Splash::LoadTitleScreenFromGameMode()
{
	// Resolve the soft object pointer to get the actual UWorld object
	DefaultTitleScreen = DefaultTitleScreen.LoadSynchronous();
	
	if(DefaultTitleScreen == nullptr)
	{
		FLOW_LOG_ERROR("DefaultTitleScreen = nullptr. Did you set the map in the GameMode?")
	}
	else
	{
		// Get the URL map address from the UWorld object now that it's loaded.
		UGameplayStatics::OpenLevel(GetWorld(), FName(*DefaultTitleScreen->GetMapName()));
	}
}

void AEDU_CORE_GameMode_Splash::LoadTitleScreenFromTable(FName RowName) const
{ FLOW_LOG
	if (TitleScreenLevelTable)
	{
		static const FString ContextString(TEXT("Load TitleScreen From Table using RowName = Default"));
		/*---------------------------------------------------------------------------------------
		  The ContextString is a helpful tool for debugging and logging. It provides additional
		  information in the logs when FindRow fails to find the requested row, helping you
		  to quickly identify and resolve issues in your data table lookups.
		---------------------------------------------------------------------------------------*/
		
		if (FEDU_CORE_TitleScreensTable* TableRow = TitleScreenLevelTable->FindRow<FEDU_CORE_TitleScreensTable>(RowName, ContextString))
		{
			if(TableRow->LevelPath == "None")
			{
				FLOW_LOG_ERROR("Default map in TitleScreenLevelTable is set to 'None'. You need to set the name.")
			}
			else
			{
				UGameplayStatics::OpenLevel(GetWorld(), *TableRow->LevelPath);
			}
		}
	}
}
