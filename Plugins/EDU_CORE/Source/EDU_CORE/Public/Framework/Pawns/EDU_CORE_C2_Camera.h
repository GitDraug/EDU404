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
// Get/Set
//------------------------------------------------------------------------------  
public:
	FORCEINLINE virtual FVector GetSavedCursorWorldPos() const { return InitialCursorWorldPos; }
	FORCEINLINE EEDU_CORE_Team GetTeam() const { return ActiveTeam; }

	void SetTeam(EEDU_CORE_Team TeamIndex ) { ActiveTeam = TeamIndex; }
	
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
	// Called every frame (Client Only)
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

	//-----------------------------------------------------------------------
	// Mouse data
	//-----------------------------------------------------------------------

	// Used for waypoint rotation
	FVector2D InitialMousePos; 
	FVector InitialCursorWorldPos; 
	FRotator CursorRotation;

	//-----------------------------------------------------------------------
	// Waypoint data
	//-----------------------------------------------------------------------

	// How many % of the Veiwport Height we need to move to mouse when RBM is pressed to rotate the waypoint.
	float RotationActivationDistance = 4.f;

	// Should we rotate the waypoint towards the cursor?
	bool bRotateWaypoint = false;


	//-----------------------------------------------------------------------
	// Team
	//-----------------------------------------------------------------------
	
	UPROPERTY()
	TArray<FGuid> ServerIDArray; // Used for finding entities on the server.
	
//---------------------------------------------------------------------------
// Input Functionality: Mouse
//---------------------------------------------------------------------------

	virtual void Input_Mouse_1_Released(const FInputActionValue& InputActionValue) override;
	
	virtual void Input_Mouse_2_Pressed(const FInputActionValue& InputActionValue) override;
	virtual void Input_Mouse_2_Released(const FInputActionValue& InputActionValue) override;

//---------------------------------------------------------------------------
// Command logic
//---------------------------------------------------------------------------

	// Create a Waypoint with a WorldPosition that AI will look at
	virtual void CreateWaypoint(const FWaypointParams& WaypointParams);
	
//---------------------------------------------------------------------------
// RPC logic
//---------------------------------------------------------------------------

	// Adds an Array of GUIDs to SelectionArray
	UFUNCTION(Server, Reliable)
	virtual void GetEntitiesOnServer(const TArray<FGuid>& IDArray);

	// Ask Server to create a Waypoint for us.
	UFUNCTION(Server, Reliable)
	void Server_CreateWaypoint(const FWaypointParams& WaypointParams);
};
