// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MainMenu.generated.h"

class UButton;
class UOSSV1_MP_Subsystem;

/*--------------------------------------------------------
  Main menu visible when game starts
--------------------------------------------------------*/
UCLASS()
class OSSV1_MULTIPLAYER_PLUGIN_API UMainMenu : public UUserWidget
{
	GENERATED_BODY()

//--------------------------------------------------------
// Object lifetime Management
//--------------------------------------------------------
public:
	UFUNCTION(BlueprintCallable)
	void MenuSetup();

protected:
	// Used to init widgets, bind buttons, etc.
	virtual bool Initialize() override;

	// Destruction
	void MenuTearDown();
	virtual void NativeDestruct() override;
	
//--------------------------------------------------------
// Functionality
//--------------------------------------------------------
private:
	//----------------------------------------------
	// Button Callbacks
	//----------------------------------------------
	UFUNCTION()
	void HostButtonClicked();
	
	UFUNCTION()
	void JoinButtonClicked();

	//----------------------------------------------
	// Delegated Events
	//----------------------------------------------
	UFUNCTION()
	void OnCreateSessionComplete(bool bWaSuccessful);

	// Beware const correctness here
	void OnFindSessionsComplete(const TArray<FOnlineSessionSearchResult>& SearchResults, bool bWasSuccessful);
	void OnJoinSessionComplete(EOnJoinSessionCompleteResult::Type Result);
	
	
//--------------------------------------------------------
// Widget Components
//--------------------------------------------------------
private:
	UPROPERTY(meta = (BindWidget))
	UButton* HostButton;
	
	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;

	// Link to our MP Subsystem
	UPROPERTY()
	UOSSV1_MP_Subsystem* MP_Subsystem;

};
