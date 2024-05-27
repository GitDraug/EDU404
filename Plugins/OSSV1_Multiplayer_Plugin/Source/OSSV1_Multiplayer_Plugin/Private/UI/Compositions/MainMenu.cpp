// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Compositions/MainMenu.h"
#include "Components/Button.h"
#include "Framework/Data/FlowLog.h"
#include "Framework/Managers/OSSV1_MP_Subsystem.h"

//--------------------------------------------------------
// Construction & Initialization
//--------------------------------------------------------
bool UMainMenu::Initialize()
{ FLOW_LOG
	// If Super returns false, we should do so also and GTFO.
	if(!Super::Initialize()) { FLOW_LOG_WARNING("Initialize returned false") return false; }

	if(HostButton) { HostButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HostButtonClicked); }
	if(JoinButton) { JoinButton->OnClicked.AddUniqueDynamic(this, &ThisClass::JoinButtonClicked); }
	
	return true;
}

//--------------------------------------------------------
// Object lifetime Management
//--------------------------------------------------------
void UMainMenu::MenuSetup()
{ FLOW_LOG
	
	AddToViewport(); // Adds it to the game's viewport and fills the entire screen, unless SetDesiredSizeInViewport is called to explicitly set the size.
	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(true);

	// Mouse settings
	if(UWorld* World = GetWorld())
	{
		if(APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			/*----------------------------------------------------------------
			  SetInputMode() needs at Data structure used to set up an input
			  mode that allows only the UI to respond to user input
			----------------------------------------------------------------*/
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget()); // TakeWidget Gets the underlying slate widget or constructs it if it doesn't exist. If you're looking to replace what slate widget gets constructed, look for RebuildWidget.
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
		else // Debug 
		{
			FLOW_LOG_ERROR("PlayerController is null")
		}
	}
	else // Debug 
	{
		FLOW_LOG_ERROR("World is null")
	}

	// Get me a link to the MP_Subsystem from the GameInstance
	if(UGameInstance* GameInstance = GetGameInstance())
	{
		MP_Subsystem = GameInstance->GetSubsystem<UOSSV1_MP_Subsystem>();
	}

	if(MP_Subsystem)
	{
		MP_Subsystem->OnCreateSessionCompleteEcho.AddUniqueDynamic(this, &ThisClass::OnCreateSessionComplete);
		MP_Subsystem->OnFindSessionsCompleteEcho.AddUObject(this, &ThisClass::OnFindSessionsComplete);
		MP_Subsystem->OnJoinSessionCompleteEcho.AddUObject(this, &ThisClass::OnJoinSessionComplete);
	}
}

void UMainMenu::NativeDestruct()
{ FLOW_LOG
	// Clean up.
	MenuTearDown();
	Super::NativeDestruct();
}

void UMainMenu::MenuTearDown()
{ FLOW_LOG
	RemoveFromParent();
}

//--------------------------------------------------------
// Functionality
//--------------------------------------------------------

void UMainMenu::HostButtonClicked()
{ FLOW_LOG
	if(MP_Subsystem)
	{
		// Create Session with settings
		MP_Subsystem->CreateSession();
	}
}

void UMainMenu::JoinButtonClicked()
{ FLOW_LOG
	if (MP_Subsystem)
	{
		MP_Subsystem->FindSessions();
	}
}

//--------------------------------------------------------
// Delegated Events
//--------------------------------------------------------

void UMainMenu::OnCreateSessionComplete(bool bWaSuccessful)
{ FLOW_LOG
	FLOW_LOG_WARNING("Session creation complete: releasing Host button")
}

void UMainMenu::OnFindSessionsComplete(const TArray<FOnlineSessionSearchResult>& SearchResults, bool bWasSuccessful)
{ FLOW_LOG
	
}

void UMainMenu::OnJoinSessionComplete(EOnJoinSessionCompleteResult::Type Result)
{ FLOW_LOG
}
