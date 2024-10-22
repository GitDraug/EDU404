// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EDU_CORE_SpectatorCamera.h"
#include "EDU_CORE_C2_Camera.generated.h"

class EDU_CORE_DataTypes;
class AEDU_CORE_Waypoint;

/*------------------------------------------------------------------------------
  Abstract SUPER Class intended to be inherited from.
--------------------------------------------------------------------------------
  The C2 (Command and control) camera allows the user to interact with actors
  and entities in the level. It is a continuation of the SpectatorCamera.
------------------------------------------------------------------------------*/

UCLASS()
class EDU_CORE_API AEDU_CORE_C2_Camera : public AEDU_CORE_SpectatorCamera
{
	GENERATED_BODY()
	
//------------------------------------------------------------------------------
// Construction & Init
//------------------------------------------------------------------------------
public:
	// Sets default values for this pawn's properties
	AEDU_CORE_C2_Camera();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

//------------------------------------------------------------------------------
// Input Setup
//------------------------------------------------------------------------------
protected:
	// Called upon possession by a PlayerController, to bind functionality to input.
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// Input mode is UI && Game etc.
	virtual void SetPlayerInputMode() override;

	// Used to initialize the variables in the DataAsset
	virtual void SetPawnDefaults() override;

//------------------------------------------------------------------------------
// Components
//------------------------------------------------------------------------------
protected:
	// Enum to easily select Team.
	EEDU_CORE_Team Team;
	
	UPROPERTY()
	TArray<FGuid> ServerIDArray; // Used for selecting entities on the server.
	
//---------------------------------------------------------------------------
// Input Functionality: Mouse
//---------------------------------------------------------------------------

	virtual void Input_Mouse_1_Released(const FInputActionValue& InputActionValue) override;
	
	virtual void Input_Mouse_2_Pressed(const FInputActionValue& InputActionValue) override;
	virtual void Input_Mouse_2_Released(const FInputActionValue& InputActionValue) override;

//---------------------------------------------------------------------------
// Command logic
//---------------------------------------------------------------------------
	
	virtual void CreateWaypoint(const FVector WorldPosition, const EEDU_CORE_WaypointType WaypointType = EEDU_CORE_WaypointType::NavigateTo, const bool Queue = false);
	
//---------------------------------------------------------------------------
// RPC logic
//---------------------------------------------------------------------------

	// Adds an Array of GUIDs to SelectionArray
	UFUNCTION(Server, Reliable)
	virtual void GetEntitiesOnServer(const TArray<FGuid>& IDArray);

	// Creates a Waypoint with a WorldPosition that AI will look at.
	UFUNCTION(Server, Reliable)
	void Server_CreateWaypoint(const FVector WorldPosition, const EEDU_CORE_WaypointType WaypointType = EEDU_CORE_WaypointType::NavigateTo, const bool Queue = false);
};
