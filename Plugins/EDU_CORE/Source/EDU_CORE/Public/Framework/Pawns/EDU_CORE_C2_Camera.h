// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EDU_CORE_SpectatorCamera.h"
#include "EDU_CORE_C2_Camera.generated.h"



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

//---------------------------------------------------------------------------
// Input Functionality: Mouse
//---------------------------------------------------------------------------
	virtual void Input_Mouse_2_Pressed(const FInputActionValue& InputActionValue) override;
	virtual void Input_Mouse_2_Released(const FInputActionValue& InputActionValue) override;

//---------------------------------------------------------------------------
// Waypoint logic
//---------------------------------------------------------------------------
	
	virtual void Command_LookAt();

	virtual void Send_Command();
	
	float CommandDelay;
	
	
};
