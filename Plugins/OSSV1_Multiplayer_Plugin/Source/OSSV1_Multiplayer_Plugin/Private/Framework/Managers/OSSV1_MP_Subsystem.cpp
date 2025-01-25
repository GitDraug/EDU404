// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Managers/OSSV1_MP_Subsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Framework/Data/FlowLog.h"
#include "Online/OnlineSessionNames.h"

//-----------------------------------------------------------------
// Construction & Initialization
//-----------------------------------------------------------------
UOSSV1_MP_Subsystem::UOSSV1_MP_Subsystem():
// Init list, binding functions in this class to OSSV1 Delegates
	OSSV1_OnCreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
	OSSV1_OnFindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)),
	OSSV1_OnJoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),
	OSSV1_OnDestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete)),
	OSSV1_OnStartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete))
{ FLOW_LOG
// Check if we are Online or Offline
	if(IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get())
	{
		// If We are offline, then set OfflineMode to true
		OnlineSubsystem->GetSubsystemName() == "NULL" ? SetOfflineMode(true) : SetOfflineMode(false);
		SessionInterface = OnlineSubsystem->GetSessionInterface();
		
		#ifdef NDEBUG 
		if(GEngine)
		{	// We can't see the Subsystem unless we launch in an independent launcher.
			GEngine->AddOnScreenDebugMessage(-1, 60.f, FColor::White, FString::Printf(TEXT("Subsystem indicates we are %s"), (OnlineSubsystem->GetSubsystemName().ToString() != "Null") ? TEXT("Online") : TEXT("Offline")));
		}
		#endif
	}
	else // Debug 
	{
		FLOW_LOG_ERROR("OnlineSubsystem is null")
	}
}

//-----------------------------------------------------------------
// Get/Set
//-----------------------------------------------------------------
void UOSSV1_MP_Subsystem::SetOfflineMode(bool IsOffline)
{ FLOW_LOG
	bOfflineMode = IsOffline;
	
	#ifdef NDEBUG
		if(bOfflineMode)
		{
			if(GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 60.f, FColor::White,
					FString::Printf(TEXT("bOfflineMode == true")));
			}
		}
		else
		{
			if(GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 60.f, FColor::White,
					FString::Printf(TEXT("bOfflineMode == false")));
			}
		}
	#endif
}

//-----------------------------------------------------------------
// Session Functionality with confirmation from bound delegate.
//-----------------------------------------------------------------
void UOSSV1_MP_Subsystem::CreateSession()
{ FLOW_LOG
	if(!SessionInterface.IsValid()) {FLOW_LOG_ERROR("SessionInterface is invalid.") return;}
	
	/*-------------------------------------------------------------------------------------------------------------
	  <!> Check if there is already a Session by our name, and destroy it before starting a new one in that case
	---------------------------------------------------------------------------------------------------------------
	  NAME_GameSession is a reserved name that refers to the game session within the engine's codebase.
	  This name serves as a unique identifier for the UGameSession class or instances of it, allowing us
	  to access or manipulate the game session through code.
	---------------------------------------------------------------------------------------------------------------*/
	if(SessionInterface->GetNamedSession(NAME_GameSession) == nullptr)
	{
		FLOW_LOG_ERROR("NAME_GameSession Already exists, destroying Session")
		SessionInterface->DestroySession(NAME_GameSession);
	}

	// Store our delegate in an FDelegateHandle, allowing us to remove it from the delegate list on demand. 
	OSSV1_OnCreateSessionCompleteDelegate_Handle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(OSSV1_OnCreateSessionCompleteDelegate);

	// Settings for the session to be created. MakeShareable is a utility function, it returns a shared pointer.
	SessionSettings = MakeShareable(new FOnlineSessionSettings());
	

	/*-------------------------------------------------------------------------------------------------------------
	  FOnlineSessionSettings is a Container for all settings describing a single online session
	  See OnlineSessionSettings.h for a full list of available settings.

	  These shouldn't be hardcoded, but set in the lobby.
	---------------------------------------------------------------------------------------------------------------*/
	// Don't go online if the player explicitly decided to be offline.
	if(IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" || bOfflineMode == true)
	{
		SessionSettings->bIsLANMatch = true;
		FLOW_LOG_WARNING("SessionSettings: Offline Mode.")
	}
	else
	{
		SessionSettings->bIsLANMatch = false;
		FLOW_LOG_WARNING("SessionSettings: Online Mode.")
	}

	SessionSettings->NumPublicConnections = StoredPublicConnections;
	SessionSettings->bAllowJoinInProgress = bStoredAllowJoinInProgress;
	SessionSettings->bAllowJoinViaPresence = bStoredAllowJoinViaPresence;
	SessionSettings->bShouldAdvertise = bStoredShouldAdvertise;
	SessionSettings->bUsesPresence = bStoredUsesPresence;
	SessionSettings->bUseLobbiesIfAvailable = bStoredUseLobbiesIfAvailable;

	/*-------------------------------------------------------------------------------------------------------------
	  Sets a key value pair combination that defines a session setting
	-------------------------------------------------------------------------------------------------------------*/ 
	SessionSettings->Set(
		StoredKey,												// Key for the setting
		StoredGameType,											// Value of the setting
		EOnlineDataAdvertisementType::ViaOnlineServiceAndPing	// Type of online advertisement
	);

	/*-------------------------------------------------------------------------------------------------------------
	  We can Create the session now, but note that Online session registration is an async process and does not
	  complete until the OnCreateSessionComplete delegate is called. OnCreateSessionComplete is a bool, and will
	  return true when it's done, that's why we wrap it in an if.
	-------------------------------------------------------------------------------------------------------------*/
	if(!SessionInterface->CreateSession(
			*GetWorld()->GetFirstLocalPlayerFromController()->GetPreferredUniqueNetId(), // HostingPlayerId the index of the player hosting the session
			NAME_GameSession,	// SessionName the name to use for this session so that multiple sessions can exist at the same time
			*SessionSettings)	// NewSessionSettings is the settings to use for the new session
	)
	{ // We don't need the handle any longer.
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(OSSV1_OnCreateSessionCompleteDelegate_Handle);

		// Tell any class waiting for this that this has failed.
		OnCreateSessionCompleteEcho.Broadcast(false);
		FLOW_LOG_ERROR("CreateSession Failed")
	}

	/*-------------------------------------------------------------------------------------------------------------
	  Now we need to wait for the async process to complete. Its delegate till broadcast true in that case and
	  trigger our bound function OnCreateSessionComplete() in that case. We can't use an else event because
	  it finishes in a single frame.
	-------------------------------------------------------------------------------------------------------------*/
}
void UOSSV1_MP_Subsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{ FLOW_LOG
	/*-------------------------------------------------------------------------------------------------------------
	  This function is only triggered if the async Online session registration process finishes successfully
	-------------------------------------------------------------------------------------------------------------*/
	
	// We don't need the handle any longer.
	SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(OSSV1_OnCreateSessionCompleteDelegate_Handle);
		
	// Tell any class waiting for this that this has succeeded.
	OnCreateSessionCompleteEcho.Broadcast(bWasSuccessful);
	FLOW_LOG_WARNING("CreateSession Successful")

	#ifdef NDEBUG
		if(IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get())
		{
			if(GEngine)
			{	// We can't see the Subsystem unless we launch in a separate launcher.
				GEngine->AddOnScreenDebugMessage(-1, 60.f, FColor::White, FString::Printf(TEXT("Subsystem = %s"), *OnlineSubsystem->GetSubsystemName().ToString()));
			}
		}
	#endif

	// The Session is created, so we want to move to the Lobby now, where other players can join us.
	TravelToLobby();
}

void UOSSV1_MP_Subsystem::FindSessions()
{FLOW_LOG
	if(!SessionInterface.IsValid()) {FLOW_LOG_ERROR("SessionInterface in InValid") return;}

	// Async, so we need to know when it's done, thus we need a handle of the right type.
	OSSV1_OnFindSessionsCompleteDelegate_Handle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(OSSV1_OnFindSessionsCompleteDelegate);

	// Search Settings
	SessionSearch = MakeShareable(new FOnlineSessionSearch);
	SessionSearch->MaxSearchResults = StoredMaxSearchResults;
	SessionSearch->bIsLanQuery = bOfflineMode;
	SessionSearch->TimeoutInSeconds = StoredTimeoutInSeconds;
	SessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals); // Updated since 5.5

		/* Ensure Consistency in Session Settings:
			Set bUsesPresence and bUseLobbiesIfAvailable to the same value in FOnlineSessionSettings.
			Update session search results to reflect these values before joining a session. For example:
			SearchResults[ID].Session.SessionSettings.bUsesPresence = true;
			SearchResults[ID].Session.SessionSettings.bUseLobbiesIfAvailable = true;

			https://chatgpt.com/share/6747754a-40b0-8013-9ac0-2d44c2095c09
	
	/*-------------------------------------------------------------------------------------------------------------
	  We can Search the session now, but note that Online session search is an async process and does not
	  complete until the OnFindSessionsComplete delegate is called. OnFindSessionsComplete is a bool, and will
	  return true when it's done, that's why we wrap it in an if.
	-------------------------------------------------------------------------------------------------------------*/
	if(!SessionInterface->FindSessions(
		*GetWorld()->GetFirstLocalPlayerFromController()->GetPreferredUniqueNetId(), // The id of the player searching for a match
		SessionSearch.ToSharedRef()) // The desired settings that the returned sessions will have turned into a shared pointer.
	)
	{// We don't need the handle any longer.
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(OSSV1_OnFindSessionsCompleteDelegate_Handle);

		// Tell any class waiting for this that this has failed, and broadcast an empty array.
		OnFindSessionsCompleteEcho.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		FLOW_LOG_ERROR("FindSession Failed")
	}
	/*-------------------------------------------------------------------------------------------------------------
	  Now we need to wait for the async process to complete. Its delegate till broadcast true in that case and
	  trigger our bound function OnFindSessionsComplete() in that case. We can't use an else event because
	  it finishes in a single frame.
	-------------------------------------------------------------------------------------------------------------*/
}
void UOSSV1_MP_Subsystem::OnFindSessionsComplete(bool bWasSuccessful)
{FLOW_LOG
	if(SessionInterface)
	{
		// We don't need the handle any longer.
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(OSSV1_OnFindSessionsCompleteDelegate_Handle);
	}

	#ifdef NDEBUG // Check if we are Online or Offline 
		if(IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get())
		{
			if(GEngine)
			{	// We can't see the Subsystem unless we launch in a separate launcher.
				GEngine->AddOnScreenDebugMessage(1, 60.f, FColor::White, FString::Printf(TEXT("Subsystem indicates we are %s"), (OnlineSubsystem->GetSubsystemName().ToString() != "Null") ? TEXT("Online") : TEXT("Offline")));
				GEngine->AddOnScreenDebugMessage(2, 60.f, FColor::Blue, FString::Printf(TEXT("Found %d sessions"), SessionSearch->SearchResults.Num() ));
			}
		}
		else // Debug 
		{
			FLOW_LOG_ERROR("OnlineSubsystem is null")
		}
	#endif
	
	if(SessionSearch->SearchResults.Num() <= 0)
	{
		OnFindSessionsCompleteEcho.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		FLOW_LOG_WARNING("SessionSearch succeded, but found no games.")
	}
	else
	{
		// Broadcast the Array of games we found.
		OnFindSessionsCompleteEcho.Broadcast(SessionSearch->SearchResults, bWasSuccessful);
		FLOW_LOG_WARNING("SessionSearch succeded, broadcasting found Games.")
	}

	/*-------------------------------------------------------------------------------------------------------------
	  If this is a matchmaker, we want to join the best session immediately, otherwise we want to build a list
	  of our search results in the search menu.
	-------------------------------------------------------------------------------------------------------------*/
	if(QuickJoin)
	{
		// Loop through the array and pick the first match.
		for(auto Session : SessionSearch->SearchResults)
		{
			Session.Session.SessionSettings.Get(
				StoredKey,				// Key for the setting
				StoredGameType);		// Value of the setting

				#ifdef NDEBUG
					if(GEngine)
					{
						GEngine->AddOnScreenDebugMessage(11, 60.f, FColor::Green, FString::Printf(TEXT("Session Key %s"), *StoredKey.ToString() ));
						GEngine->AddOnScreenDebugMessage(12, 60.f, FColor::Emerald, FString::Printf(TEXT("Game Type %s"), *StoredGameType ));
					}
				#endif
			
			if(StoredKey == StoredGameType)
			{
				JoinSession(Session);
			}
		}
	}
}

void UOSSV1_MP_Subsystem::JoinSession(const FOnlineSessionSearchResult& Session)
{ FLOW_LOG
	if(!SessionInterface.IsValid())
	{ FLOW_LOG_ERROR("SessionInterface is InValid")
		OnJoinSessionCompleteEcho.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return;
	}

	// We need a handle for functionality.
	OSSV1_OnJoinSessionCompleteDelegate_Handle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(OSSV1_OnJoinSessionCompleteDelegate);

	if(!SessionInterface->JoinSession(
		*GetWorld()->GetFirstLocalPlayerFromController()->GetPreferredUniqueNetId(), // The id of the player searching for a match
		NAME_GameSession,	// The name of the session to join
		Session)			// The desired session to join
	)
	{ FLOW_LOG_ERROR("Failed tyo join session")
		// Remove The handle so we can start over.
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(OSSV1_OnJoinSessionCompleteDelegate_Handle);
		// Broadcast to anyone listening that we failed.
		OnJoinSessionCompleteEcho.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
	}

	/*-------------------------------------------------------------------------------------------------------------
	  Success will trigger our bound function OnJoinSessionComplete()
	-------------------------------------------------------------------------------------------------------------*/
}
void UOSSV1_MP_Subsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{ FLOW_LOG
	if(SessionInterface)
	{ // We no longer need the handle.
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(OSSV1_OnJoinSessionCompleteDelegate_Handle);
	}
	
	// Broadcast to anyone listening that we succeeded.
	OnJoinSessionCompleteEcho.Broadcast(Result);
	// Save information
	SessionInterface->GetResolvedConnectString(NAME_GameSession, StoredAddress);

	// We need a playerController to travel
	if(APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController())
	{
		#ifdef NDEBUG
			if(GEngine)
			{
				GEngine->AddOnScreenDebugMessage(3, 60.f, FColor::Green, FString::Printf(TEXT("Traveling to Adress %s"), *StoredAddress));
			}
		#endif
		PlayerController->ClientTravel(StoredAddress, ETravelType::TRAVEL_Absolute);
	}
		
}

void UOSSV1_MP_Subsystem::DestroySession()
{ FLOW_LOG
	
}
void UOSSV1_MP_Subsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{ FLOW_LOG
	
}

void UOSSV1_MP_Subsystem::StartSession()
{ FLOW_LOG
	
}
void UOSSV1_MP_Subsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{ FLOW_LOG
	
}

//-----------------------------------------------------------------
// Internal Functionality
//-----------------------------------------------------------------
void UOSSV1_MP_Subsystem::TravelToLobby()
{ FLOW_LOG
	if(UWorld* World = GetWorld())
	{
		if(*LobbyAddress)
		{
			World->ServerTravel(LobbyAddress);
		}
		else
		{
			FLOW_LOG_ERROR("LobbyAdreess is null.")
		}
	}
}










