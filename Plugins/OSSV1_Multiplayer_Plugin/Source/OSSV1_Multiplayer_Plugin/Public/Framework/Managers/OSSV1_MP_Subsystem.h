// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OSSV1_MP_Subsystem.generated.h"

//-----------------------------------------------------------------
// Custom Delegates
//-----------------------------------------------------------------
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCreateSessionCompleteEcho, bool, bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnFindSessionsCompleteEcho, const TArray<FOnlineSessionSearchResult>& SearchResults, bool bWasSuccessful); 
DECLARE_MULTICAST_DELEGATE_OneParam(FOnJoinSessionCompleteEcho, EOnJoinSessionCompleteResult::Type Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDestroySessionCompleteEcho, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStartSessionCompleteEcho, bool, bWasSuccessful);

/*-----------------------------------------------------------------
 OSSV1 Multiplayer subsystem
-----------------------------------------------------------------*/
UCLASS()
class OSSV1_MULTIPLAYER_PLUGIN_API UOSSV1_MP_Subsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
//-----------------------------------------------------------------
// Local Delegate Echoes, for others to bind to us.
//-----------------------------------------------------------------
public:

	FOnCreateSessionCompleteEcho OnCreateSessionCompleteEcho;
	FOnFindSessionsCompleteEcho OnFindSessionsCompleteEcho;
	FOnJoinSessionCompleteEcho OnJoinSessionCompleteEcho;
	FOnDestroySessionCompleteEcho OnDestroySessionCompleteEcho;
	FOnStartSessionCompleteEcho OnStartSessionCompleteEcho;

	// These are OSSV1 Echoes triggered by OSSV1 delegates. We use Echoes so others can subscribe to this class
	// rather than OSSV1. That way, we only need to manage this class.
	
//-----------------------------------------------------------------
// Session Functionality, works like a public API
//-----------------------------------------------------------------
public:
	void CreateSession();
	void FindSessions();
	void JoinSession(const FOnlineSessionSearchResult& SearchResult);
	void DestroySession();
	void StartSession();

//-----------------------------------------------------------------
// Get/Set
//-----------------------------------------------------------------
	void SetOfflineMode(bool bIsOfflineMode);
	
//-----------------------------------------------------------------
// Construction & Initialization
//-----------------------------------------------------------------
public:
	UOSSV1_MP_Subsystem();
	
//-----------------------------------------------------------------
// Unreal Engine OnlineSubSystem V1 Functionality
//-----------------------------------------------------------------
private:
	// ThreadSafe SmartPtr
	TSharedPtr<IOnlineSession, ESPMode::ThreadSafe> SessionInterface;
	
	// Saved SessionSettings
	TSharedPtr<FOnlineSessionSettings>SessionSettings;
	// Saved SessionSearch
	TSharedPtr<FOnlineSessionSearch>SessionSearch;
	
	// OSSV1 Delegates, these needs to be bound in the InitList of the constructor to work properly.
	FOnCreateSessionCompleteDelegate	OSSV1_OnCreateSessionCompleteDelegate;
	FOnFindSessionsCompleteDelegate		OSSV1_OnFindSessionsCompleteDelegate;
	FOnJoinSessionCompleteDelegate		OSSV1_OnJoinSessionCompleteDelegate;
	FOnDestroySessionCompleteDelegate	OSSV1_OnDestroySessionCompleteDelegate;
	FOnStartSessionCompleteDelegate		OSSV1_OnStartSessionCompleteDelegate;

	// OSSV1 DelegateHandles, serves as a reference to a specific delegate INSTANCE, allowing us to manage and manipulate the delegate. since we might be hosting sessions in parallel, it's important that each use of delegate has a unique handle.
	FDelegateHandle	OSSV1_OnCreateSessionCompleteDelegate_Handle;
	FDelegateHandle	OSSV1_OnFindSessionsCompleteDelegate_Handle;
	FDelegateHandle	OSSV1_OnJoinSessionCompleteDelegate_Handle;
	FDelegateHandle	OSSV1_OnDestroySessionCompleteDelegate_Handle;
	FDelegateHandle	OSSV1_OnStartSessionCompleteDelegate_Handle;
	
//-----------------------------------------------------------------
// Internal Callbacks for delegates
//-----------------------------------------------------------------
protected:
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);
	
//-----------------------------------------------------------------
// Internal Functionality
//-----------------------------------------------------------------
private:
	void TravelToLobby();

	// We need a better way to set this, preferable through a DataType
	// ?listen allows us to listen for incoming pings, so others can join.
	const FString& LobbyAddress { "/Game/Maps/MultiplayerLobby?listen" };
	
//-----------------------------------------------------------------
// Stored/Default session settings
//-----------------------------------------------------------------
private:
	bool bOfflineMode { false };
	bool bStoredAllowJoinInProgress {true};
	bool bStoredAllowJoinViaPresence {true};
	bool bStoredShouldAdvertise {true};
	bool bStoredUsesPresence {true};
	bool bStoredUseLobbiesIfAvailable {true};

	int32 StoredPublicConnections { 10 };

	// These works as identifiers if they match, we can join a game.
	FName StoredKey { TEXT("FreeForAll") };
	FString StoredGameType { TEXT("FreeForAll") };

//-----------------------------------------------------------------
// Stored/Default Search settings
//-----------------------------------------------------------------
	int32 StoredMaxSearchResults { 10000 }; // Max number of queries returned by the matchmaking service
	bool StoredIsLanQuery { false } ; // Whether the query is intended for LAN matches or not
	int32 StoredTimeoutInSeconds { 60 }; // Amount of time to wait for the search results. May not apply to all platforms
	
	bool QuickJoin { true };

//-----------------------------------------------------------------
// Stored findings
//-----------------------------------------------------------------
	FString StoredAddress; // We store the address of our last game here, in case we crash and want to rejoin.
};
