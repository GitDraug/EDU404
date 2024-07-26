// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Managers/GameModes/EDU_CORE_GameMode_TitleScreen.h"
#include "EnhancedInputSubsystems.h"
#include "Framework/Data/FLOWLOGS/FLOWLOG_MANAGERS.h"

//------------------------------------------------------------------------------
// Initialization & Object lifetime management
//------------------------------------------------------------------------------
AEDU_CORE_GameMode_TitleScreen::AEDU_CORE_GameMode_TitleScreen(const FObjectInitializer& ObjectInitializer)
{ FLOW_LOG
	// Disable ticking for this GameMode
	PrimaryActorTick.bCanEverTick = false;
}

void AEDU_CORE_GameMode_TitleScreen::BeginPlay()
{ FLOW_LOG
	Super::BeginPlay();

	if(APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if(UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			// In case hidden earlier
			PlayerController->SetShowMouseCursor(true);
			
			// Reset InputMappings, in case anything remains from the previous level or menu or whatnot.
			InputSubsystem->ClearAllMappings();

			FInputModeGameAndUI InputMode; // Settings container
			InputMode.SetHideCursorDuringCapture(false); // Whether to hide the cursor during temporary mouse capture caused by a mouse down
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock); // Don't lock to window. It's better to put this in a menu.

			PlayerController->SetInputMode(InputMode);
		}
	}
	
}

//------------------------------------------------------------------------------
// Functionality
//------------------------------------------------------------------------------
