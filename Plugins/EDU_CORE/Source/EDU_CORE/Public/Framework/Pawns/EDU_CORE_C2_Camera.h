// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EDU_CORE_SpectatorCamera.h"
#include "EDU_CORE_C2_Camera.generated.h"

class AEDU_CORE_Waypoint;
class AEDU_CORE_Waypoint_Move;

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
	// Called upon possession by a PlayerController, to bind functionality to input.
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// Input mode is UI && Game etc.
	virtual void SetPlayerInputMode() override;

	// Used to initialize the variables in the DataAsset
	virtual void SetPawnDefaults() override;

//------------------------------------------------------------------------------
// Components
//------------------------------------------------------------------------------
	
	UPROPERTY()
	TArray<FGuid> ServerIDArray; // Used for selecting entities on the server.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoints")
	TSubclassOf<AEDU_CORE_Waypoint> WaypointClass;

	UPROPERTY()
	TObjectPtr<AEDU_CORE_Waypoint_Move> Waypoint_NavigateTo;

	UPROPERTY()
	TObjectPtr<AEDU_CORE_Waypoint_Move> Waypoint_LookAt ;
	
//---------------------------------------------------------------------------
// Input Functionality: Mouse
//---------------------------------------------------------------------------

	virtual void Input_Mouse_1_Released(const FInputActionValue& InputActionValue) override;
	
	virtual void Input_Mouse_2_Pressed(const FInputActionValue& InputActionValue) override;
	virtual void Input_Mouse_2_Released(const FInputActionValue& InputActionValue) override;

//---------------------------------------------------------------------------
// Waypoint logic
//---------------------------------------------------------------------------
	
	virtual void Command_LookAt();
	virtual void Command_NavigateTo();
	
//---------------------------------------------------------------------------
// Server logic
//---------------------------------------------------------------------------

	UFUNCTION(Server, Reliable)
	virtual void GetEntitiesOnServer(const TArray<FGuid>& IDArray);
	
	UFUNCTION(Server, Reliable)
	void ServerCommand_LookAt();
	
};
